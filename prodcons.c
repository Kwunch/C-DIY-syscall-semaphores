#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <signal.h>

struct cs1550_sem;

struct cs1550_sem* bins;
struct cs1550_sem* empty;
struct cs1550_sem* full;

int buf_size;

// Prod Cons Vars
int* in;
int* out;
int* total;
int* buffer;

//Alphabet For ID
char alpha[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'};

// Initialize semaphore var
int init_sem(struct cs1550_sem *sem, int value) {
	return syscall(441, sem, value);
}

// Wait
void down(struct cs1550_sem *sem) {
	syscall(442, sem);
}

// Signal
void up(struct cs1550_sem *sem) {
	syscall(443, sem);
}

// Producer function. Adds to buffer
void producer(char id) {
	while(1) {
		down(empty); // Decrement empty
		down(bins); // Decrement Mutex (locks)

		*(buffer + *in) = (*total)++;
		printf("Producer %c Produced: %d\n", id, *(buffer + *in));
		*in = ((*in) + 1) % (buf_size);

		up(bins); // Increment Mutex (unlocks)
		up(full); // Increment full (unlocks consumer)
	}
}

// Consumer function. Reads from buffer
void consumer(char id) {
	while(1) {
		down(full); // Decrement full (may lock consumer)
		down(bins); // Decrement Mutex (locks)

		printf("Consumer %c Consumed: %d\n", id, *(buffer + *out));
		*out = ((*out) + 1) % (buf_size);

		up(bins); // Increment Mutex (Unlocks)
		up(empty); // Increment empty
	}
}

int main(int argc, char *argv[]) {
	// CMD Args
	// prodcons[0]
	// num_prods[1]
	// num_cons[2]
	// buffer[3]

	// Pid var for forking
	pid_t pid;

	// Get cmd args
	int num_prods = atoi(argv[1]);
	int num_cons = atoi(argv[2]);
	buf_size = atoi(argv[3]);

	// Each sem gets 12 bytes (3ints) (9 ints total)
	// Total gets 4 bytes (1 int)
	// In gets 4 bytes (1 int)
	// Out gets 4 bytes (1 int)
	// Need size of 12 ints total
	// Plus bufsize
	int n = (sizeof(int)*12) + buf_size;

	// Shared memory space (n = 48 + bufsize) * 4
	void *ptr = mmap(NULL, n * sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED) {
		printf("Map Failed\n");
		return -1;
	}

	// Bins gets index 0
	bins = ptr;
	// Move empty to give bins 12 bytes
	empty = (struct cs1550_sem*)((int *)bins + 3);
	// Move full to give empty 12 bytes
	full = (struct cs1550_sem*)((int *)empty + 3);

	// Prod Cons Vars
	// Move total to give 12 bytes
	total = (int *)((int *)full + 3);
	// Total gets 1 byte point in at next int
	in = (total + 1);
	// In gets 1 byte, point out at next int
	out = (in + 1);

	// Points buffer at next int and give it the rest of the bytes
	// Equal to buf_size * sizeof(int)
	buffer = (out + 1); // Set buffer pointer

	// Set Prod Cons Vars to 0
	*total = 0;
	*in = 0;
	*out = 0;


	// Mutex semaphore gets value of 1
	if (init_sem(bins, 1) == 0) {
		printf("Mutex Semaphore Initialized.\n");
	} else {
		printf("Failed to Initialize\n");
		return -1;
	}

	//Empty semaphore gets value of buf_size
	if (init_sem(empty, buf_size) == 0) {
		printf("Empty Semaphore Initialized.\n");
	} else {
		printf("Failed to initialize empty\n");
		return -1;
	}

	//Full semaphore gets value of 0
	if (init_sem(full, 0) == 0) {
		printf("Full Semaphore Initialized.\n");
	} else {
		printf("Failed to initialize full\n");
		return -1;
	}


	// Create index
	int i = 0;
	// Iterate through num_prods forking and creating a producer
	while (i < num_prods) {
		pid = fork();
		if (pid < 0) {
			printf("Error Forking Num Prods\n");
			return -1;
		} else if (pid == 0) {
			// Create producer with char id at index i
			producer(alpha[i]);
			// Child breaks after loop to finish
			break;
		} else {
			// Parent increments i and continues loop
			i += 1;
			continue;
		}
	}


	i = 0; // Reset index
	// Iterate through num_cons forking and creating a consumer
	while (i < num_cons) {
		pid = fork();
		if (pid < 0) {
			printf("Error Forking Num Cons\n");
			return -1;
		} else if (pid == 0) {
			// Create consumer with char id at index i
			consumer(alpha[i]);
			// Child breaks after loop to finish
			break;
		} else {
			// Parent increments i and continues loop
			i += 1;
			continue;
		}
	}


	if (pid != 0) {
		// Parent process waits for child
		wait(NULL);
	}
}
