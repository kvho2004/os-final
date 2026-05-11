#ifndef SIMULATION_H
#define SIMULATION_H

#include <pthread.h>
#include <semaphore.h>

/* ── Configuration ─────────────────────────────────────────────────────────── */
typedef struct {
    int numBowls;
    int numCats;
    int numMice;
    int catEat;
    int catFull;
} Config;

/* ── Shared simulation state ────────────────────────────────────────────────── */
typedef struct {
    Config cfg;

    /* Bowl semaphores – one pool for cats, one for mice.
       Both initialised to cfg.numBowls so at most B of each can feed at once.
       Named semaphores used for macOS compatibility.                          */
    sem_t *cat_bowls;
    sem_t *mouse_bowls;

    /* Active-cat counter and its guard.
       The condition variable lets mice block efficiently until active_cats == 0
       and lets an arriving cat broadcast a "flee" signal to feeding mice.    */
    int             active_cats;
    pthread_mutex_t cat_count_mutex;
    pthread_cond_t  no_cats;        /* signalled when active_cats drops to 0  */
    pthread_cond_t  cats_present;   /* signalled when active_cats rises to 1  */
} SimState;

/* ── Thread argument bundles ────────────────────────────────────────────────── */
typedef struct {
    int       id;
    SimState *state;
} CatArgs;

typedef struct {
    int       id;
    SimState *state;
} MouseArgs;

/* ── Function declarations ──────────────────────────────────────────────────── */

/* cat.c */
void *cat_thread(void *arg);

/* mouse.c */
void *mouse_thread(void *arg);

/* Utility – thread-safe timestamped log line */
void sim_log(const char *fmt, ...);

#endif /* SIMULATION_H */