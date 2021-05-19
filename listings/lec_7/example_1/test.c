#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <fcntl.h>
#include "test.h"

/*
 * The purpose of the program is to help illustrate
 * a very simple example of mutual exclusion
 * between two competing threads using mutexes
 * instead of the Peterson's algorithm.
 * The code presented in based on:
 * https://github.com/MM-Solutions/lsp-course/tree/main/listings/lec_7/example_1
 * and builds on the example sources from lec_6.
 *
 * The program is to be used for educational purposes
 * only and the author(s) are not responsible for any
 * damage or consequences as a result of using this
 * example.
 * These sources are part of the resources, accompanying
 * the MM-Solutions Linux System Programming course:
 * https://github.com/MM-Solutions/lsp-course
 */

/*
 * Questions:
 * 1. Is the proposed example more efficient compared to
 * the one with the original version of the Peterson's algorithm?
 * 2. Could you rework the example using Posix semaphores instead of
 * mutexes?
 * 3. Is it necessary for a thread to always block when competing for
 * a resource? Can you try out a variation of the example using the trylock
 * versions of the available mutex_lock functions?
 * 4. Try to answer the inline questions below as well.
 */

/* Initiliaze mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Primitive error print and exit function. */
static inline void exit_err(int err, const char *msg) {

	if (err != 0) {
		fprintf(stderr, "errno value: %d\n", errno);
		exit(EXIT_FAILURE);
	} else {
		exit(EXIT_SUCCESS);
	}
}

/* Helper function for randomized timed sleep as in the previous example */
static inline int random_sleep() {
	int ret = MAX_SLEEP_TIME_SECS;
	struct timespec tms;

	ret = clock_gettime(CLOCK_MONOTONIC, &tms);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "Error calling clock_gettime!\n");
	} else {
		srand((unsigned)(tms.tv_sec ^ tms.tv_nsec
				 ^ (tms.tv_nsec >> 31)));
	}
	ret = rand() % MAX_SLEEP_TIME_SECS;

	return ret;
}

void update_int_data(int *int_ptr, int val) {
	int temp = 0;
	int sleep_ts = 0;

	if (NULL == int_ptr) {
		fprintf(stderr, "%s: null int ptr arg found!\n", __func__);
		return;
	}
	if (val == 0) {
		fprintf(stderr, "%s: expected a nonzero value!\n",
			__func__);
		return;
	}
	sleep_ts = random_sleep();
	/* Copying here is redundant, but left as in the previous example */
	temp = *int_ptr;
	sleep(sleep_ts);

	/* Modify the shared resource data copy */
	if (temp > 0)
		temp += val;

	/* Update the shared data variable reference with the local copy */
	*int_ptr = temp;
}

/* Body of the first competing thread (process) */
/* The example assumes a thread is equivalent to a lightweight process */
void *consumer_one_work(void *data_ptr) {
	struct shared_block *sb_ptr = NULL;

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);

	/* Lock the mutex */
	pthread_mutex_lock(&mutex);

	/* Enter the critical section and operate on the shared resource safely */
	update_int_data(&sb_ptr->buffer, CONSUMER_ONE_VALUE);

	/* Exit the critical section and release lock */
	pthread_mutex_unlock(&mutex);

	fprintf(stdout, "%s complete\n", __func__);
	pthread_exit((void *)sb_ptr);
}

/* Body of the second competing thread */
void *consumer_two_work(void *data_ptr) {
	struct shared_block *sb_ptr = NULL;

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);

	/* Lock the mutex */
	pthread_mutex_lock(&mutex);

	/* Enter the critical section and operate on the shared resource safely */
	update_int_data(&sb_ptr->buffer, CONSUMER_TWO_VALUE);

	/* Exit the critical section and release lock */
	pthread_mutex_unlock(&mutex);

	fprintf(stdout, "%s complete\n", __func__);
	pthread_exit((void *)sb_ptr);
}

int init_thr_attribute(pthread_attr_t * attr) {
	int ret = EXIT_FAILURE;

	if (NULL == attr) {
		fprintf(stderr, "%s: null ptr arg found!\n", __func__);
		return ret;
	}
	ret = pthread_attr_init(attr);
	if (ret != EXIT_SUCCESS) {
		return ret;
	}
	pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
	ret = EXIT_SUCCESS;

	return ret;
}

