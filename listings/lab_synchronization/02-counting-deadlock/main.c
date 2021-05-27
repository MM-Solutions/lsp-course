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

const int num_iterations = 1000 * 1000;

static int counter;

static pthread_mutex_t counter_mutex;
static pthread_mutex_t print_mutex;

/* Counter thread. */
static void *mythread_counter(void *arg)
{
	int *c = arg;
	int i;

	for (i = 0;  i < num_iterations; i++) {
		pthread_mutex_lock(&counter_mutex);
		(*c)++;
		if ((*c % 1000) == 0) {
			pthread_mutex_lock(&print_mutex);
			printf("Ding - a counter milestone.\n");
			pthread_mutex_unlock(&print_mutex);
		}
		pthread_mutex_unlock(&counter_mutex);
	}
	return NULL;
}

/* Aggressive counter reader. */
static void *mythread_printer(void *arg)
{
	int *c = arg;
	int i;

	for (i = 0;  i < num_iterations; i++) {
		pthread_mutex_lock(&print_mutex);

		printf("==========: ");
		pthread_mutex_lock(&counter_mutex);
		printf("%d\n", *c);
		pthread_mutex_unlock(&counter_mutex);

		pthread_mutex_unlock(&print_mutex);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t threads[2];

	if (argc != 1)
		return EXIT_FAILURE;

	pthread_mutex_init(&counter_mutex, NULL);
	pthread_mutex_init(&print_mutex, NULL);

	pthread_create(&threads[0], NULL, mythread_counter, &counter);
	pthread_create(&threads[1], NULL, mythread_printer, &counter);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	printf("counter: Expected: %d, actual: %d\n", num_iterations, counter);

	return EXIT_SUCCESS;
}
