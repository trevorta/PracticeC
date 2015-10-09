#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_t threadArray[2];

void* executeThread(void *arg) {
	long tid = (long) arg;
	printf("Thread %ld is running\n", tid);
	pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
	int err;

	for (int i = 0; i < 20; i++) {
		err = pthread_create(newThread, NULL, &executeThread, (void*) i);
		if (err != 0) {
			printf("Can't create thread: [%s]\n", strerror(err));
		}
	}

	for (int i = 0; i < 20; i++) {
		pthread_join()
	}



	printf("main thread terminates"); // main thread terminates before thread 9 executes
}
