#include "test.h"
#include "file_ops.h"

static int file_close(struct shared_block *sb_ptr);

/*
 * Static helper for simplified internal cleanup.
 * Added for safety and could be redesigned, if necessary.
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
static int exit_err(struct shared_block *sb_ptr, int err) {

	int ret = EXIT_SUCCESS;

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "Errno value: %d\n", errno);
		/* Attempt to perform cleanup */
		destroy_map_data(sb_ptr);
		sb_ptr->fd = -1;
		sb_ptr->buffer = NULL;
		ret = EXIT_FAILURE;
		goto exit;
	}
	fprintf(stdout, "%s: returned no error\n", __func__);
 exit:
	return ret;
}

/*
 * Static helper for opening the input text file
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
static int file_open(struct shared_block *sb_ptr, const char *fname) {

	int flags = O_RDONLY;
	int perms = S_IRUSR | S_IRGRP | S_IROTH;
	struct stat status;
	int ret = EXIT_SUCCESS;

	sb_ptr->fd = open(FNAME, flags, perms);

	if (-1 == sb_ptr->fd) {
		ret = EXIT_FAILURE;
		goto exit;
	}
	if (fstat(sb_ptr->fd, &status) < 0) {
		ret = exit_err(sb_ptr, -1);
		goto exit;
	}

	sb_ptr->buffer_size = status.st_size;
 exit:
	return ret;
}

/*
 * Static helper for closing the input text file
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
static int file_close(struct shared_block *sb_ptr) {
	int ret = EXIT_FAILURE;

	if (NULL == sb_ptr) {
		fprintf(stderr, "%s: NULL sb_ptr pointer\n", __func__);
		goto exit;
	}

	if (sb_ptr->fd && sb_ptr->fd != -1) {
		if (close(sb_ptr->fd)) {
			fprintf(stderr, "%s: error during file closing\n",
				__func__);
			goto exit;
		}
	}
	ret = EXIT_SUCCESS;
 exit:
	return ret;
}

int init_map_data(struct shared_block *sb_ptr, const char *fname) {

	char **buffer = NULL;
	int ret = EXIT_SUCCESS;

	if (NULL == sb_ptr || NULL == fname) {
		fprintf(stderr, "%s: NULL pointer argument provided\n",
			__func__);
		ret = EXIT_FAILURE;
		goto exit;
	}

	ret = file_open(sb_ptr, fname);
	if (ret != EXIT_SUCCESS) {
		goto exit;
	}

	buffer = &sb_ptr->buffer;

	if ((*buffer = mmap(NULL, sb_ptr->buffer_size, PROT_READ,
			    MAP_PRIVATE, sb_ptr->fd, 0)) == MAP_FAILED) {
		ret = exit_err(sb_ptr, -1);
		goto exit;
	}

 exit:
	return ret;
}

void destroy_map_data(struct shared_block *sb_ptr) {

	if (NULL == sb_ptr || NULL == sb_ptr->buffer) {
		fprintf(stderr, "%s: NULL pointer argument provided\n",
			__func__);
		goto exit;
	}

	if (munmap(sb_ptr->buffer, sb_ptr->buffer_size) == -1) {
		fprintf(stderr, "%s: error during unmap\n", __func__);
	}

	if (file_close(sb_ptr) != EXIT_SUCCESS) {
		fprintf(stderr, "%s: error during file closing\n", __func__);
	}

 exit:
	return;
}

/* Used for debug purposes and primitive parser validation */
int print_map_data(struct shared_block *sb_ptr) {

	off_t ind;
	struct vect_coord_pair *buf;
	float op_a, op_b;
	int ret = EXIT_SUCCESS;

	if (NULL == sb_ptr->buffer || sb_ptr->buffer_size <= 0) {
		goto exit;
	}
	fprintf(stdout, "Mapped file length is %zu:\n", sb_ptr->buffer_size);
	fprintf(stdout, "Size of struct vect_coord_pair is %zu:\n",
		sizeof(struct vect_coord_pair));
	fprintf(stdout, "Mapped file number of lines is %zu:\n",
		sb_ptr->buffer_size / sizeof(struct vect_coord_pair));

	buf = (struct vect_coord_pair *)sb_ptr->buffer;
	for (ind = 0; ind < sb_ptr->buffer_size /
	     sizeof(struct vect_coord_pair); ind++) {
		fprintf(stdout, "coord one: %.*s, coord two: %.*s\n",
			(int)sizeof(buf[ind].vec_a), buf[ind].vec_a,
			(int)sizeof(buf[ind].vec_a), buf[ind].vec_b);
		op_a = strtof(buf[ind].vec_a, NULL);
		op_b = strtof(buf[ind].vec_b, NULL);
		fprintf(stdout, "float operand one: %5.2f,\
			float operand two: %5.2f\n", op_a, op_b);
	}

	fprintf(stdout, "\n");
	fprintf(stdout, "Mapped file reading finished!\n");
 exit:
	return ret;
}
