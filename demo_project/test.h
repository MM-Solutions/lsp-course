#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <time.h>
#include <fcntl.h>

/* Maximum number of active worker threads */
#define NUM_THREADS		4

/* Name of the input text file to be mapped into memory */
#define FNAME			"coords.txt"

/*
 * Primitive file parser specific parameters:
 *
 * NUM_SYM_FORMAT - according to the convention used - each vector component
 * is set via 5 ASCII symbols following the notation [0-9][0-9].[0-9][0-9]
 * with two digits for the whole part and two - for the fractional part.
 * NUM_SYM_SEP - number of blank spaces separating each text coordinate.
 * NUM_SYM_TERM - number of terminating symbols for each text file line.
 * Each FNAME file line is expected to follow the format:
 * [float number][space * NUM_SYM_SPACE][float number][new line * NUM_SYN_TERM]
 */

/* Expected number of bytes for each vector component */
#define NUM_SYM_FORMAT		5
/* Expected number of bytes for each data separator */
#define NUM_SYM_SEP		1
/* Expected number of bytes for each text file line terminator */
#define NUM_SYM_TERM		1

/* Sleep interval for in microseconds */
#define SLEEP_INT		100000

/*
 * Data structure, which represents a pair of input vector components
 * in the listed text notation above.
 * Adapt and revise, if you modify the structure of the input text file.
 * The pragma attribute helps ensure fixed and compact data alignment.
 */
#pragma pack(push, 1)
struct vect_coord_pair {
	char vec_a[NUM_SYM_FORMAT];
	char bsp[NUM_SYM_SEP];
	char vec_b[NUM_SYM_FORMAT];
	char term[NUM_SYM_TERM];
};
#pragma pack(pop)

/*
 * Shared memory object data type declaration.
 * Represents common data configuration shared among all worker threads.
 */
struct shared_block {
	/* Reference to the text file opened and mapped */
	int fd;
	/* Pointer to the mmapped text file contents */
	char *buffer;
	/* Read text file size in Bytes */
	off_t buffer_size;
	/* Common worker thread behavior attribute object */
	pthread_attr_t attr;
	/* Common barrier object for worker thread synchronization */
	pthread_barrier_t barrier;
};

/*
 * Represents attributes for each one of the worker threads.
 * The program operates with an array of such objects.
 * After all worker threads are finished, the partially
 * calculated dot product sums should be added together
 * to obtain the final result.
 */
struct thread_arr_cfg {
	/* Associated worker thread pthread object for each instance */
	pthread_t thread;
	/* Associated worker thread function pointer */
	void *(*work_func_t) (void *ptr);
	/* Index of the associated thread instance */
	off_t thr_index;
	/*  Index of the starting offset within the mapped array - inclusive */
	off_t lower;
	/*  Index of the ending offset within the mapped array - exclusive */
	off_t upper;
	/* Partially calculated dot product result */
	float part_dot_sum;
	/* Reference to the common worker thread configuration */
	struct shared_block *sb_ptr;
};
#endif
