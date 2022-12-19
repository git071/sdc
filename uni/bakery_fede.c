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

volatile int tickets[THREAD_COUNT];
volatile int choosing[THREAD_COUNT];
volatile int resource;

void critical_section(int thread, int ticket)
{

	if (resource != 0) {
		printf(ANSI_COLOR_RED "Resource was acquired by [Thread: %d], but is still in-use by [Thread: %d]!\n" ANSI_COLOR_RESET, thread, resource);
        fflush(stdout);
    }

	resource = thread;
	printf(ANSI_COLOR_GREEN "[Thread: %d] using resource in CS [Ticket: %d]\n" ANSI_COLOR_RESET, thread, ticket);
    fflush(stdout);

	MEMBAR;
    // usleep(300000);
	resource = 0;
}

int maximum()
{
    int max = 0;
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        int temp = tickets[i];
        if (max < temp)
            max = temp;
    }
    return max;
}

void lock(void *t)
{

    while(1)
    {
        long thread = (long)t;
        choosing[thread] = 1;

        MEMBAR;

        int max_ticket = maximum() + 1;

        tickets[thread] = max_ticket;

        MEMBAR;
        choosing[thread] = 0;
        MEMBAR;

        for (int other = 0; other < THREAD_COUNT; ++other) {
            
            int i = 0;
            while (choosing[other]) {
                printf(ANSI_COLOR_RED "[Thread: %ld] are waiting other choose\n" ANSI_COLOR_RESET, thread);
                fflush(stdout);
            }

            MEMBAR;

            while (tickets[other] != 0 && (tickets[other] < tickets[thread] || (tickets[other] == tickets[thread] && other < thread))) {
            }

        }
        critical_section(thread, max_ticket);

        MEMBAR;
        tickets[thread] = 0;
        printf(ANSI_COLOR_YELLOW "[Thread: %ld] setted his ticket to 0\n" ANSI_COLOR_RESET, thread);
        fflush(stdout);
    }
}

int main(int argc, char** argv)
{

	memset((void*)tickets, 0, sizeof(tickets));
	memset((void*)choosing, 0, sizeof(choosing));
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
