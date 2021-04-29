#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "test.h"

/*
 * The purpose of the program is to help illustrate
 * the placement of the ELF segments in the virtual
 * memory address space during program execution.
 * Moreover, the program illustrates how to load
 * a text file via creating a mapping of the file
 * contents in the virtual address space assigned
 * for the ELF segments.
 * The program is to be used for educational purposes
 * only and the authors are not responsible for any
 * damage or consequences as a result of using this
 * example.
 */

/*
 * Questions:
 * 1. Can you think of some better/more robust error checks?
 * 2. Is the proposed mapping mechanism sufficient?
 * 3. Is the code style ok? If not, suggest improvements.
 * 4. Is the header file structured and used appropriately?
 * 5. Try examining the segment mapping via /procfs
 * 6. Try to answer the inline questions below as well.
 */

/* _etext, _edata, and _end ELF segment markers */
extern char end, etext, edata;

/* p_arr will be part of the .bss segment */
int p_arr[NUM_BEL];
/* d_arr will be part of the .data segment */
int d_arr[NUM_DEL] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

void unmap_data(int *fd, char *c, ssize_t len);
void file_close(int *fd);

/* Very primitive error handling function. */
/*
 * If you create more complex examples,
 * plan more complicated error handling
 * logic and do not forget to take care
 * about freeing any allocated resources.
 */
void exit_err(int err, char *c,
		int *fd, ssize_t len) {

	if (err != 0) {
		/* errno is thread local */
		/* Can you explain what thread local is? */
		fprintf(stderr, "Errno value: %d\n",
					errno);
		perror("Error detected:");
		/* Attempt to perform cleanup */
		unmap_data(fd, c, len);
		file_close(fd);
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "%s: returned\n", __func__);
	return;
}

void file_open(int *fd, char *c, ssize_t *len) {

	int flags = O_RDONLY;
	int perms = S_IRUSR | S_IRGRP | S_IROTH;
	struct stat status;

	*fd = open(FNAME, flags, perms);

	if (-1 == *fd) {
		exit_err(errno, c, fd, *len);
	}
	if (fstat(*fd, &status) < 0) {
		exit_err(errno, c, fd, *len);
	}

	*len = status.st_size;
	return;
}

void file_close(int *fd) {

	if (NULL != fd && *fd != -1) {
		if (close(*fd))
			perror("Error during file close");
	}
	return;
}

void map_data(int *fd, char **c, ssize_t len) {

	if (NULL == fd || NULL == c
			|| len <= 0) {
		exit_err(-1, *c, fd, len);
	}
	if ((*c = mmap(NULL, len, PROT_READ, MAP_PRIVATE,
				*fd, 0)) == MAP_FAILED) {
		exit_err(errno, *c, fd, len);
	}
	return;
}

void unmap_data(int *fd, char *c, ssize_t len) {

	if (NULL == c)
		return;

	/*
	 * Not an optimal error handling path,
	 * but could work for the example.
	 */
	if (munmap(c, len) == -1)
		perror("Error during unmap");

	return;
}

void print_map_data(int *fd, char *c, ssize_t len) {

	ssize_t i;

	if (NULL == c || len <= 0) {
		exit_err(-1, c, fd, len);
	}
	fprintf(stdout, "Mapped file contents is: \n\n");

	for (i = 0; i < len; i++)
		fprintf(stdout, "%c", c[i]);

	fprintf(stdout, "\n");
	fprintf(stdout, "Mapped file reading finished!\n\n");
	return;	
}

void print_limit(int resource, const char *rname) {

	struct rlimit rlim;

	fprintf(stdout, "%s stats:\n", rname);
        if (getrlimit(resource, &rlim)) {
		perror("Error reading limits");
		return;
	}
        fprintf(stdout, "\t Program %s size:\n", rname);
	fprintf(stdout, "\t\t\t\t cur: %ld max: %ld in bytes\n",
                rlim.rlim_cur, rlim.rlim_max);
	return;
}

void print_pagesize() {


	fprintf(stdout, "Program VM segment page size:\n");
	fprintf(stdout, "\t\t\t\t cur: %d in bytes\n", getpagesize());
	return;
}

void print_cmdl(int cnt, char *argv[]) {
	int c;

	fprintf(stdout, "Program cmdline:\n");
	for (c = 0; c < cnt; c++) {
		fprintf(stdout,
			"\t Detected arg:\t\t\t: %s\n",
			argv[c]);
	}
	return;
}

void dyn_mem_alloc(int **p, ssize_t len) {

	/* Is this sufficient? */
	*p = (int *)malloc(len * sizeof(int));

	if (NULL == *p) {
		perror("error allocating dynamic memory!");
	}
	return;
}

void dyn_mem_free(int **p) {
	free(*p);
	*p = NULL;
}

void wait_for_input() {

	fprintf(stdout, "Press any key!\n");
	getc(stdin);
}

int main(int argc, char *argv[]) {

	/* Local or automatic variables */
	/* Where in memory are these going to be located? */
	int ret = EXIT_SUCCESS;
	int fd;
	ssize_t len;
	char *c = NULL;
	int *int_p = NULL;

	/* Attempt to obtain some stats and print them */
	fprintf(stdout, "Hello from: %s: pid: %d\n",
				argv[0], getpid());
	fprintf(stdout, "First address past:\n");
	fprintf(stdout, "\t program text (etext):\t\t %16p\n",
				&etext);
	fprintf(stdout, "\t .data end (edata):\t\t %16p\n",
				&edata);
	fprintf(stdout, "\t .bss data (end):\t\t %16p\n",
				&end);

	print_cmdl(argc, argv);
	print_limit(RLIMIT_STACK, "Stack");
	print_limit(RLIMIT_AS, "Virtual Address");
	print_limit(RLIMIT_DATA, "Data Segment");
	print_pagesize();

	/*
	 * The approach presumes linear execution with an
	 * implicit built-in error handling at the expense
	 * of some extra stack usage probably.
	 * Is this assumption correct?
	 */
	file_open(&fd, c, &len);
	map_data(&fd, &c, len);

	/* Print the mapped file contents */
	print_map_data(&fd, c, len);

	/* Allocate a block of dynamic memory here */
	dyn_mem_alloc(&int_p, NUM_ELEMENTS);
	wait_for_input();
	dyn_mem_free(&int_p);

	unmap_data(&fd, c, len);
	file_close(&fd);

	return ret;
}
