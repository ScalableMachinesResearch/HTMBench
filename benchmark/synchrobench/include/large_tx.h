#ifndef _LARGE_TX_H
#define _LARGE_TX_H

#define RELEASE_NODE_LOCK(node_ptr) if(node_ptr){node_ptr->lock = 0;}
#define RELEASE_NODE_LOCK_IF_ACQUIRED(node_ptr) if(node_ptr){ if(node_ptr->lock) node_ptr->lock = 0;}

/* You need to define the following things
#define LTX_NODE_TYPE
#define ITERATION_NUMBER_GENERATOR //(rand()%30 + 10)
*/

#define LARGE_TX_BEGIN(starting_node) \
int iteration = 0;\
int intended_iteration=ITERATION_NUMBER_GENERATOR;\
TM_BEGIN();\
LTX_NODE_TYPE *break_node = starting_node;

#define LARGE_TX_END RELEASE_NODE_LOCK(break_node);TM_END();

#define LARGE_TX_BREAK \
if (iteration++ >= intended_iteration){\
	iteration = 0;\
	if (TM_TEST()){\
		if (prev->lock){\
				TM_ABORT();\
		}\
		else {\
			prev->lock = 1;\
			RELEASE_NODE_LOCK(break_node);\
			break_node = prev;\
			TM_END();TM_BEGIN();\
		}\
	}\
	else {\
		if (prev->lock) {\
			prev = break_node;\
      TM_END();\
      intended_iteration=ITERATION_NUMBER_GENERATOR;\
      TM_BEGIN_FALLBACK();\
		}\
		else {\
			prev->lock = 1;\
			RELEASE_NODE_LOCK(break_node);\
			break_node = prev;\
      TM_END();TM_BEGIN();\
		}\
	}\
}

#define LARGE_TX_ACQUIRE_ONE_NODE_LOCK_AND_END(node1, RETRY_POINT) \
if (TM_TEST()){\
	if (node1->lock){\
			TM_ABORT();\
	}\
	else {\
		node1->lock = 1;\
		RELEASE_NODE_LOCK(break_node);\
		TM_END();\
	}\
}\
else {\
	if (node1->lock) {\
		prev = break_node;\
		TM_END();\
		TM_BEGIN_FALLBACK();\
		goto RETRY_POINT;\
	}\
	else {\
		node1->lock = 1;\
		RELEASE_NODE_LOCK(break_node);\
		TM_END();\
	}\
}

#define LARGE_TX_ACQUIRE_TWO_NODES_LOCK_AND_END(node1, node2, RETRY_POINT) \
if (TM_TEST()){\
	if (node1->lock || node2->lock){\
			TM_ABORT();\
	}\
	else {\
		node1->lock = node2->lock = 1;\
		RELEASE_NODE_LOCK(break_node);\
		TM_END();\
	}\
}\
else {\
	if (node1->lock || node2->lock) {\
		prev = break_node;\
		TM_END();\
		TM_BEGIN_FALLBACK();\
		goto RETRY_POINT;\
	}\
	else {\
		node1->lock = node2->lock = 1;\
		RELEASE_NODE_LOCK(break_node);\
		TM_END();\
	}\
}

#define LARGE_TX_BEFORE_ADD(RETRY_POINT) \
if (TM_TEST()) {\
	if (prev->lock != 0) TM_ABORT();\
}\
else {\
	if (prev->lock) {\
		prev = break_node;\
    TM_END();\
    TM_BEGIN_FALLBACK();\
		goto RETRY_POINT;\
	}\
	else {\
		RELEASE_NODE_LOCK(break_node);\
	}\
}

#define LARGE_TX_BEFORE_REMOVE(RETRY_POINT) \
if (TM_TEST()) {\
	if (prev->lock != 0 || next->lock !=0) TM_ABORT();\
}\
else {\
	if (prev->lock || next->lock) {\
		prev = break_node;\
    TM_END();\
    TM_BEGIN_FALLBACK();\
		goto RETRY_POINT;\
	}\
	else {\
		RELEASE_NODE_LOCK(break_node);\
	}\
}

#define LARGE_TX_BEFORE_SUM(RETRY_POINT) \
if (TM_TEST()) {\
	if (prev->lock != 0) TM_ABORT();\
}\
else {\
	if (prev->lock) {\
		prev = break_node;\
		sum = sum_backup;\
    TM_END();\
    TM_BEGIN_FALLBACK();\
		goto RETRY_POINT;\
	}\
	else {\
		RELEASE_NODE_LOCK(break_node);\
	}\
}

#define LARGE_TX_BREAK_SUM \
if (iteration++ >= intended_iteration){\
	iteration = 0;\
	if (TM_TEST()){\
		if (prev->lock){\
				TM_ABORT();\
		}\
		else {\
			prev->lock = 1;\
			RELEASE_NODE_LOCK(break_node);\
			break_node = prev;\
			sum_backup = sum;\
			TM_END();TM_BEGIN();\
		}\
	}\
	else {\
		if (prev->lock) {\
			prev = break_node;\
			sum = sum_backup;\
      TM_END();\
      intended_iteration=ITERATION_NUMBER_GENERATOR;\
      TM_BEGIN_FALLBACK();\
		}\
		else {\
			prev->lock = 1;\
			RELEASE_NODE_LOCK(break_node);\
			break_node = prev;\
			sum_backup = sum;\
      TM_END();TM_BEGIN();\
		}\
	}\
}


#endif /* _LARGE_TX_H */
