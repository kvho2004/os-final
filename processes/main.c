#include <stdio.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include "house.h"

int NUM_BOWLS = 5;
int NUM_CATS = 10;
int NUM_MICE = 10;
int CAT_EAT_TIME = 2;
int CAT_FULL_TIME = 3;

//cat logic
void cat_logic(House *house, int id) 
{
    //Cat arrives and gets rid of other mice from bowls
    sem_wait(&house -> mutex);
    house -> cats_waiting++;
    if(house -> cats_waiting == 1) {
        sem_wait(&house -> mice_can_eat);
    }
    sem_post(&house -> mutex);

    //Cat gets a bowl and waits if necessary
    sem_wait(&house -> bowl_stack);
    printf("Cat %d is eating\n", id);
    sleep(CAT_EAT_TIME);
    sem_post(&house -> bowl_stack);

    //Leave when done
    sem_wait(&house -> mutex);
    house -> cats_waiting--;
    //if no more cats allow mice to eat
    if(house -> cats_waiting == 0) {
        sem_post(&house -> mice_can_eat);
    }
    sem_post(&house -> mutex);

}

//mouse logic
void mouse_logic(House *house, int id)
{
    while(true) {
        //Mouse arrives and waits if cats are eating
        sem_wait(&house -> mutex);
        if(house -> cats_waiting > 0) {
            printf("Mouse %d is waiting\n", id);
        }
        while(house -> cats_waiting > 0) {
            sem_post(&house -> mutex);
            sleep(1);
            sem_wait(&house -> mutex);
        }
        house -> mice_at_bowls++;
        sem_post(&house -> mutex);

        //Mouse gets a bowl and waits if necessary
        sem_wait(&house -> bowl_stack);
        printf("Mouse %d is eating\n", id);
        sleep(1);
        sem_post(&house -> bowl_stack);

        //Leave when done
        sem_wait(&house -> mutex);
        house -> mice_at_bowls--;
        sem_post(&house -> mutex);
    }
}


int main(int argc, char *argv[])
{
    //House set up
    House *house = mmap(NULL, sizeof(House), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (house == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    house -> cats_at_bowls = 0;
    house -> mice_at_bowls = 0;
    house -> cats_waiting = NUM_CATS;
    house -> cats_eating = false;

    sem_init(&house -> mutex, 1, 1);
    sem_init(&house -> bowl_stack, 1, NUM_BOWLS);
    sem_init(&house -> mice_can_eat, 1, 0);

    //forking logic
    for(int i = 0; i < NUM_CATS; i++) {
        if(fork() == 0) {
            cat_logic(house, i);
            exit(0);
        }
    }



    //Take in constants from user call
    /*
    if (argc == 6) {
        NUM_BOWLS = atoi(argv[1]);
        NUM_CATS = atoi(argv[2]);
        NUM_MICE = atoi(argv[3]);
        CAT_EAT_TIME = atoi(argv[4]);
        CAT_FULL_TIME = atoi(argv[5]);
    }
    */

    sem_destroy(&house -> mutex);
    munmap(house, sizeof(House));

    return 0;
}