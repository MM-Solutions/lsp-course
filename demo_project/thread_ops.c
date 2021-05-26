#include "test.h"
#include "thread_ops.h"

/*
 * Common thread barrier wait helper function.
 * Prints are not necessary and have been included for
 * clarity.
 * Is the fprintf function used below reentrant and safe
 * to use in a multithreaded evnironment without proper locking?
 */
static void wait_thr_barrier(pthread_barrier_t * barrier, off_t ind) {
	int ret;

	fprintf(stdout, "%s called from thread with index: %ld\n",
		__func__, ind);

	ret = pthread_barrier_wait(barrier);

	if (0 == ret) {
		fprintf(stdout, "%s thread with index: %ld passed \
			the barrier\n", __func__, ind);
		usleep(SLEEP_INT);
	} else if (ret == PTHREAD_BARRIER_SERIAL_THREAD) {
		fprintf(stdout, "%s thread with index: %ld passed \
		the barrier with SERIAL_THREAD \n", __func__, ind);
		usleep(SLEEP_INT);
	} else {
		fprintf(stderr, "%s thread with index: %ld encountered \
		error during pthread_barrier_wait\n", __func__, ind);
		usleep(SLEEP_INT);
	}

	/* pthread_barrier_wait(barrier); */
}

/* Function body of each worker thread */
void *worker_thread(void *data_ptr) {

	struct thread_arr_cfg *thr_ptr = NULL;
	off_t ind;
	struct vect_coord_pair *buf;
	float op_a, op_b;

	/* Obtain the reference to the shared data config */
	thr_ptr = (struct thread_arr_cfg *)(data_ptr);

	/* Perform some basic pointer checking */
	if (NULL == thr_ptr || NULL == thr_ptr->sb_ptr
	    || NULL == thr_ptr->sb_ptr->buffer) {
		goto exit;
	}

	buf = (struct vect_coord_pair *)thr_ptr->sb_ptr->buffer;

	if (thr_ptr->lower == -1 || thr_ptr->upper == -1) {
		goto exit;
	}

	/* Index the mmapped file contents and operate on assigned slice */
	for (ind = thr_ptr->lower; ind < thr_ptr->upper; ind++) {
		/* fprintf(stdout, "coord one: %.*s, coord two: %.*s\n",
		   (int)sizeof(buf[ind].vec_a), buf[ind].vec_a,
		   (int)sizeof(buf[ind].vec_a), buf[ind].vec_b); */
		/* Is the parsing mechanics appropriate? */
		op_a = strtof(buf[ind].vec_a, NULL);
		op_b = strtof(buf[ind].vec_b, NULL);
		/*fprintf(stdout, "thread: %ld float operand one: %5.2f,\
		   float operand two: %5.2f\n", thr_ptr->thr_index,
		   op_a, op_b); */
		thr_ptr->part_dot_sum += op_a * op_b;
	}
 exit:
	wait_thr_barrier(&thr_ptr->sb_ptr->barrier, thr_ptr->thr_index);

	/* fprintf(stdout, "%s complete\n", __func__); */
	pthread_exit((void *)thr_ptr);
}

/* Static helper for initializing the common thread attribute */
static int init_thr_attribute(pthread_attr_t * attr) {
	int ret = EXIT_FAILURE;

	if (NULL == attr) {
		fprintf(stderr, "%s: null ptr arg found!\n", __func__);
		goto exit;
	}
	ret = pthread_attr_init(attr);
	if (ret != EXIT_SUCCESS) {
		goto exit;
	}
	pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
	ret = EXIT_SUCCESS;

 exit:
	return ret;
}

/* Static helper for destroying the common thread attribute */
static int destroy_thr_attribute(pthread_attr_t * attr) {
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

/*
 * Static helper to fill in data attributes for each worker
 * thread during the initialization sequence in init_thr_config
 * below.
 * No error result gets generated and dispatched, because at this
 * point all required data is assumed to be present and provided.
 * Arguments expected:
 * thr_ptr - pointer to a struct thread_arr_cfg object
 * lower - index of the starting offset into the mmapped file
 * upper - index of the ending offset into the mmapped file
 * index - assigned worker thread index
 * val - initial partial dot product value
 * work_func_t - function pointer to assigned thread worker
 * sb_ptr - pointer to the shared memory configuration
 */
static void create_thr(struct thread_arr_cfg *thr,
		       off_t lower,
		       off_t upper,
		       off_t index,
		       float val,
		       void *(*work_func_t) (void *ptr),
		       struct shared_block *sb_ptr) {
	thr->lower = lower;
	thr->upper = upper;
	thr->thr_index = index;
	thr->part_dot_sum = val;
	thr->work_func_t = work_func_t;
	thr->sb_ptr = sb_ptr;
}

/*
 * Static helper to fill in the config parameters during worker
 * thread construction.
 * Arguments expected are:
 * thr_ptr - an array of struct thread_arr_cfg objects
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
static int init_thr_config(struct thread_arr_cfg *thr_ptr) {

	/* Calculated total number of coordinate pairs found */
	off_t total_pair_cnt = 0;
	/* Calculate mapped file slice (or number of items per thread) */
	off_t pair_range = 0;
	off_t ind;
	int ret = EXIT_FAILURE;

	if (NULL == thr_ptr || NULL == thr_ptr[0].sb_ptr) {
		fprintf(stderr, "%s: NULL ptr arg found!\n", __func__);
		goto exit;
	}
	if (NULL == thr_ptr[0].sb_ptr->buffer ||
	    0 == thr_ptr[0].sb_ptr->buffer_size) {
		fprintf(stderr, "%s: unconfigured shared block found!\n",
			__func__);
		goto exit;
	}

	/* Adjust the common data params */
	total_pair_cnt = thr_ptr[0].sb_ptr->buffer_size /
	    sizeof(struct vect_coord_pair);
	pair_range = total_pair_cnt / NUM_THREADS;

	/* There are fewer file coordinate pair entries than worker threads */
	if (pair_range == 0) {
		/* Only the first thread will be assigned actual work */
		create_thr(&thr_ptr[0], 0, total_pair_cnt, 0,
			   0.0f, worker_thread, thr_ptr[0].sb_ptr);
		/* Set the limits for the other threads to be -1 */
		for (ind = 1; ind < NUM_THREADS; ind++) {
			create_thr(&thr_ptr[ind], -1, -1, ind,
				   0.0f, worker_thread, thr_ptr[0].sb_ptr);
		}
	} else {
		/* Set ranges for all worker threads */
		for (ind = 0; ind < NUM_THREADS; ind++) {
			create_thr(&thr_ptr[ind], 0 + ind * pair_range,
				   0 + ind * pair_range + pair_range,
				   ind, 0.0f, worker_thread, thr_ptr[0].sb_ptr);
		}
		/* Fix the upper bound for the last worker, if necessary */
		if (thr_ptr[NUM_THREADS - 1].upper < total_pair_cnt) {
			thr_ptr[NUM_THREADS - 1].upper = total_pair_cnt;
		}
	}
	ret = EXIT_SUCCESS;
 exit:
	return ret;
}

