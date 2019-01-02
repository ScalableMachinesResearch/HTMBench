/* =============================================================================
 *
 * client.c
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */
/* Copyright (c) IBM Corp. 2014. */


#include <assert.h>
#include "action.h"
#include "client.h"
#include "manager.h"
#include "reservation.h"
#include "thread.h"
#include "types.h"


/* =============================================================================
 * client_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
client_t*
client_alloc (long id,
              manager_t* managerPtr,
              long numOperation,
              long numQueryPerTransaction,
              long queryRange,
              long percentUser)
{
    client_t* clientPtr;

    clientPtr = (client_t*)malloc(sizeof(client_t));
    if (clientPtr == NULL) {
        return NULL;
    }

    clientPtr->randomPtr = random_alloc();
    if (clientPtr->randomPtr == NULL) {
        return NULL;
    }

    clientPtr->id = id;
    clientPtr->managerPtr = managerPtr;
    random_seed(clientPtr->randomPtr, id);
    clientPtr->numOperation = numOperation;
    clientPtr->numQueryPerTransaction = numQueryPerTransaction;
    clientPtr->queryRange = queryRange;
    clientPtr->percentUser = percentUser;

    return clientPtr;
}


/* =============================================================================
 * client_free
 * =============================================================================
 */
void
client_free (client_t* clientPtr)
{
    free(clientPtr);
}


/* =============================================================================
 * selectAction
 * =============================================================================
 */
static action_t
selectAction (long r, long percentUser)
{
    action_t action;

    if (r < percentUser) {
        action = ACTION_MAKE_RESERVATION;
    } else if (r & 1) {
        action = ACTION_DELETE_CUSTOMER;
    } else {
        action = ACTION_UPDATE_TABLES;
    }

    return action;
}


/* =============================================================================
 * client_run
 * -- Execute list operations on the database
 * =============================================================================
 */
