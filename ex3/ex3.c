#include "ex3.h"

// splitConfigFile the config file
char **splitConfigFile(const char *s, char delim, int *numTokens)
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
    *numTokens = count;
    return tokens;
}

// Bounded queue functions
void boundedQueueInit(BoundedQueue *queue, int max_size)
{
    queue->buffer = malloc(max_size * sizeof(char *));
    queue->capacity = max_size;
    queue->size = 0;
    queue->headIndex = 0;
    queue->tailIndex = 0;

    semaphoreInit(&queue->empty_sem, max_size);
    semaphoreInit(&queue->full_sem, 0);
    pthread_mutex_init(&queue->mutex, NULL);
}

void boundedQueuePush(BoundedQueue *queue, char *s)
{
    // Decrease one to the empty semaphore
    semaphoreDecrease(&queue->empty_sem);
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
    // Increase one from the full semaphore
    semaphoreIncrease(&queue->full_sem);
}

char *boundedQueuePop(BoundedQueue *queue)
{
    if (queue->buffer[queue->headIndex] == NULL)
    {
        return NULL;
    }
    semaphoreDecrease(&queue->full_sem);
    pthread_mutex_lock(&queue->mutex);
    char *result = queue->buffer[queue->headIndex];
    queue->headIndex = (queue->headIndex + 1) % queue->capacity;
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    semaphoreIncrease(&queue->empty_sem);
    return result;
}

void boundedQueueDestroy(BoundedQueue *queue)
{
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    pthread_mutex_destroy(&queue->full_sem.mutex);
    pthread_mutex_destroy(&queue->full_sem.cond);
    pthread_mutex_destroy(&queue->empty_sem.mutex);
    pthread_mutex_destroy(&queue->empty_sem.cond);
}

// Unbounded queue functions
void unBoundedQueueInit(Queue *queue)
{
    queue->buffer = malloc(sizeof(char *));
    queue->buffer[0] = NULL;
    queue->capacity = 1; // Start with a capacity of 1
    queue->size = 0;
    queue->headIndex = 0;
    queue->tailIndex = 0;
    semaphoreInit(&queue->full_sem, 0);
    pthread_mutex_init(&queue->mutex, NULL);
}

void unBoundedQueuePush(Queue *queue, const char *s)
{
    pthread_mutex_lock(&queue->mutex);
    if (queue->size >= queue->capacity)
    {
        // Increase the capacity dynamically
        int new_capacity = queue->capacity * 2;
        char **new_buffer = realloc(queue->buffer, new_capacity * sizeof(char *));
        if (new_buffer == NULL)
        {
            free(queue->buffer);
            return;
        }
        else
        {
            queue->buffer = new_buffer;
            queue->capacity = new_capacity;
        }
    }
    queue->buffer[queue->tailIndex] = strdup(s);
    queue->tailIndex = (queue->tailIndex + 1) % queue->capacity;
    queue->size++;
    pthread_mutex_unlock(&queue->mutex);
    semaphoreIncrease(&queue->full_sem);
}

