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
 * concurrent access of two competing threads
 * to a shared data object, which results in a race
 * condition.
 * The example reveals a simple read, copy, modify
 * and update sequence of operations with no mutual
 * exclusion taking place.
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
 * 1. Is the proposed example understandable and clear?
 * 2. Can you think of a simpler way to code the competing threads body of work?
 * 3. Is the code style appropriate?
 * 4. Can you point out an issue with the provided Makefile? If yes,
 * could you prepare an improved version of the Makefile?
 * 5. Try to answer the inline questions below as well.
 */

/* Primitive error print and exit function. */
static inline void exit_err(int err, const char *msg) {
/* Be careful, if you need to free resources beforehand. */
	if (err != 0) {
		/* errno is thread local - is this model portable and guaranteed? */
		/* Hint: consult the GNU GCC documentation */
		fprintf(stderr, "errno value: %d\n", errno);
		exit(EXIT_FAILURE);
	} else {
		exit(EXIT_SUCCESS);
	}
}

/* Helper function for randomized timed sleep */
static inline int random_sleep() {
	int ret = MAX_SLEEP_TIME_SECS;
	struct timespec tms;

	/* Is the referenced clock resolution implementation specific? */
	ret = clock_gettime(CLOCK_MONOTONIC, &tms);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "Error calling clock_gettime!\n");
	} else {
		/* Why does this look so complicated? */
		/* Could you provide a simpler alternative? */
		srand((unsigned)
		      (tms.tv_sec ^ tms.tv_nsec ^ (tms.tv_nsec >> 31)));
	}
	ret = rand() % MAX_SLEEP_TIME_SECS;

	return ret;
}

/* Body of the first competing thread (process) */
/* The example assumes a thread is equivalent to a lightweight process */
void *consumer_one_work(void *data_ptr) {
	/* Is this explicit pointer initialization necessary here? */
	struct shared_block *sb_ptr = NULL;
	int sleep_ts = 0;
	int temp = 0;

	if (data_ptr == NULL) {
		fprintf(stdout, "%s null data ptr!\n", __func__);
		pthread_exit((void *)sb_ptr);
	}

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);
	/* fprintf(stdout, "%s: found sb_ptr at address: %p\n",
	   __func__, sb_ptr); */

	/* Give the second thread chance to run and access the shared data */
	sleep_ts = random_sleep();
	fprintf(stdout, "%s sleeping for: %d secs\n", __func__, sleep_ts);
	/* Make a local copy of the pointed integer variable */
	temp = sb_ptr->buffer;
	sleep(sleep_ts);

	/* Access and modify a local copy of the shared integer data */
	if (temp > 0) {
		temp -= 10;
	}

	/*
	 * Update the shared data reference with the local value, exposing
	 * the indeterminate result of the simulated race condition.
	 */
	sb_ptr->buffer = temp;
	fprintf(stdout, "%s complete\n", __func__);
	pthread_exit((void *)sb_ptr);
}


/* Body of the second competing thread */
void *consumer_two_work(void *data_ptr) {
	struct shared_block *sb_ptr = NULL;
	int sleep_ts = 0;
	int temp = 0;

	if (data_ptr == NULL) {
		fprintf(stdout, "%s null data ptr!\n", __func__);
		pthread_exit((void *)sb_ptr);
	}

	/* Obtain the reference to the shared data object */
	sb_ptr = (struct shared_block *)(data_ptr);
	/* fprintf(stdout, "%s: found sb_ptr at address: %p\n",
	   __func__, sb_ptr); */

	/* Give the first thread chance to run and access the shared data */
	sleep_ts = random_sleep();
	fprintf(stdout, "%s sleeping for: %d secs\n", __func__, sleep_ts);
	/* Make a local copy of the pointed integer variable */
	temp = sb_ptr->buffer;
	sleep(sleep_ts);

	/* Access and modify a local copy of the shared integer data */
	if (temp > 0) {
		temp -= 6;
	}

	/*
	 * Update the shared data reference with the local value, exposing
	 * the indeterminate result of the simulated race condition.
	 */
	sb_ptr->buffer = temp;
	fprintf(stdout, "%s complete\n", __func__);
	pthread_exit((void *)sb_ptr);
}

int init_thr_attribute(pthread_attr_t * attr) {
	int ret = EXIT_FAILURE;

	/* Can you think of a better way for error handling? */
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

	/* Can you make it more flexible, once worker thread logic is generalized? */
	res[0] =
	    pthread_create(&thr[0], attr, consumer_one_work, (void *)sb_ptr);
	res[NUM_THREADS - 1] =
	    pthread_create(&thr[NUM_THREADS - 1], attr, consumer_two_work,
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

int main(int argc, char *argv[]) {

	int ret = EXIT_SUCCESS;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t atrb;
	struct shared_block sb;
	void *thr_join_status;

	/* Initialize the shared data integer variable */
	sb.buffer = 10;

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
