#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>
#include "house.h"

#define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG][pid %d] " fmt "\n", getpid(), ##__VA_ARGS__)

int NUM_BOWLS = 5;
int NUM_CATS = 10;
int NUM_MICE = 10;
int CAT_EAT_TIME = 2;
int CAT_FULL_TIME = 10;

//cat logic
void cat_logic(House *house, int id) 
{
    time_t start_time = time(NULL);
    while (true) {
        //Cat arrives and gets rid of other mice from bowls
        DEBUG_LOG("cat %d trying to lock mutex before entering, cats_waiting=%d", id, house->cats_waiting);
        sem_wait(house -> mutex);
        DEBUG_LOG("cat %d acquired mutex", id);
        house -> cats_waiting++;
        sem_post(house -> mutex);
        DEBUG_LOG("cat %d released mutex after entering", id);

        //Cat gets a bowl and waits if necessary
        DEBUG_LOG("cat %d waiting for bowl_stack", id);
        sem_wait(house -> bowl_stack);
        DEBUG_LOG("cat %d acquired bowl", id);
        printf("Cat %d is eating\n", id);
        sleep(CAT_EAT_TIME);
        sem_post(house -> bowl_stack);
        DEBUG_LOG("cat %d released bowl", id);

        //Leave when done
        DEBUG_LOG("cat %d trying to lock mutex before leaving", id);
        sem_wait(house -> mutex);
        DEBUG_LOG("cat %d acquired mutex to leave", id);
        house -> cats_waiting--;
        //if no more cats allow all waiting mice to proceed
        if(house->cats_waiting == 0) {
            // Snapshot the count so the loop is stable
            int to_wake = house->mice_waiting; 
            DEBUG_LOG("Last cat leaving. Waking %d mice.", to_wake);
            
            for (int i = 0; i < to_wake; i++) {
                sem_post(house->mice_can_eat);
            }
        }
        sem_post(house -> mutex);
        DEBUG_LOG("cat %d released mutex after leaving", id);

        sleep(CAT_FULL_TIME);
        if (time(NULL) - start_time > 30) {
            DEBUG_LOG("cat %d exiting due to time limit", id);
            break;
        }
    }
}

//mouse logic
void mouse_logic(House *house, int id)
{
    time_t start_time = time(NULL);
    while(true) {
        //Mouse waits until cats are finished
        sem_wait(house -> mutex);
        if (house -> cats_waiting > 0) {
            house -> mice_waiting++;
            DEBUG_LOG("mouse %d queued while cats_waiting=%d", id, house->cats_waiting);
            sem_post(house -> mutex);
            sem_wait(house -> mice_can_eat);
            DEBUG_LOG("mouse %d awakened by broadcast", id);
            sem_wait(house -> mutex);
            house -> mice_waiting--;
        }
        house -> mice_at_bowls++;
        DEBUG_LOG("mouse %d incremented mice_at_bowls=%d", id, house->mice_at_bowls);
        sem_post(house -> mutex);
        DEBUG_LOG("mouse %d released mutex", id);

        //Mouse gets a bowl and waits if necessary
        DEBUG_LOG("mouse %d waiting for bowl_stack", id);
        sem_wait(house -> bowl_stack);
        DEBUG_LOG("mouse %d acquired bowl", id);
        printf("Mouse %d is eating\n", id);

        //If a cat arrives while mouse is eating, mouse should stop eating and release bowl
        // Eat in 10 small "bites" of 0.1 seconds each
        for (int i = 0; i < 10; i++) {
            usleep(100000); 
            
            // Check shared memory for waiting cats
            if (house->cats_waiting > 0) {
                DEBUG_LOG("Mouse %d: CAT! SCATTER!", id);
                break; 
            }
        }
        sem_post(house -> bowl_stack);
        DEBUG_LOG("mouse %d released bowl", id);

        //Leave when done
        sem_wait(house -> mutex);
        house -> mice_at_bowls--;
        DEBUG_LOG("mouse %d decremented mice_at_bowls=%d", id, house->mice_at_bowls);
        sem_post(house -> mutex);

        if (time(NULL) - start_time > 30) {
            DEBUG_LOG("mouse %d exiting due to time limit", id);
            break;
        }
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
    house -> cats_waiting = 0;
    house -> mice_waiting = 0;
    house -> cats_eating = false;

    char mutex_name[64];
    char bowl_name[64];
    char mice_name[64];
    pid_t pid = getpid();

    snprintf(mutex_name, sizeof(mutex_name), "/house_mutex_%d", pid);
    snprintf(bowl_name, sizeof(bowl_name), "/house_bowl_%d", pid);
    snprintf(mice_name, sizeof(mice_name), "/house_mice_%d", pid);

    house -> mutex = sem_open(mutex_name, O_CREAT | O_EXCL, 0600, 1);
    house -> bowl_stack = sem_open(bowl_name, O_CREAT | O_EXCL, 0600, NUM_BOWLS);
    house -> mice_can_eat = sem_open(mice_name, O_CREAT | O_EXCL, 0600, 0);

    if (house -> mutex == SEM_FAILED || house -> bowl_stack == SEM_FAILED || house -> mice_can_eat == SEM_FAILED) {
        perror("sem_open failed");
        return 1;
    }

    pid_t *child_pids = malloc(sizeof(pid_t) * (NUM_CATS + NUM_MICE));
    if (!child_pids) {
        perror("malloc failed");
        return 1;
    }
    int child_count = 0;

    //forking logic
    int cat_index = 0;
    int mouse_index = 0;
    while (cat_index < NUM_CATS || mouse_index < NUM_MICE) {
        if (cat_index < NUM_CATS) {
            pid_t cpid = fork();
            if (cpid == 0) {
                cat_logic(house, cat_index);
                sem_close(house -> mutex);
                sem_close(house -> bowl_stack);
                sem_close(house -> mice_can_eat);
                exit(0);
            }
            child_pids[child_count++] = cpid;
            cat_index++;
        }

        if (mouse_index < NUM_MICE) {
            pid_t mpid = fork();
            if (mpid == 0) {
                mouse_logic(house, mouse_index);
                sem_close(house -> mutex);
                sem_close(house -> bowl_stack);
                sem_close(house -> mice_can_eat);
                exit(0);
            }
            child_pids[child_count++] = mpid;
            mouse_index++;
        }
    }

    fprintf(stderr, "[MAIN] running children for 30 seconds\n");
    sleep(30);
    fprintf(stderr, "[MAIN] stopping child processes\n");
    for (int i = 0; i < child_count; i++) {
        kill(child_pids[i], SIGTERM);
    }
    for (int i = 0; i < child_count; i++) {
        waitpid(child_pids[i], NULL, 0);
    }
    free(child_pids);

    //Take in constants from user call
    
    if (argc == 6) {
        NUM_BOWLS = atoi(argv[1]);
        NUM_CATS = atoi(argv[2]);
        NUM_MICE = atoi(argv[3]);
        CAT_EAT_TIME = atoi(argv[4]);
        CAT_FULL_TIME = atoi(argv[5]);
    }

    sem_close(house -> mutex);
    sem_close(house -> bowl_stack);
    sem_close(house -> mice_can_eat);
    sem_unlink(mutex_name);
    sem_unlink(bowl_name);
    sem_unlink(mice_name);
    munmap(house, sizeof(House));

    return 0;
}