char *unBondedQueuePop(Queue *queue)
{
    if (queue->buffer[queue->headIndex] == NULL)
    {
        return NULL;
    }
    semaphoreDecrease(&queue->full_sem);
    pthread_mutex_lock(&queue->mutex);
    char *result = queue->buffer[queue->headIndex];
    queue->headIndex++;
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

void unBondedQueueDestroy(Queue *queue)
{
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    pthread_mutex_destroy(&queue->full_sem.mutex);
    pthread_mutex_destroy(&queue->full_sem.cond);
}

// Counting Semaphore functions
void semaphoreInit(CountingSemaphore *semaphore, int startValue)
{
    semaphore->value = startValue;
    pthread_mutex_init(&semaphore->mutex, NULL);
    pthread_mutex_init(&semaphore->cond, NULL);
    pthread_mutex_lock(&semaphore->cond);
}

void semaphoreDecrease(CountingSemaphore *semaphore)
{
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->value--;
    if (semaphore->value < 0)
    {
        pthread_mutex_unlock(&semaphore->mutex);
        pthread_mutex_lock(&semaphore->cond);
    }
    pthread_mutex_unlock(&semaphore->mutex);
}

void semaphoreIncrease(CountingSemaphore *semaphore)
{
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->value++;
    if (semaphore->value <= 0)
    {
        pthread_mutex_unlock(&semaphore->cond);
    }
    else
    {
        pthread_mutex_unlock(&semaphore->mutex);
    }
}

void *handleProducer(void *arg)
{
    ProducerParams *params = (ProducerParams *)arg;
    int id = params->id;
    int numberOfProducts = params->numberOfProducts;
    BoundedQueue *queue = params->queue;

    int sports = 0, news = 0, weather = 0;
    srand(time(NULL)); // Set the seed value using the current time

    for (int i = 0; i < numberOfProducts; ++i)
    {
        int type = rand() % 3;
        char product[MAX_STRING_LENGTH];
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
        boundedQueuePush(queue, s);
    }
    boundedQueuePush(queue, "DONE");
    free(params);
    return NULL;
}

void *handleDispatcher(void *arg)
{
    DispatcherParams *dispatcher = (DispatcherParams *)arg;
    BoundedQueue **producersQueues = dispatcher->producersQueues;
    Queue **coEditorsQueues = dispatcher->coEditorsQueues;
    int producersNumber = dispatcher->producersNumber;

    int current_queue = 0;
    int ended = 0;

    while (ended != producersNumber)
    {
        if (producersQueues[current_queue] == NULL)
        {
            current_queue = (current_queue + 1) % producersNumber;
            continue;
        }
        char *poppedItem = boundedQueuePop(producersQueues[current_queue]);
        int previous_queue = current_queue;
        current_queue = (current_queue + 1) % producersNumber;
        if (poppedItem == NULL)
        {
            continue;
        }
        if (strcmp(poppedItem, "DONE") == 0)
        {
            free(poppedItem);
            boundedQueueDestroy(producersQueues[previous_queue]);
            free(producersQueues[previous_queue]);
            producersQueues[previous_queue] = NULL;

            ended++;
            continue;
        }
        if (strstr(poppedItem, "SPORTS") != NULL)
        {
            unBoundedQueuePush(coEditorsQueues[0], poppedItem);
        }
        if (strstr(poppedItem, "NEWS") != NULL)
        {
            unBoundedQueuePush(coEditorsQueues[1], poppedItem);
        }
        if (strstr(poppedItem, "WEATHER") != NULL)
        {
            unBoundedQueuePush(coEditorsQueues[2], poppedItem);
        }
        free(poppedItem);
    }

    for (int i = 0; i < 3; ++i)
    {
        unBoundedQueuePush(coEditorsQueues[i], "DONE");
    }
    free(dispatcher);
    return NULL;
}

void *handleCoEditor(void *arg)
{
    CoEditorParams *coEditor = (CoEditorParams *)arg;

    while (1)
    {
        char *s = unBondedQueuePop(coEditor->coEditorQueue);
        if (s == NULL)
        {
            continue;
        }
        if (strcmp(s, "DONE") == 0)
        {
            free(s);
            unBondedQueueDestroy(coEditor->coEditorQueue);
            free(coEditor->coEditorQueue);
            break;
        }
        usleep(100000);
        boundedQueuePush(coEditor->screenQueue, s);
        free(s);
    }
    boundedQueuePush(coEditor->screenQueue, "DONE");
    free(coEditor);
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Arguments error\n");
        return 1;
    }

    // Open the config file
    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", argv[1]);
        return 1;
    }

    // Read the file
    char config[MAX_STRING_LENGTH + 1];
    int temp = fread(config, sizeof(char), MAX_STRING_LENGTH, file);
    config[temp] = '\0';
    fclose(file);

    // Split the config file
    int numTokens;
    char **tokens = splitConfigFile(config, '\n', &numTokens);

    // Get the Producers number and the Screen Manager queue capacity
    int producersNumber = numTokens / 3;
    int screenCapacity = atoi(tokens[numTokens - 1]);

    BoundedQueue **producersQueues = (BoundedQueue **)malloc(producersNumber * sizeof(BoundedQueue *));
    Queue **coEditorsQueues = (Queue **)malloc(3 * sizeof(Queue *));

    // Init producers and activate their threads
    for (int i = 0; i < producersNumber; i++)
    {
        producersQueues[i] = (BoundedQueue *)malloc(sizeof(BoundedQueue));
        boundedQueueInit(producersQueues[i], atoi(tokens[i * 3 + 2]));

        int id = atoi(tokens[i * 3]) - 1;
        int numberOfProducts = atoi(tokens[i * 3 + 1]);

        ProducerParams *params = (ProducerParams *)malloc(sizeof(ProducerParams));
        params->id = id;
        params->numberOfProducts = numberOfProducts;
        params->queue = producersQueues[i];

        pthread_t producerThread;
        pthread_create(&producerThread, NULL, handleProducer, params);
        pthread_detach(producerThread);
    }

    // Create the screen queue
    BoundedQueue *screenQueue = (BoundedQueue *)malloc(sizeof(BoundedQueue));
    boundedQueueInit(screenQueue, screenCapacity);

    // Create co editors
    for (int i = 0; i < 3; i++)
    {
        coEditorsQueues[i] = (Queue *)malloc(sizeof(Queue));
        unBoundedQueueInit(coEditorsQueues[i]);
    }

    // Init dispatcher and activate its thread
    DispatcherParams *dispatcher = (DispatcherParams *)malloc(sizeof(DispatcherParams));
    dispatcher->producersQueues = producersQueues;
    dispatcher->coEditorsQueues = coEditorsQueues;
    dispatcher->producersNumber = producersNumber;

    pthread_t dispatcherThread;
    pthread_create(&dispatcherThread, NULL, handleDispatcher, dispatcher);

    // Init co editors and activate their threads
    for (int i = 0; i < 3; i++)
    {
        CoEditorParams *coEditor = (CoEditorParams *)malloc(sizeof(CoEditorParams));
        coEditor->id = i;
        coEditor->screenQueue = screenQueue;
        coEditor->coEditorQueue = coEditorsQueues[i];
        pthread_t coEditorThread;
        pthread_create(&coEditorThread, NULL, handleCoEditor, coEditor);
        pthread_detach(coEditorThread);
    }

    // The screen is running on the main thread
    int ended = 0;
    while (ended != 3)
    {
        char *s = boundedQueuePop(screenQueue);
        if (s == NULL)
        {
            continue;
        }
        if (strcmp(s, "DONE") == 0)
        {
            free(s);
            ended++;
            continue;
        }
        printf("%s\n", s);
        free(s);
    }
    printf("DONE\n");

    // Free the resources
    boundedQueueDestroy(screenQueue);

    free(producersQueues);
    free(coEditorsQueues);
    free(screenQueue);

    for (int i = 0; i < numTokens; i++)
    {
        free(tokens[i]);
    }
    free(tokens);

    return 0;
}
