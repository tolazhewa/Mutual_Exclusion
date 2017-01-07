#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NITEMS 3

typedef struct circ_buff_tag {
    int length;
    int *base;
    int *head;
    int *tail;
}circ_buff;

int P, C, N;

circ_buff buffer;

sem_t stock_sem;
sem_t free_sem;
pthread_mutex_t lock_mut;

pthread_t *cT;
pthread_t *pT;

void *consumer(void *arg);
void *producer(void *arg);
void circ_buff_insert(circ_buff *cb, int item);
int circ_buff_remove(circ_buff *cb);
void circ_buff_print(circ_buff *cb);
int circ_buff_init(circ_buff *cb, int l);
void *consumerWithout(void *arg);
void *producerWithout(void *arg);
void circ_buff_free(circ_buff *cb);

int main(int argc, char *argv[])
{
    int i;
    char *buff = (char*)calloc(sizeof(char), 24);

    printf("Please enter the number of producer threads: ");
    fgets(buff, 24, stdin);
    P = (int)strtol(buff, NULL, 10);

    printf("Please enter the number of consumer threads: ");
    fgets(buff, 24, stdin);
    C = (int)strtol(buff, NULL, 10);

    printf("Please enter size of buffer: ");
    fgets(buff, 24, stdin);
    N = (int)strtol(buff, NULL, 10);

    cT = (pthread_t*)calloc(sizeof(pthread_t), C);
    pT = (pthread_t*)calloc(sizeof(pthread_t), P);

    circ_buff_init(&buffer, N);
    int *arr = (int*)calloc(sizeof(int), P+C);

    sem_init(&stock_sem, 0, 0);
    sem_init(&free_sem, 0, N);

    printf("\n\nWith Mutual Exclusions\n\n\n\n");

    if (pthread_mutex_init(&lock_mut, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    for(i = 0; i < P; i++)
    {
        arr[i] = i + 1;
        pthread_create(&(pT[i]), NULL, producer, &arr[i]);
    }

    for(; i < C + P; i++)
    {
        arr[i] = i + 1;
        pthread_create(&(cT[i - P]), NULL, consumer, &arr[i]);
    }

    for(i = 0; i < P; i++)
    {
        pthread_join(pT[i], NULL);
    }

    for(i = 0; i < C; i++)
    {
        pthread_join(cT[i], NULL);
    }

    printf("\n\nWithout Mutual Exclusions\n\n\n");

    circ_buff_free(&buffer);
    circ_buff_init(&buffer, N);

    for(i = 0; i < P; i++)
    {
        arr[i] = i + 1;
        pthread_create(&(pT[i]), NULL, producerWithout, &arr[i]);
    }

    for(; i < C + P; i++)
    {
        arr[i] = i + 1;
        pthread_create(&(cT[i]), NULL, consumerWithout, &arr[i]);
    }

    for(i = 0; i < P; i++)
    {
        pthread_join(pT[i], NULL);
    }

    for(i = 0; i < C; i++)
    {
        pthread_join(cT[i], NULL);
    }

    sem_destroy(&free_sem);
    sem_destroy(&stock_sem);
    pthread_mutex_destroy(&lock_mut);

    free(arr);
    free(pT);
    free(cT);
    circ_buff_free(&buffer);

    printf("Finished\n");

    return 0;
}

void circ_buff_insert(circ_buff *cb, int item)
{
    *(cb->head) = item;
    (cb->head)++;
    if(cb->head >= cb->base + cb->length)
    {
        cb->head = cb->base;
    }
}

void circ_buff_free(circ_buff *cb)
{
  free(cb->base);
}

int circ_buff_remove(circ_buff *cb)
{
    int item = *(cb->tail);
    *(cb->tail) = 0;
    (cb->tail)++;
    if(cb->tail >= cb->base + cb->length)
    {
        cb->tail = cb->base;
    }

    return item;
}

void circ_buff_print(circ_buff *cb)
{
    int i;
    printf("Buffer: [");
    for(i = 0; i < cb->length-1; i++)
    {
        printf("%d,", cb->base[i]);
    }
    printf("%d]\n\n", cb->base[cb->length - 1]);
}

int circ_buff_init(circ_buff *cb, int l)
{
    if((cb->base = (int*)calloc(sizeof(int), l)) == 0)
    {
        printf("Circular Buffer memory allocation unsuccessful!\n");
        return -1;
    };
    cb->length = l;
    cb->head = cb->base;
    cb->tail = cb->base;
    return 0;
}

void *producer(void *arg)
{
    int i;
    for(i = 0; i < NITEMS; i++)
    {
        int item = (rand() % 20) + 1;

        sem_wait(&free_sem);
        pthread_mutex_lock(&lock_mut);

        printf("Produce: item: %d, index: %lu, thread: %d [P]\n", item, (buffer.head - buffer.base), *(int*)arg);
        circ_buff_insert(&buffer, item);
        circ_buff_print(&buffer);

        pthread_mutex_unlock(&lock_mut);
        sem_post(&stock_sem);
    }
    return NULL;
}

void *consumer(void *arg)
{
    int i;
    for(i = 0; i < NITEMS; i++)
    {
        sem_wait(&stock_sem);
        pthread_mutex_lock(&lock_mut);

        printf("Consume: item: %d, index: %lu, thread: %d [C]\n", *(buffer.tail), buffer.tail - buffer.base, *(int*)arg);

        circ_buff_remove(&buffer);
        circ_buff_print(&buffer);

        pthread_mutex_unlock(&lock_mut);
        sem_post(&free_sem);
    }
    return NULL;
}

void *producerWithout(void *arg)
{
    int i;
    for(i = 0; i < NITEMS; i++)
    {
        int item = (rand() % 20) + 1;
        printf("Produce: item: %d, index: %lu, thread: %d [P]\n\n", item, (buffer.head - buffer.base), *(int*)arg);
        circ_buff_insert(&buffer, item);
    }
    return NULL;
}

void *consumerWithout(void *arg)
{
    int i;
    for(i = 0; i < NITEMS; i++)
    {
        printf("Consume: item: %d, index: %lu, thread: %d [C]\n\n", *(buffer.tail), buffer.tail - buffer.base, *(int*)arg);
        circ_buff_remove(&buffer);
    }
    return NULL;
}
