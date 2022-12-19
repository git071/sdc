#include "pthread.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"

#define MEMBAR __sync_synchronize()
#define THREAD_COUNT 10

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

volatile int interested[THREAD_COUNT];
volatile int passed[THREAD_COUNT];
volatile int k;
volatile int resource;

void critical_section(int thread)
{

	if (resource != 0) {
		printf(ANSI_COLOR_RED "Resource was acquired by [Thread: %d], but is still in-use by [Thread: %d]!\n" ANSI_COLOR_RESET, thread, resource);
        fflush(stdout);
	}

	printf(ANSI_COLOR_GREEN "[Thread: %d] using resource in CS\n" ANSI_COLOR_RESET, thread);
    fflush(stdout);
    resource = thread;
	MEMBAR;
    // usleep(300000);
    resource = 0;
}

void lock(void *t)
{
    long thread = (long)t;
    while(1)
    {
        L1:
            interested[thread] = 1;
            while (k != thread)
            {
                passed[thread] = 0;
                if (!interested[k])
                    k = thread;
            }

        L2:
            passed[thread] = 1;
            for (int i = 0; i < THREAD_COUNT; i++)
            {
                if (i != thread)
                    if (passed[i])
                        goto L1;
            }
            critical_section(thread);
            MEMBAR;
            passed[thread] = 0, interested[thread] = 0;
            printf(ANSI_COLOR_YELLOW "[Thread: %ld] setted passed & interested to 0\n" ANSI_COLOR_RESET, thread); 
            fflush(stdout);
    }
}

int main(int argc, char** argv)
{

	memset((void*)interested, 0, sizeof(interested));
	memset((void*)passed, 0, sizeof(passed));
	k = 0;
    resource = 0;

	pthread_t threads[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; ++i) {
		pthread_create(&threads[i], NULL, lock, (void*)((long)i));
	}

	for (int i = 0; i < THREAD_COUNT; ++i) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