int destroy_thr_attribute(pthread_attr_t * attr) {
	int ret = EXIT_FAILURE;

	if (attr != NULL) {
		ret = pthread_attr_destroy(attr);
		if (ret != EXIT_SUCCESS) {
			fprintf(stderr, "%s: error destroying attributes!\n",
				__func__);
		}
	}
	return ret;
}

/* Create the competing threads */
int create_thr_array(pthread_t * thr, pthread_attr_t * attr,
		     struct shared_block *sb_ptr) {
	work_func_t worker_arr[NUM_THREADS] = {
		consumer_one_work,
		consumer_two_work
	};
	int ret = EXIT_FAILURE;
	int res[NUM_THREADS];
	int idx;

	if (NULL == thr || NULL == attr) {
		fprintf(stderr, "%s: null ptr arg found!\n", __func__);
		return ret;
	}
	if (NULL == sb_ptr) {
		fprintf(stderr, "%s: null shared ptr arg found!\n", __func__);
		return ret;
	}
	/* Left for simplicity */
	/* Could you rework even more and simplify thread creation? */
	if (NUM_THREADS != 2) {
		fprintf(stderr, "%s: expected 2 threads!\n", __func__);
		return ret;
	}

	for (idx = 0; idx < NUM_THREADS; idx++) {
		res[idx] = pthread_create(&thr[idx], attr, worker_arr[idx],
					  (void *)sb_ptr);
		if (res[idx] != EXIT_SUCCESS) {
			fprintf(stderr, "%s: error creating thread: %d!\n",
				__func__, idx);
			return ret;
		}
	}
	ret = EXIT_SUCCESS;

	return ret;
}

/* Wait for both worker threads to finish execution */
int wait_thr_complete(pthread_t * thr, void **state) {
	int ret = EXIT_FAILURE;
	int idx;
	fprintf(stdout, "main thread waiting for child threads...\n");
	for (idx = 0; idx < NUM_THREADS; idx++) {
		ret = pthread_join(thr[idx], state);
		if (ret != EXIT_SUCCESS) {
			exit_err(ret, "error when joining threads!");
		}
	}
	fprintf(stdout, "main thread waiting completed\n");
	return ret;
}

void init_shared_data(struct shared_block *sb_ptr) {

	if (NULL == sb_ptr) {
		fprintf(stderr, "%s: null shared ptr arg found!\n", __func__);
		exit_err(-1, "shared data pointer is null");
	}

	/* Provide initial value for the shared data buffer */
	sb_ptr->buffer = 10;

	/* I don't think a memory barrier is need here or in this example at all */
}

int main(int argc, char *argv[]) {

	int ret = EXIT_SUCCESS;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t atrb;
	struct shared_block sb;
	void *thr_join_status;

	fprintf(stdout, "%s thread pid: %d\n", __func__, getpid());

	/* Initialize the shared data block */
	init_shared_data(&sb);

	/* Start with filling in the thread attributes */
	ret = init_thr_attribute(&atrb);
	if (ret != EXIT_SUCCESS) {
		exit_err(ret, "issue when calling: init_thr_attribute");
	}

	/* Create and run both consumer threads */
	ret = create_thr_array(threads, &atrb, &sb);
	if (ret != EXIT_SUCCESS) {
		exit_err(ret, "issue when creating child threads");
	}

	/* Let the main thread wait for child completion */
	ret = wait_thr_complete(threads, &thr_join_status);
	if (ret != EXIT_SUCCESS) {
		/* No need to examine error code here */
		destroy_thr_attribute(&atrb);
		exit_err(ret, "issue when creating child threads");
	}

	/* Destroy the thread attributes */
	ret = destroy_thr_attribute(&atrb);
	if (ret != EXIT_SUCCESS) {
		exit_err(ret, "issue when calling: init_thr_attribute");
	}

	/* Finally, print out the value of the shared data variable */
	fprintf(stdout, "%s shared data buffer value is: %d\n",
		__func__, sb.buffer);

	return ret;
}
