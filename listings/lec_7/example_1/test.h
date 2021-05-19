#define NUM_THREADS		2
#define MAX_SLEEP_TIME_SECS	10

static const int CONSUMER_ONE_VALUE = 9;
static const int CONSUMER_TWO_VALUE = -6;

/* Shared memory object data type declaration */
struct shared_block {
	int buffer;
};

/* In order to simplify thread creation,
 * a function pointer could be defined along
 * with a static array in test.c.
 */

typedef void *(*work_func_t)(void *ptr);
