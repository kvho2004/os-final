#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "simulation.h"

/*
 * Mouse lifecycle (repeats forever until the main thread cancels it):
 *
 *  1. Not hungry  – sleep a random short interval
 *  2. Hungry      – wait until active_cats == 0  (block on no_cats cond var)
 *  3. Wait for a free mouse bowl
 *  4. Feed        – sleep in short bursts, checking cats_present each burst
 *                   If a cat arrives mid-feed, flee immediately (rule 11-12)
 *  5. Release bowl and go back to 1
 *
 * The key subtlety: steps 2 and 4 both watch active_cats, but in different ways.
 *   - Step 2 uses a condition variable wait so the thread truly sleeps.
 *   - Step 4 uses a timed wait on the same cond var so we can be interrupted.
 */
void *mouse_thread(void *arg)
{
    MouseArgs *a     = (MouseArgs *)arg;
    SimState  *state = a->state;
    int        id    = a->id;

    // Setting length of feeding burst before rechecking for cats
    const int BURST_MS = 50;

    while (1) {
        // Not hungry
        int rest = 100 + rand() % 400; 
        sim_log("Mouse %d: not hungry, resting %d ms", id, rest);
        usleep(rest * 1000);

        // Wait until no cats
        pthread_mutex_lock(&state->cat_count_mutex);
        while (state->active_cats > 0) {
            sim_log("Mouse %d: cats present, waiting...", id);
            pthread_cond_wait(&state->no_cats, &state->cat_count_mutex);
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        // Wait for open bowl
        sim_log("Mouse %d: no cats, waiting for a bowl", id);
        sem_wait(state->mouse_bowls);

        // Recheck if cat has arrived
        pthread_mutex_lock(&state->cat_count_mutex);
        if (state->active_cats > 0) {
            pthread_mutex_unlock(&state->cat_count_mutex);
            sim_log("Mouse %d: cat arrived just before feeding – fleeing", id);
            sem_post(state->mouse_bowls);
            continue;
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        sim_log("Mouse %d: START feeding", id);

        // Feed in small increments + leave if a cat arrives
        int fled = 0;
        while (!fled) {
            // Sleep for one burst
            usleep(BURST_MS * 1000);

            // Check for cat
            pthread_mutex_lock(&state->cat_count_mutex);
            if (state->active_cats > 0) {
                fled = 1;
                sim_log("Mouse %d: cat arrived – FLEEING", id);
            }
            pthread_mutex_unlock(&state->cat_count_mutex);
        }

        // Free bowl
        if (!fled) {
            sim_log("Mouse %d: STOP feeding (finished normally)", id);
        }
        sem_post(state->mouse_bowls);
    }

    return NULL;
}