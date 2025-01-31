#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "train.h"


typedef struct QueueNode {
    Train *train;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    pthread_mutex_t lock;
} Queue;

void init_queue(Queue *queue);
int is_queue_empty(Queue *queue);
Train *peek(Queue *queue);
void enqueue(Queue *queue, Train *train);
Train *dequeue(Queue *queue);
void destroy_queue(Queue *queue);
void print_queue(Queue *queue);

#endif