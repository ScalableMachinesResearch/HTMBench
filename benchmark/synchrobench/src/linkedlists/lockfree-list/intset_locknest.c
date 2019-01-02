/*
 *  intset.c
 *
 *  Integer set operations (contain, insert, delete)
 *  that call stm-based / lock-free counterparts.
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "intset.h"
#include "large_tx.h"

#define LTX_NODE_TYPE node_t
#define ITERATION_NUMBER_GENERATOR (rand()%30 + 10)

int set_contains(intset_t *set, val_t val, int transactional)
{
	int result;

#ifdef DEBUG
	printf("++> set_contains(%d)\n", (int)val);
	IO_FLUSH;
#endif

#ifdef SEQUENTIAL
	node_t *prev, *next;

	prev = set->head;
	next = prev->next;
	while (next->val < val) {
		prev = next;
		next = prev->next;
	}
	result = (next->val == val);

#elif defined STM

	node_t *prev, *next;
	val_t v = 0;


	LARGE_TX_BEGIN(set->head);
	prev = set->head;
	//next = (node_t *)TX_LOAD(&prev->next);
	while (1) {
		next = (node_t *)TX_LOAD(&prev->next);// add jqswang
	  v = TX_LOAD((uintptr_t *) &next->val);
	  if (v >= val)
	    break;
		LARGE_TX_BREAK;
		prev = next;
		//next = (node_t *)TX_LOAD(&prev->next);
	}
	LARGE_TX_END;
	result = (v == val);

#elif defined LOCKFREE
	result = harris_find(set, val);
#endif

	return result;
}

inline int set_seq_add(intset_t *set, val_t val)
{
	int result;
	node_t *prev, *next;

	prev = set->head;
	next = prev->next;
	while (next->val < val) {
		prev = next;
		next = prev->next;
	}
	result = (next->val != val);
	if (result) {
		prev->next = new_node(val, next, 0);
	}
	return result;
}


int set_add(intset_t *set, val_t val, int transactional)
{
	int result;

#ifdef DEBUG
	printf("++> set_add(%d)\n", (int)val);
	IO_FLUSH;
#endif

	if (!transactional) {

		result = set_seq_add(set, val);

	} else {

#ifdef SEQUENTIAL /* Unprotected */

		result = set_seq_add(set, val);

#elif defined STM

		node_t *prev, *next;
		val_t v;


	LARGE_TX_BEGIN(set->head);
	prev = set->head;
	SET_ADD_TX:
  while (1) {
		next = (node_t *)TX_LOAD(&prev->next);
    v = TX_LOAD((uintptr_t *) &next->val);
    if (v >= val)
      break;
		LARGE_TX_BREAK;
    prev = next;
    //next = (node_t *)TX_LOAD(&prev->next);
  }
  result = (v != val);
  if (result) {
		LARGE_TX_BEFORE_ADD(SET_ADD_TX);
    TX_STORE(&prev->next, new_node(val, next, transactional));
  }
	LARGE_TX_END;

#elif defined LOCKFREE
		result = harris_insert(set, val);
#endif

	}

	return result;
}

int set_remove(intset_t *set, val_t val, int transactional)
{
	int result = 0;

#ifdef DEBUG
	printf("++> set_remove(%d)\n", (int)val);
	IO_FLUSH;
#endif

#ifdef SEQUENTIAL /* Unprotected */

	node_t *prev, *next;

	prev = set->head;
	next = prev->next;
	while (next->val < val) {
		prev = next;
		next = prev->next;
	}
	result = (next->val == val);
	if (result) {
		prev->next = next->next;
		free(next);
	}

#elif defined STM

	node_t *prev, *next;
	val_t v;
	node_t *n;


	LARGE_TX_BEGIN(set->head);
  prev = set->head;
	SET_REMOVE_TX:
  while (1) {
		next = (node_t *)TX_LOAD(&prev->next);
    v = TX_LOAD((uintptr_t *) &next->val);
    if (v >= val)
      break;
		LARGE_TX_BREAK;
		prev = next;
    //next = (node_t *)TX_LOAD(&prev->next);
  }
  result = (v == val);
  if (result) {
		LARGE_TX_BEFORE_REMOVE(SET_REMOVE_TX);
    n = (node_t *)TX_LOAD(&next->next);
    TX_STORE(&prev->next, n);
		LARGE_TX_END;
		FREE(next, sizeof(node_t));
  }
	else {
		LARGE_TX_END;
	}	

#elif defined LOCKFREE
	result = harris_delete(set, val);
#endif

	return result;
}
