#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "simulation.h"

/*
 * Cat lifecycle (repeats forever until the main thread cancels it):
 *
 *  1. Not hungry  – sleep for N ms
 *  2. Hungry      – wait for a free bowl  (cat_bowls semaphore)
 *  3. Arrive      – increment active_cats; if 0→1, broadcast cats_present
 *                   so any feeding mice know to flee
 *  4. Feed        – sleep for F ms
 *  5. Leave       – decrement active_cats; if 1→0, broadcast no_cats
 *                   so waiting mice can enter
 *  6. Go back to 1
 */
void *cat_thread(void *arg)
{
    CatArgs  *a     = (CatArgs *)arg;
    SimState *state = a->state;
    int       id    = a->id;
    Config *cfg = &state->cfg;

    while (1) {
        /* ── 1. Not hungry ──────────────────────────────────────────────── */
        sim_log("Cat %d: not hungry, resting for %d ms", id, cfg->catFull);
        usleep(cfg->catFull * 1000);

        /* ── 2. Hungry – wait for a free bowl ──────────────────────────── */
        sim_log("Cat %d: hungry, waiting for a bowl", id);
        sem_wait(&state->cat_bowls);

        /* ── 3. Arrive at bowl – update active cat count ────────────────── */
        pthread_mutex_lock(&state->cat_count_mutex);
        state->active_cats++;
        if (state->active_cats == 1) {
            /* First cat to arrive – wake all feeding mice so they flee */
            pthread_cond_broadcast(&state->cats_present);
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        sim_log("Cat %d: START feeding (active cats: %d)", id, state->active_cats);

        /* ── 4. Feed ────────────────────────────────────────────────────── */
        usleep(cfg->catEat * 1000);

        /* ── 5. Leave bowl ──────────────────────────────────────────────── */
        pthread_mutex_lock(&state->cat_count_mutex);
        state->active_cats--;
        sim_log("Cat %d: STOP  feeding (active cats: %d)", id, state->active_cats);
        if (state->active_cats == 0) {
            /* Last cat left – wake all waiting mice */
            pthread_cond_broadcast(&state->no_cats);
        }
        pthread_mutex_unlock(&state->cat_count_mutex);

        sem_post(&state->cat_bowls);
    }

    return NULL;
}