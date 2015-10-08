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
	long i = 0;
	int err;

	while (i < 20) {
		pthread_t* newThread = malloc(sizeof(pthread_t));
		err = pthread_create(newThread, NULL, &executeThread, (void*) i);
		if (err != 0) {
			printf("Can't create thread: [%s]\n", strerror(err));
		}
		i++;
	}

	printf("main thread terminates"); // main thread terminates before thread 9 executes
}
