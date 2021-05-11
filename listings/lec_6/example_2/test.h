#define NUM_THREADS		2
#define MAX_SLEEP_TIME_SECS	10

/* Shared memory object data type declaration */
struct shared_block {
	bool ready[NUM_THREADS];
	int turn_idx;
	int buffer;
};
