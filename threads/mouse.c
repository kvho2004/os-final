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

    /* How long each "feeding burst" lasts before rechecking for cats (ms).
       Smaller = more responsive to cat arrival; larger = less CPU overhead.  */
    const int BURST_MS = 50;

    while (1) {
        /* ── 1. Not hungry ──────────────────────────────────────────────── */
        int rest = 100 + rand() % 400;   /* 100–500 ms */
        sim_log("Mouse %d: not hungry, resting %d ms", id, rest);
        usleep(rest * 1000);

        /* ── 2. Wait until no cats are feeding ─────────────────────────── */
        pthread_mutex_lock(&state->cat_count_mutex);
        while (state->active_cats > 0) {
            sim_log("Mouse %d: cats present, waiting...", id);
            pthread_cond_wait(&state->no_cats, &state->cat_count_mutex);
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        /* ── 3. Wait for a free bowl ────────────────────────────────────── */
        sim_log("Mouse %d: no cats, waiting for a bowl", id);
        sem_wait(state->mouse_bowls);

        /*
         * Re-check: a cat may have arrived between steps 2 and 3.
         * If so, give up the bowl immediately and go back to waiting.
         */
        pthread_mutex_lock(&state->cat_count_mutex);
        if (state->active_cats > 0) {
            pthread_mutex_unlock(&state->cat_count_mutex);
            sim_log("Mouse %d: cat arrived just before feeding – fleeing", id);
            sem_post(state->mouse_bowls);
            continue;
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        sim_log("Mouse %d: START feeding", id);

        /* ── 4. Feed in short bursts, fleeing if a cat arrives ──────────── */
        int fled = 0;
        while (!fled) {
            /* Sleep one burst */
            usleep(BURST_MS * 1000);

            /* Check whether a cat has arrived */
            pthread_mutex_lock(&state->cat_count_mutex);
            if (state->active_cats > 0) {
                fled = 1;
                sim_log("Mouse %d: cat arrived – FLEEING", id);
            }
            pthread_mutex_unlock(&state->cat_count_mutex);

            /*
             * Optional: break out after a random total feeding time even
             * without a cat, so mice don't feed forever when no cats come.
             * Uncomment and adjust as needed:
             *
             * if (!fled && (rand() % 10 == 0)) break;
             */
        }

        /* ── 5. Release bowl ────────────────────────────────────────────── */
        if (!fled) {
            sim_log("Mouse %d: STOP feeding (finished normally)", id);
        }
        sem_post(state->mouse_bowls);
    }

    return NULL;
}