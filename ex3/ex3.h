#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_STRING_LENGTH 100

typedef struct
{
    int value;
    pthread_mutex_t mutex;
    pthread_mutex_t cond;
} CountingSemaphore;

typedef struct
{
    char **buffer;
    int capacity;
    int size;
    int headIndex;
    int tailIndex;
    CountingSemaphore empty_sem;
    CountingSemaphore full_sem;
    pthread_mutex_t mutex;
} BoundedQueue;

typedef struct
{
    char **buffer;
    int capacity;
    int size;
    int headIndex;
    int tailIndex;
    CountingSemaphore full_sem;
    pthread_mutex_t mutex;
} Queue;

typedef struct 
{
    int id;
    int numberOfProducts;
    BoundedQueue *queue;
} ProducerParams;

typedef struct
{
    int id;
    Queue *coEditorQueue;
    BoundedQueue *screenQueue;
} CoEditorParams;

typedef struct 
{
    BoundedQueue **producersQueues;
    Queue **coEditorsQueues;
    int producersNumber;
} DispatcherParams;

char **splitConfigFile(const char *s, char delim, int *num_tokens);
void boundedQueueInit(BoundedQueue *queue, int max_size);
void boundedQueuePush(BoundedQueue *queue, char *s);
void boundedQueueDestroy(BoundedQueue *queue);

void unBoundedQueueInit(Queue *queue);
void unBoundedQueuePush(Queue *queue, const char *s);
char *unBondedQueuePop(Queue *queue);
void unBondedQueueDestroy(Queue *queue);

void semaphoreInit(CountingSemaphore *sem, int startValue);
void semaphoreDecrease(CountingSemaphore *sem);
void semaphoreIncrease(CountingSemaphore *sem);

void *handleProducer(void *arg);
void *handleDispatcher(void *arg);
void *handleCoEditor(void *arg);