#ifndef TRAIN_H
#define TRAIN_H


typedef struct {
    int id;
    char direction;
    char priority;
    int loading_time;
    int crossing_time;

} Train;

#endif