#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h> 
#include <unistd.h>
#include <time.h>
#include "queue.h"
#include "train.h"

#define INITIAL_CAPACITY 10

typedef struct {
    Train* trains;
    int num_trains;
} TrainList;

pthread_cond_t start_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t crossing_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t crossing_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_for_all_trains = PTHREAD_MUTEX_INITIALIZER;

int start_signal = 0;
int train_crossing = 0;
int trains_remaining;
char last_train_direction = 'w';
int threads_ready = 0;

Queue East_Priority_Low;
Queue East_Priority_High;
Queue West_Priority_Low;
Queue West_Priority_High;

struct timespec program_start_time;
FILE *output_file;

// Logs an event with a basic timestamp in seconds and 1 millisecond precision
void log_event(const char *event, int train_id) {
    pthread_mutex_lock(&file_mutex);

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    long seconds = current_time.tv_sec - program_start_time.tv_sec;
    long milliseconds = (current_time.tv_nsec - program_start_time.tv_nsec) / 1000000;

    if (milliseconds < 0) { 
        milliseconds += 1000;
        seconds -= 1;
    }

    long hours = seconds / 3600;
    long minutes = (seconds % 3600) / 60;
    long remaining_seconds = seconds % 60;

    fprintf(output_file, "%02ld:%02ld:%02ld.%01ld Train %2d %s\n", 
            hours, minutes, remaining_seconds, milliseconds / 100, train_id, event);
    //printf("%02ld:%02ld:%02ld.%01ld Train %2d %s\n", hours, minutes, remaining_seconds, milliseconds / 100, train_id, event);
    fflush(output_file);

    pthread_mutex_unlock(&file_mutex);
}

//reads input.txt and outputs a trainlist obect which is just a list of trains and a count of the number of trains
TrainList read_input(const char *filename) {
    FILE *file = fopen(filename, "r"); 
    if (!file) { 
        perror("Failed file opening");
        exit(EXIT_FAILURE);
    }

    int capacity = INITIAL_CAPACITY;
    Train *trains = malloc(capacity * sizeof(Train));
    if (!trains) {
        perror("failed malloc");
        exit(EXIT_FAILURE);
    }

    int num_trains = 0;
    while (!feof(file)) {
        Train t;
        if (fscanf(file, "%c %d %d", &t.direction, &t.loading_time, &t.crossing_time) == 3) {
            t.priority = (t.direction == 'e' || t.direction == 'w') ? 'l' : 'h';
            t.id = num_trains;
            if (num_trains >= capacity) {
                capacity *= 2;
                trains = realloc(trains, capacity * sizeof(Train));
                if (!trains) {
                    perror("failed realloc");
                    exit(EXIT_FAILURE);
                }
            }
            trains[num_trains] = t;
            num_trains++;
        }
    }
    fclose(file);

    TrainList train_list;
    train_list.trains = trains;
    train_list.num_trains = num_trains;

    for (int i = 0; i < num_trains; i++) {
        trains[i].direction = tolower(trains[i].direction);
    }

    return train_list;
}
//thread for each train. waits for start signal, enqueues to appropriate queue,prints ready to go message, signals main
void *train_thread(void *arg) {
    Train *train = (Train *)arg;

    pthread_mutex_lock(&wait_for_all_trains);
    threads_ready++;
    pthread_mutex_unlock(&wait_for_all_trains);

    pthread_mutex_lock(&start_mutex);

    while (!start_signal) {
        pthread_cond_wait(&start_condition, &start_mutex);
    }
    pthread_mutex_unlock(&start_mutex);

    usleep(train->loading_time * 100000);

    pthread_mutex_lock(&crossing_mutex);
    if (train->direction == 'e' && train->priority == 'l') {
        enqueue(&East_Priority_Low, train);
    } else if (train->direction == 'e' && train->priority == 'h') {
        enqueue(&East_Priority_High, train);
    } else if (train->direction == 'w' && train->priority == 'l') {
        enqueue(&West_Priority_Low, train);
    } else {
        enqueue(&West_Priority_High, train);
    }

    char event[50];
    sprintf(event, "is ready to go %s", train->direction == 'e' ? "East" : "West");
    log_event(event,train->id);

    pthread_cond_signal(&crossing_condition);
    pthread_mutex_unlock(&crossing_mutex);

    pthread_exit(NULL);
}

//selecting train logic for which train should go next. INCOMPLETE: NO STARVATION, LIMITED SPEC ACCURACY
Train *select_and_signal_train() {
    Train *train = NULL;
    pthread_mutex_lock(&crossing_mutex);
    while (!train_crossing && trains_remaining > 0) {
        if (!is_queue_empty(&East_Priority_High)) {
            train = dequeue(&East_Priority_High);
        } else if (!is_queue_empty(&West_Priority_High)) {
            train = dequeue(&West_Priority_High);
        } else if (!is_queue_empty(&East_Priority_Low)) {
            train = dequeue(&East_Priority_Low);
        } else if (!is_queue_empty(&West_Priority_Low)) {
            train = dequeue(&West_Priority_Low);
        } else {
            if (trains_remaining == 0) {
                break;
            }
            pthread_cond_wait(&crossing_condition, &crossing_mutex);
            continue;
        }
        if (train) {
            train_crossing = 1;
            pthread_cond_signal(&crossing_condition);
            last_train_direction = train->direction;
        }
    }
    pthread_mutex_unlock(&crossing_mutex);
    return train;
}
//DISPACTH THREAD. uses select train, simulates selected train crossing, logs crossing and crossing finish
void *main_control(void *arg) {
    Train *train = NULL;
    while (trains_remaining > 0) {
        train = select_and_signal_train();
        if (train != NULL) {
            char event_on[50];
            sprintf(event_on, "is ON the main track going %s", train->direction == 'e' ? "East" : "West");
            log_event(event_on,train->id);
            
            usleep(train->crossing_time * 100000);
            
            char event_off[50];
            sprintf(event_off, "is OFF the main track after going %s", train->direction == 'e' ? "East" : "West");
            log_event(event_off,train->id);

            pthread_mutex_lock(&crossing_mutex);
            train_crossing = 0;
            trains_remaining--;
            pthread_cond_signal(&crossing_condition);
            pthread_mutex_unlock(&crossing_mutex);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    const char* filename = argv[1];
    TrainList train_list = read_input(filename);
    trains_remaining = train_list.num_trains;

    output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    clock_gettime(CLOCK_MONOTONIC, &program_start_time);

    init_queue(&East_Priority_Low);
    init_queue(&East_Priority_High);
    init_queue(&West_Priority_Low);
    init_queue(&West_Priority_High);

    pthread_t threads[train_list.num_trains];

    for (int i = 0; i < train_list.num_trains; i++) {
        if (pthread_create(&threads[i], NULL, train_thread, (void *)&train_list.trains[i])) {
            fprintf(stderr, "Error creating thread for train %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    while (threads_ready < train_list.num_trains) {
        usleep(100);
    }
    

    pthread_mutex_lock(&start_mutex);
    start_signal = 1;
    pthread_cond_broadcast(&start_condition);
    pthread_mutex_unlock(&start_mutex);

    pthread_t controller_thread;
    pthread_create(&controller_thread, NULL, main_control, NULL);

    for (int i = 0; i < train_list.num_trains; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(controller_thread, NULL);

    free(train_list.trains);
    fclose(output_file);
    pthread_mutex_destroy(&start_mutex);
    pthread_cond_destroy(&start_condition);
    pthread_mutex_destroy(&crossing_mutex);
    pthread_cond_destroy(&crossing_condition);
    pthread_mutex_destroy(&file_mutex);

    return 0;
}
