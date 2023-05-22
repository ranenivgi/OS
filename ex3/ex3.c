#include "ex3.h"

// split the config file
char **split(const char *s, char delim, int *num_tokens)
{
    int count = 0;
    const char *p = s;
    int in_token = 0;
    while (*p != '\0')
    {
        if (*p == delim)
        {
            in_token = 0;
        }
        else if (in_token == 0)
        {
            in_token = 1;
            count++;
        }
        p++;
    }

    char **tokens = (char **)malloc(count * sizeof(char *));
    int i = 0;
    char *token = strtok((char *)s, &delim);
    while (token != NULL)
    {
        tokens[i] = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
        strcpy(tokens[i], token);
        i++;
        token = strtok(NULL, &delim);
    }

    *num_tokens = count;
    return tokens;
}

// Bounded queue functions
void BoundedQueue_init(BoundedQueue *queue, int max_size)
{
    queue->buffer = malloc(max_size * sizeof(char *));
    queue->capacity = max_size;
    queue->size = 0;
    queue->headIndex = 0;
    queue->tailIndex = 0;
    CountingSemaphore_init(&queue->empty_sem, max_size);
    CountingSemaphore_init(&queue->full_sem, 0);
    pthread_mutex_init(&queue->mutex, NULL);
}

void BoundedQueue_enqueue(BoundedQueue *queue, char *s)
{
    // Increase one to the empty semaphore
    CountingSemaphore_down(&queue->empty_sem);
    // Lock the thread to prevent race condition
    pthread_mutex_lock(&queue->mutex);
    // Add the new string to the end of the buffer
    queue->buffer[queue->tailIndex] = strdup(s);
    // We are in a circled queue so we do module
    queue->tailIndex = (queue->tailIndex + 1) % queue->capacity;
    // Add one to the queue size
    queue->size++;
    // Release the lock
    pthread_mutex_unlock(&queue->mutex);
    // Decrease one from the full semaphore
    CountingSemaphore_up(&queue->full_sem);
}

char *BoundedQueue_dequeue(BoundedQueue *queue)
{
    CountingSemaphore_down(&queue->full_sem);
    pthread_mutex_lock(&queue->mutex);
    char *result = queue->buffer[queue->headIndex];
    queue->headIndex = (queue->headIndex + 1) % queue->capacity;
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    CountingSemaphore_up(&queue->empty_sem);
    return result;
}

void bounded_queue_destroy(BoundedQueue *queue)
{
    for (int i = 0; i < queue->size; i++)
    {
        free(queue->buffer[i]);
    }
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    pthread_mutex_destroy(&queue->full_sem.mutex);
    pthread_cond_destroy(&queue->full_sem.cond);
    pthread_mutex_destroy(&queue->empty_sem.mutex);
    pthread_cond_destroy(&queue->empty_sem.cond);
}

// Unbounded queue functions
void Queue_init(Queue *queue)
{
    queue->buffer = malloc(sizeof(char *));
    queue->capacity = 1; // Start with a capacity of 1
    queue->size = 0;
    queue->headIndex = 0;
    queue->tailIndex = 0;
    CountingSemaphore_init(&queue->full_sem, 0);
    pthread_mutex_init(&queue->mutex, NULL);
}

void Queue_enqueue(Queue *queue, const char *s)
{
    pthread_mutex_lock(&queue->mutex);
    if (queue->size >= queue->capacity)
    {
        // Increase the capacity dynamically
        int new_capacity = queue->capacity * 2;
        char **new_buffer = realloc(queue->buffer, new_capacity * sizeof(char *));
        if (new_buffer == NULL)
        {
            // Handle allocation failure
        }
        else
        {
            // Free the previous buffer contents
            for (int i = 0; i < queue->size; i++)
            {
                free(queue->buffer[i]);
            }
            // Free the previous buffer
            free(queue->buffer);
            queue->buffer = new_buffer;
            queue->capacity = new_capacity;
        }
    }
    queue->buffer[queue->tailIndex] = strdup(s);
    queue->tailIndex = (queue->tailIndex + 1) % queue->capacity;
    queue->size = queue->size + 1;
    pthread_mutex_unlock(&queue->mutex);
    CountingSemaphore_up(&queue->full_sem);
}

