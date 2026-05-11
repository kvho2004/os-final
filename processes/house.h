
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
typedef struct {
    //Counters
    int cats_at_bowls;
    int mice_at_bowls;
    int cats_waiting;
    int mice_waiting;
    bool cats_eating;


    //rules
    sem_t *mutex;
    sem_t *bowl_stack;
    sem_t *mice_can_eat;

} House;
