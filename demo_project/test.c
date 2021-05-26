#include "test.h"
#include "file_ops.h"
#include "thread_ops.h"

/*
 * The purpose of the program is to help illustrate
 * a simple multithreaded dot product calculation
 * of two input n-dimensional geometric vectors.
 * Data for both of the vectors is read from a text file
 * with each vector presented as a column of floating
 * point numeric components.
 * Text file reading is accomplished via mmap and
 * dot product calculation is organized with a set of
 * worker threads. Thread synchronization is made with
 * a pthread barrier and join approach.
 * Source code is separated into a few logical modules
 * and some primitive form of C-style data abstraction
 * is introduced.
 * The example shows how the solution of a practical
 * problem could be implemented as a simple C project.
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
 * Test prerequisites:
 * 1) generate an input test file via the provided file_generate.sh
 * 2) compile the project via running make
 * 3) run the program
 */

/*
 * Questions:
 * 1. Is the code style and structure appropriate?
 * 2. How could you improve the input file validation?
 * 3. Is the error checking logic robust and correct?
 * 4. Can you find and point out any bugs in the proposed
 * input text file parsing?
 * 5. How can you improve the design and implementation of
 * this simple problem solution?
 * 5.1. Is the float format and precision sufficient?
 * 5.2. Can you write a simple unit test script?
 * 5.3. Can you decouple the common thread creation attributes
 * from the shared_block data declaration and make the
 * design more flexible?
 * 5.4. Can you provide an API function for setting the worker
 * thread processing function externally?
 * Any suggestions and comments are welcome and expected.
 * 6. Try to answer the inline questions as usual.
 */

int main(int argc, char *argv[]) {

	/* Local or automatic variables */
	struct shared_block sb;
	struct thread_arr_cfg threads[NUM_THREADS];
	float dot_product_thr = 0.0f;
	int ret = EXIT_SUCCESS;

	/* Actual work begins */
	ret = init_map_data(&sb, FNAME);
	if (ret != EXIT_SUCCESS)
		goto exit_map;

	ret = init_thr_workers(threads, &sb);
	if (ret != EXIT_SUCCESS)
		goto exit;

	ret = run_thr_workers(threads);
	if (ret != EXIT_SUCCESS)
		goto exit;

	dot_product_thr = get_thr_result(threads);
	fprintf(stdout, "%s: final multithreaded dot product is: %f\n",
		__func__, dot_product_thr);
	/* print_map_data(&sb); */

 exit:
	fprintf(stdout, "%s: the program has been completed\n", __func__);
	destroy_thr_workers(threads);
 exit_map:
	destroy_map_data(&sb);
	return ret;
}
