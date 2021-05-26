#ifndef FILE_OPS_H
#define FILE_OPS_H

/*
 * The file lists the API functions for opening and mapping
 * the input text file from within the main C application.
 * The recommended sequence of calls should be:
 * 1) init_map_data - wraps fopen and mmap
 * 2) destroy_map_data - frees the initialized file and shared block
 * references
 */

/*
 * Used for the input text file closing and memory unmapping
 * Arguments expected:
 * sb_ptr - pointer to a shared memory configuration object
 */
void destroy_map_data(struct shared_block *sb_ptr);

/*
 * Used for the input text file opening and memory mapping
 * Arguments expected:
 * sb_ptr - pointer to a shared memory configuration object
 * fname - string wiht the name of the file to be accessed
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
int init_map_data(struct shared_block *sb_ptr, const char *fname);

/*
 * Used for debugging purposes only
 * Arguments expected:
 * sb_ptr - pointer to a shared memory configuration object
 * Returns either EXIT_SUCCESS or EXIT_FAILURE
 */
int print_map_data(struct shared_block *sb_ptr);
#endif
