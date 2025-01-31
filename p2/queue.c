#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

void init_queue(Queue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->lock, NULL);
}

int is_queue_empty(Queue *queue) {
    return queue->head == NULL;
}

Train* peek(Queue *queue) {
    if (queue->head == NULL) {
        return NULL; // Queue is empty
    }
    return queue->head->train; // Return the train at the front
}

void enqueue(Queue *queue, Train *train) {
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    new_node->train = train;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->lock);

    if (queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    pthread_mutex_unlock(&queue->lock);
}

Train *dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->lock);

    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    QueueNode *node = queue->head;
    Train *train = node->train;
    queue->head = node->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    pthread_mutex_unlock(&queue->lock);
    free(node);
    return train;
}

void destroy_queue(Queue *queue) {
    while (dequeue(queue) != NULL); // Clear remaining nodes
    pthread_mutex_destroy(&queue->lock);
}

void print_queue(Queue *queue) {
    pthread_mutex_lock(&queue->lock); // Lock the queue for thread safety
    QueueNode *current = queue->head;

    if (current == NULL) {
        printf("The queue is empty.\n");
    } else {
        printf("Queue contents:\n");
        while (current != NULL) {
            // Assuming Train has fields id, direction, priority, loading_time, crossing_time
            printf("Train ID: %d, Direction: %c, Priority: %c, Loading Time: %d, Crossing Time: %d\n",
                current->train->id,
                current->train->direction,
                current->train->priority,
                current->train->loading_time,
                current->train->crossing_time);
            current = current->next;  // Move to the next node
        }
    }

    pthread_mutex_unlock(&queue->lock); // Unlock the queue
}