void
client_run (void* argPtr)
{
    TM_THREAD_ENTER();

    long myId = thread_getId();
    client_t* clientPtr = ((client_t**)argPtr)[myId];

    manager_t* managerPtr = clientPtr->managerPtr;
    random_t*  randomPtr  = clientPtr->randomPtr;

    long numOperation           = clientPtr->numOperation;
    long numQueryPerTransaction = clientPtr->numQueryPerTransaction;
    long queryRange             = clientPtr->queryRange;
    long percentUser            = clientPtr->percentUser;

    long* types  = (long*)P_MALLOC(numQueryPerTransaction * sizeof(long));
    long* ids    = (long*)P_MALLOC(numQueryPerTransaction * sizeof(long));
    long* ops    = (long*)P_MALLOC(numQueryPerTransaction * sizeof(long));
    long* prices = (long*)P_MALLOC(numQueryPerTransaction * sizeof(long));
    reservation_t** reservations = (reservation_t**)P_MALLOC( numQueryPerTransaction* sizeof(reservation_t*));
    long i;

    for (i = 0; i < numOperation; i++) {

        long r = random_generate(randomPtr) % 100;
        action_t action = selectAction(r, percentUser);

        switch (action) {

            case ACTION_MAKE_RESERVATION: {
                long maxPrices[NUM_RESERVATION_TYPE] = { -1, -1, -1 };
                long maxIds[NUM_RESERVATION_TYPE] = { -1, -1, -1 };
                long n;
                long numQuery = random_generate(randomPtr) % numQueryPerTransaction + 1;
                long customerId = random_generate(randomPtr) % queryRange + 1;
                for (n = 0; n < numQuery; n++) {
                    types[n] = random_generate(randomPtr) % NUM_RESERVATION_TYPE;
                    ids[n] = (random_generate(randomPtr) % queryRange) + 1;
                }
                bool_t isFound = FALSE;
                TM_BEGIN_ID(0);
                for (n = 0; n < numQuery; n++) {
                    long t = types[n];
                    long id = ids[n];
                    long price = -1;
		    reservation_t *reservationPtr = NULL;
                    switch (t) {
                        case RESERVATION_CAR:
			    reservationPtr =(reservation_t *)TMMAP_FIND(managerPtr->carTablePtr,id);
			    break;
                        case RESERVATION_FLIGHT:
                            reservationPtr =(reservation_t *)TMMAP_FIND(managerPtr->flightTablePtr,id);
                            break;
                        case RESERVATION_ROOM:
                            reservationPtr =(reservation_t *)TMMAP_FIND(managerPtr->roomTablePtr,id);
                            break;
                        default:
                            assert(0);
                    }
                    if (reservationPtr != NULL && (long)TM_SHARED_READ(reservationPtr->numFree)>0 ){
                        price = (long)TM_SHARED_READ(reservationPtr->price);
                    }
                    if (price > maxPrices[t]) {
                        maxPrices[t] = price;
                        maxIds[t] = id;
			reservations[t] = reservationPtr;
                        isFound = TRUE;
                    }
                } /* for n */
		customer_t* customerPtr = NULL;
                if (isFound) {
		    bool_t status;
		    customerPtr = (customer_t*)TMMAP_FIND(managerPtr->customerTablePtr, customerId);
		    if (customerPtr == NULL) {
			customerPtr = CUSTOMER_ALLOC(customerId);
    			assert(customerPtr != NULL);
   			status = TMMAP_INSERT(managerPtr->customerTablePtr, customerId, customerPtr);
   			if (status == FALSE) {
				TM_RESTART();
			}
		    }
                }
                if (maxIds[RESERVATION_CAR] > 0) {
			myReserveCar(customerPtr, reservations[RESERVATION_CAR],maxIds[RESERVATION_CAR]);			
                }
                if (maxIds[RESERVATION_FLIGHT] > 0) {
                	myReserveFlight(customerPtr, reservations[RESERVATION_FLIGHT],maxIds[RESERVATION_FLIGHT]);
		}
                if (maxIds[RESERVATION_ROOM] > 0) {
                	myReserveRoom(customerPtr, reservations[RESERVATION_ROOM],maxIds[RESERVATION_ROOM]);
		}

                TM_END();
                break;
            }

            case ACTION_DELETE_CUSTOMER: {
                long customerId = random_generate(randomPtr) % queryRange + 1;
                TM_BEGIN_ID(1);
                long bill = MANAGER_QUERY_CUSTOMER_BILL(managerPtr, customerId);
                if (bill >= 0) {
                    MANAGER_DELETE_CUSTOMER(managerPtr, customerId);
                }
                TM_END();
                break;
            }

            case ACTION_UPDATE_TABLES: {
                long numUpdate = random_generate(randomPtr) % numQueryPerTransaction + 1;
                long n;
                for (n = 0; n < numUpdate; n++) {
                    types[n] = random_generate(randomPtr) % NUM_RESERVATION_TYPE;
                    ids[n] = (random_generate(randomPtr) % queryRange) + 1;
                    ops[n] = random_generate(randomPtr) % 2;
                    if (ops[n]) {
                        prices[n] = ((random_generate(randomPtr) % 5) * 10) + 50;
                    }
                }
                TM_BEGIN_ID(2);
                for (n = 0; n < numUpdate; n++) {
                    long t = types[n];
                    long id = ids[n];
                    long doAdd = ops[n];
                    if (doAdd) {
                        long newPrice = prices[n];
                        switch (t) {
                            case RESERVATION_CAR:
                                MANAGER_ADD_CAR(managerPtr, id, 100, newPrice);
                                break;
                            case RESERVATION_FLIGHT:
                                MANAGER_ADD_FLIGHT(managerPtr, id, 100, newPrice);
                                break;
                            case RESERVATION_ROOM:
                                MANAGER_ADD_ROOM(managerPtr, id, 100, newPrice);
                                break;
                            default:
                                assert(0);
                        }
                    } else { /* do delete */
                        switch (t) {
                            case RESERVATION_CAR:
                                MANAGER_DELETE_CAR(managerPtr, id, 100);
                                break;
                            case RESERVATION_FLIGHT:
                                MANAGER_DELETE_FLIGHT(managerPtr, id);
                                break;
                            case RESERVATION_ROOM:
                                MANAGER_DELETE_ROOM(managerPtr, id, 100);
                                break;
                            default:
                                assert(0);
                        }
                    }
                }
                TM_END();
                break;
            }

            default:
                assert(0);

        } /* switch (action) */

    } /* for i */

    TM_THREAD_EXIT();
}


/* =============================================================================
 *
 * End of client.c
 *
 * =============================================================================
 */




