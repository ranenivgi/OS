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

struct ProducerParams
{
    int id;
    int numberOfProducts;
    BoundedQueue *queue;
};

struct CoEditorParams
{
    int id;
    Queue *co_editor_queue;
    BoundedQueue *screen_queue;
};

struct DispatcherParams
{
    BoundedQueue **producers_queues;
    Queue **co_editors_queues;
    int num_producers;
};

char **split(const char *s, char delim, int *num_tokens);
void BoundedQueue_init(BoundedQueue *queue, int max_size);
void BoundedQueue_enqueue(BoundedQueue *queue, char *s);
void bounded_queue_destroy(BoundedQueue *queue);

void Queue_init(Queue *queue);
void Queue_enqueue(Queue *queue, const char *s);
char *Queue_dequeue(Queue *queue);
void queue_destroy(Queue *queue);

void CountingSemaphore_init(CountingSemaphore *sem, int startValue);
void CountingSemaphore_down(CountingSemaphore *sem);
void CountingSemaphore_up(CountingSemaphore *sem);
int counting_semaphore_value(CountingSemaphore *sem);

void *producer_thread(void *arg);
void *dispatcher_thread(void *arg);
void *co_editor_thread(void *arg);