#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_t threadArray[2];

void* executeThread(void *arg) {
	pthread_t currentThreadId = pthread_self(); // get current thread's id
	printf(currentThreadId)

	if (pthread_equal(currentThreadId, threadArray[0])) {
		printf("First thread is running\n");
	} else {
		printf("Second thread is running\n");
	}

	unsigned long i = 0;
	//for(i=0; i<(0xFFFFFFFF);i++);

	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char * argv[]) {
	printf("Hello world\n");
	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("%d: %s\n", i, argv[i]);
	}

	int i = 0;
	int err;

	while (i < 2) {
		err = pthread_create(&threadArray[i], NULL, &executeThread, NULL);
		if (err != 0) {
			printf("Can't create thread: [%s]\n", strerror(err));
		} else {
			printf("Thread %d has been created successfully\n", i);
		}
		i++;
	}
	pthread_join(threadArray[0], NULL);
	pthread_join(threadArray[1], NULL);	

	//sleep(5);

	// ============= TESTING ERRORS HERE ======================
	// warning: format string is not a string literal
	//printf(strcat(argv[1], "\n")); // Segmentation fault when explicitly access an unassigned part of memory
}
// ==========================================================================================================
