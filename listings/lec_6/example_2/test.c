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
 * a high-level software example of mutual exclusion
 * between two processes using the Peterson's algorithm.
 * The code presented in based on:
 * https://en.wikipedia.org/wiki/Peterson%27s_algorithm
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
 * 1. Is the proposed program understandable and clear?
 * 2. Try to answer the inline questions below as well.
 */

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

/* Body of the first competing thread (process) */
/* The example assumes a thread is equivalent to a lightweight process */
void *consumer_one_work(void *data_ptr) {
	struct shared_block *sb_ptr = NULL;
	int sleep_ts = 0;
	int temp = 0;

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);

	/* Give priority to the second process, if it has become ready */
	sb_ptr->ready[0] = true;
	sb_ptr->turn_idx = 1;

	/* Attempt to place a memory barrier */
	/* Is this specialized memory access synchronization necessary? */
	/* Can you think of any implementation defined constraints? */
	__sync_synchronize();

	/* Busy wait, if the second process is active */
	/* This helps ensure a basic form of mutual exclusion */
	while (sb_ptr->ready[1] == true && sb_ptr->turn_idx == 1) {
		fprintf(stdout, "thread: %d blocked and waiting\n", 0);
		sleep(1);
		;
	}

	/* Enter into the critical section and manipulate the data safely */
	sleep_ts = random_sleep();
	temp = sb_ptr->buffer;
	sleep(sleep_ts);

	/* Access and modify the shared resource data copy */
	if (temp > 0) {
		temp -= 9;
	}

	/* Update the shared data variable reference with the local copy */
	sb_ptr->buffer = temp;

	/* Exit the critical section and let the other process become unblocked */
	sb_ptr->ready[0] = false;
	__sync_synchronize();

	fprintf(stdout, "%s complete\n", __func__);
	pthread_exit((void *)sb_ptr);
}

/* Body of the second competing thread */
void *consumer_two_work(void *data_ptr) {
	struct shared_block *sb_ptr = NULL;
	int sleep_ts = 0;
	int temp = 0;

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);

	/* Give priority to the first process, if it has become ready */
	sb_ptr->ready[1] = true;
	sb_ptr->turn_idx = 0;

	/* Attempt to place a memory barrier */
	__sync_synchronize();

	/* Busy wait, if the first process is active */
	/* This helps ensure a basic form of mutual exclusion */
	while (sb_ptr->ready[0] == true && sb_ptr->turn_idx == 0) {
		fprintf(stdout, "thread: %d blocked and waiting\n", 1);
		sleep(1);
		;
	}
	/* Enter into the critical section and manipulate the data safely */
	sleep_ts = random_sleep();
	temp = sb_ptr->buffer;
	sleep(sleep_ts);

	/* Access and modify the shared resource dara copy */
	if (temp > 0) {
		temp += 6;
	}

	/* Update the shared data variable reference with the local copy */
	sb_ptr->buffer = temp;

	/* Exit the critical section and let the other process become unblocked */
	sb_ptr->ready[1] = false;
	__sync_synchronize();

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
	if (NUM_THREADS != 2) {
		fprintf(stderr, "%s: expected 2 threads!\n", __func__);
		return ret;
	}
	res[0] = pthread_create(&thr[0], attr,
				consumer_one_work, (void *)sb_ptr);
	res[NUM_THREADS - 1] = pthread_create(&thr[NUM_THREADS - 1], attr,
					      consumer_two_work,
					      (void *)sb_ptr);

	for (idx = 0; idx < NUM_THREADS; idx++) {
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

	for (idx = 0; idx < NUM_THREADS; idx++) {
		ret = pthread_join(thr[idx], state);
		if (ret != EXIT_SUCCESS) {
			exit_err(ret, "error when joining threads!");
		}
	}
	fprintf(stdout, "%s main thread completed\n", __func__);
	return ret;
}

void init_shared_data(struct shared_block *sb_ptr) {
	int idx;

	if (NULL == sb_ptr) {
		fprintf(stderr, "%s: null shared ptr arg found!\n", __func__);
		exit_err(-1, "shared data pointer is null");
	}
	/* Reset ready flags here */
	for (idx = 0; idx < NUM_THREADS; idx++) {
		sb_ptr->ready[idx] = false;
	}
	/* Provide initial values for the shared data variables */
	sb_ptr->buffer = 10;
	/* Unblock the first thread only */
	sb_ptr->turn_idx = 1;
	/* Attempt to place a memory barrier */
	__sync_synchronize();
}

int main(int argc, char *argv[]) {

	int ret = EXIT_SUCCESS;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t atrb;
	struct shared_block sb;
	void *thr_join_status;

	fprintf(stdout, "%s main thread pid: %d\n", __func__, getpid());

	/* Initializa the shared data block */
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
