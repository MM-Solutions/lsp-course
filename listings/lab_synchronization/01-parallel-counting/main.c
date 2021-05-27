#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

const int num_threads = 100;
const int num_iterations = 1000 * 1000;

static int counter;

static void *mythread(void *arg)
{
	int *c = arg;
	int i;

	for (i = 0;  i < num_iterations; i++) {
		(*c)++;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t threads[num_threads];
	int i;

	if (argc != 1)
		return EXIT_FAILURE;

	for (i = 0; i < num_threads; i++)
		pthread_create(&threads[i], NULL, mythread, &counter);

	for (i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	printf("counter: Expected: %d, actual: %d\n", num_threads * num_iterations, counter);

	return EXIT_SUCCESS;
}