char *Queue_dequeue(Queue *queue)
{
    CountingSemaphore_down(&queue->full_sem);
    pthread_mutex_lock(&queue->mutex);
    char *result = queue->buffer[queue->headIndex];
    queue->headIndex = (queue->headIndex + 1) % queue->capacity;
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

void queue_destroy(Queue *queue)
{
    for (int i = 0; i < queue->capacity; i++)
    {
        if (queue->buffer[i] != NULL)
        {
            free(queue->buffer[i]);
        }
    }
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    pthread_mutex_destroy(&queue->full_sem.mutex);
    pthread_cond_destroy(&queue->full_sem.cond);
}

// Counting Semaphore functions
void CountingSemaphore_init(CountingSemaphore *sem, int startValue)
{
    sem->value = startValue;
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
}

void CountingSemaphore_down(CountingSemaphore *sem)
{
    pthread_mutex_lock(&sem->mutex);
    while (sem->value <= 0)
    {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    sem->value--;
    pthread_mutex_unlock(&sem->mutex);
}

void CountingSemaphore_up(CountingSemaphore *sem)
{
    pthread_mutex_lock(&sem->mutex);
    sem->value++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}

int counting_semaphore_value(CountingSemaphore *sem)
{
    return sem->value;
}

void *producer_thread(void *arg)
{
    struct ProducerParams *params = (struct ProducerParams *)arg;
    int id = params->id;
    int numberOfProducts = params->numberOfProducts;
    BoundedQueue *queue = params->queue;

    int sports = 0, news = 0, weather = 0;

    for (int i = 0; i < numberOfProducts; ++i)
    {
        int type = rand() % 3;
        char product[MAX_STRING_LENGTH];
        // snprintf(product, MAX_STRING_LENGTH, "%s %d", types[type]);
        switch (type)
        {
        case 0:
            snprintf(product, MAX_STRING_LENGTH, "SPORTS %d", sports++);
            break;
        case 1:
            snprintf(product, MAX_STRING_LENGTH, "NEWS %d", news++);
            break;
        case 2:
            snprintf(product, MAX_STRING_LENGTH, "WEATHER %d", weather++);
            break;
        }
        char s[MAX_STRING_LENGTH];
        snprintf(s, MAX_STRING_LENGTH, "Producer %d %s", id, product);
        printf("%s!@#!@#!@#!@#!@#\n", s);
        BoundedQueue_enqueue(queue, s);
        printf("%s\n", s);
    }

    BoundedQueue_enqueue(queue, "DONE");

    return NULL;
}

void *dispatcher_thread(void *arg)
{
    struct DispatcherParams *params = (struct DispatcherParams *)arg;
    BoundedQueue **producers_queues = params->producers_queues;
    Queue **co_editors_queues = params->co_editors_queues;
    int num_producers = params->num_producers;

    int current_queue = 0;
    int ended = 0;

    while (ended != num_producers)
    {
        char *s = BoundedQueue_dequeue(producers_queues[current_queue]);
        if (strcmp(s, "") == 0)
        {
            continue;
        }
        if (strcmp(s, "DONE") == 0)
        {
            ended++;
            continue;
        }
        if (strstr(s, "SPORTS") != NULL)
        {
            Queue_enqueue(co_editors_queues[0], s);
        }
        if (strstr(s, "NEWS") != NULL)
        {
            Queue_enqueue(co_editors_queues[0], s);
        }
        if (strstr(s, "WEATHER") != NULL)
        {
            Queue_enqueue(co_editors_queues[0], s);
        }
        current_queue = (current_queue + 1) % num_producers;
    }

    for (int i = 0; i < 3; ++i)
    {
        Queue_enqueue(co_editors_queues[i], "DONE");
    }

    return NULL;
}

void *co_editor_thread(void *arg)
{
    struct CoEditorParams *params = (struct CoEditorParams *)arg;
    BoundedQueue *screen_queue = params->screen_queue;

    while (1)
    {
        char *s = Queue_dequeue(params->co_editor_queue);
        if (strcmp(s, "") == 0)
        {
            continue;
        }
        if (strcmp(s, "DONE") == 0)
        {
            break;
        }
        //usleep(500000);
        //BoundedQueue_enqueue(screen_queue, s);
    }

    //BoundedQueue_enqueue(screen_queue, "DONE");

    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: ./program config_file\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", argv[1]);
        return 1;
    }

    char config[MAX_STRING_LENGTH + 1];
    int size = fread(config, sizeof(char), MAX_STRING_LENGTH, file);
    config[size] = '\0';

    fclose(file);

    int num_tokens;
    char **tokens = split(config, '\n', &num_tokens);

    int num_producers = num_tokens / 3;
    int screenCapacity = atoi(tokens[num_tokens - 1]);

    BoundedQueue **producers_queues = (BoundedQueue **)malloc(num_producers * sizeof(BoundedQueue *));
    Queue **co_editors_queues = (Queue **)malloc(3 * sizeof(Queue *));

    for (int i = 0; i < num_producers; i++)
    {
        int id = atoi(tokens[i * 3]) - 1;
        int numberOfProducts = atoi(tokens[i * 3 + 1]);
        producers_queues[i] = (BoundedQueue *)malloc(sizeof(BoundedQueue));
        BoundedQueue_init(producers_queues[i], atoi(tokens[i * 3 + 2]));
        struct ProducerParams *params = (struct ProducerParams *)malloc(sizeof(struct ProducerParams));
        params->id = id;
        params->numberOfProducts = numberOfProducts;
        params->queue = producers_queues[i];
        pthread_t thread;
        pthread_create(&thread, NULL, producer_thread, params);
        pthread_detach(thread);
    }


    BoundedQueue *screenQueue = (BoundedQueue *)malloc(sizeof(BoundedQueue));
    BoundedQueue_init(screenQueue, screenCapacity);

    for (int i = 0; i < 3; i++)
    {
        co_editors_queues[i] = (Queue *)malloc(sizeof(Queue));
        Queue_init(co_editors_queues[i]);
        struct CoEditorParams *params = (struct CoEditorParams *)malloc(sizeof(struct CoEditorParams));
        params->id = i + 1;
        params->screen_queue = screenQueue;
        params->co_editor_queue = co_editors_queues[i];
        pthread_t thread;
        pthread_create(&thread, NULL, co_editor_thread, params);
        pthread_detach(thread);
    }

    struct DispatcherParams *dispatcher_params = (struct DispatcherParams *)malloc(sizeof(struct DispatcherParams));
    dispatcher_params->producers_queues = producers_queues;
    dispatcher_params->co_editors_queues = co_editors_queues;
    dispatcher_params->num_producers = num_producers;

    dispatcher_thread(dispatcher_params);
    // pthread_t dispatcher_thread_id;
    // pthread_create(&dispatcher_thread_id, NULL, dispatcher_thread, dispatcher_params);
    // pthread_join(dispatcher_thread_id, NULL);

    for (int i = 0; i < num_producers; i++)
    {
        bounded_queue_destroy(producers_queues[i]);
        free(producers_queues[i]);
    }

    for (int i = 0; i < 3; i++)
    {
        queue_destroy(co_editors_queues[i]);
        free(co_editors_queues[i]);
    }

    free(producers_queues);
    free(co_editors_queues);

    for (int i = 0; i < num_tokens; i++)
    {
        free(tokens[i]);
    }
    free(tokens);

    return 0;
}