int init_thr_workers(struct thread_arr_cfg *thr_ptr,
		     struct shared_block *sb_ptr) {
	int ret = EXIT_FAILURE;

	if (NULL == thr_ptr || NULL == sb_ptr || NULL == sb_ptr->buffer) {
		fprintf(stderr, "%s: NULL ptr arg found!\n", __func__);
		goto exit;
	}

	/* Initialize the common pthread barrier */
	ret = pthread_barrier_init(&sb_ptr->barrier, NULL, NUM_THREADS);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "%s: error with barrier initialization!\n",
			__func__);
		goto exit;
	}

	/* Initialize the common pthread attribute */
	ret = init_thr_attribute(&sb_ptr->attr);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "%s: error with attribute initialization!\n",
			__func__);
		goto exit;
	}

	/*
	 * Attach the initialized shared object to thr_ptr[0],
	 * all other worker threads are going to copy the reference
	 * to it later on.
	 */
	thr_ptr[0].sb_ptr = sb_ptr;

	/* Set up the configuration data for all worker threads */
	ret = init_thr_config(thr_ptr);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr,
			"%s: error with thread config initialization!\n",
			__func__);
		goto exit;
	}

 exit:
	return ret;
}

int run_thr_workers(struct thread_arr_cfg *thr_ptr) {

	int ret = EXIT_FAILURE;
	int res[NUM_THREADS];
	off_t ind;

	if (NULL == thr_ptr || NULL == thr_ptr[0].sb_ptr) {
		fprintf(stderr, "%s: null ptr arg found!\n", __func__);
		goto exit;
	}

	for (ind = 0; ind < NUM_THREADS; ind++) {
		res[ind] = pthread_create(&thr_ptr[ind].thread,
					  &thr_ptr[ind].sb_ptr->attr,
					  thr_ptr[ind].work_func_t,
					  &thr_ptr[ind]);
		if (res[ind] != EXIT_SUCCESS) {
			fprintf(stderr, "%s: error creating thread: %ld!\n",
				__func__, ind);
			goto exit;
		}
	}

	/* Wait until all worker threads are completed */
	for (ind = 0; ind < NUM_THREADS; ind++) {
		pthread_join(thr_ptr[ind].thread, NULL);
	}

	ret = EXIT_SUCCESS;

 exit:
	return ret;
}

/* Return the final result of worker thread processing */
float get_thr_result(struct thread_arr_cfg *thr_ptr) {

	off_t ind;
	float dot_product = 0.0f;

	for (ind = 0; ind < NUM_THREADS; ind++) {
		if (NULL == thr_ptr || NULL == thr_ptr[ind].sb_ptr
		    || NULL == thr_ptr[ind].sb_ptr->buffer) {
			fprintf(stderr, "%s: NULL ptr arg found!\n", __func__);
			goto exit;
		}
		dot_product += thr_ptr[ind].part_dot_sum;
	}

 exit:
	return dot_product;
}

/*
 * Destructor method to help free the dynamically allocated memory references.
 * Arguments are:
 * thr_ptr - an array of intialized struct thread_arr_cfg objects
 * Return error prints are informative only
 */

void destroy_thr_workers(struct thread_arr_cfg *thr_ptr) {
	off_t ind;
	int ret;

	if (NULL == thr_ptr || NULL == thr_ptr[0].sb_ptr) {
		fprintf(stderr, "%s: NULL ptr arg found!\n", __func__);
		goto exit;
	}

	/* Destroy the pthread attribute object */
	ret = destroy_thr_attribute(&thr_ptr[0].sb_ptr->attr);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "%s: error with attr destroy!\n", __func__);
	}

	/* Destroy the pthread barrier object */
	ret = pthread_barrier_destroy(&thr_ptr[0].sb_ptr->barrier);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "%s: error with barrier destroy!\n", __func__);
	}

	/* Reset the intermediate thread data members */
	for (ind = 0; ind < NUM_THREADS; ind++) {
		thr_ptr[ind].lower = 0;
		thr_ptr[ind].upper = 0;
		thr_ptr[ind].part_dot_sum = 0.0f;
		/* Do not forget to call the file_ops destructor afterwards */
		thr_ptr[ind].sb_ptr = NULL;
	}

 exit:
	return;
}
