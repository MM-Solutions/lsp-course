#ifndef THREAD_OPS_H
#define THREAD_OPS_H

/*
 * The file lists the helper Pthread wrapper API functions
 * to be used from within the main C application.
 * The recommended sequence of calls should be:
 * 1) init_thr_workers - fills in the common config data shared
 * among all the worker threads
 * 2) run_thr_workers - sets up all of the worker threads, starts
 * them and waits till all of the workers get finished
 * 3) get_thr_result - add up the partially calculated dot products
 * once the worker threads are completed and return the final result
 * 4) destroy_thr_workers - serves as a thread array destructor
 */

/*
 * Used for setting up all of the worker threads with the proper
 * data configuration needed.
 * Arguments expected:
 * thr_ptr - an array of struct thread_arr_cfg objects
 * sb_ptr - pointer to a shared memory configuration object
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
int init_thr_workers(struct thread_arr_cfg *thr_ptr,
		     struct shared_block *sb_ptr);

/*
 * Used for starting all of the worker threads and waiting till
 * all of the workers are completed.
 * Arguments expected:
 * thr_ptr - an array of struct thread_arr_cfg objects
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
int run_thr_workers(struct thread_arr_cfg *thr_ptr);

/*
 * Used for obtaining the final dot product result.
 * Arguments expected:
 * thr_ptr - an array of struct thread_arr_cfg objects
 * Returns a float value
 */
float get_thr_result(struct thread_arr_cfg *thr_ptr);

/*
 * Used for freeing the requested dynamically configured attributes.
 * Arguments expected:
 * thr_ptr - an array of struct thread_arr_cfg objects
 */
void destroy_thr_workers(struct thread_arr_cfg *thr_ptr);
#endif
