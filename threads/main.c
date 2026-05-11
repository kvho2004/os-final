#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "simulation.h"

/* ── Logging ────────────────────────────────────────────────────────────────── */

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void sim_log(const char *fmt, ...)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    pthread_mutex_lock(&log_mutex);
    printf("[%8ld ms] ", ms);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

/* ── Usage ──────────────────────────────────────────────────────────────────── */

static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s -B <bowls> -C <cats> -M <mice> -F <feed_ms> -N <rest_ms>\n"
        "  -B  number of bowls\n"
        "  -C  number of cats\n"
        "  -M  number of mice\n"
        "  -F  cat feeding duration in ms\n"
        "  -N  cat not-hungry duration in ms\n",
        prog);
    exit(EXIT_FAILURE);
}

/* ── Main ───────────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));

    /* ── Parse arguments ────────────────────────────────────────────────── */
    Config cfg = { .numBowls = 0, .numCats = 0, .numMice = 0, .catEat = 0, .catFull = 0 };
    int opt;
    while ((opt = getopt(argc, argv, "B:C:M:F:N:")) != -1) {
        switch (opt) {
            case 'B': cfg.numBowls = atoi(optarg); break;
            case 'C': cfg.numCats  = atoi(optarg); break;
            case 'M': cfg.numMice  = atoi(optarg); break;
            case 'F': cfg.catEat   = atoi(optarg); break;
            case 'N': cfg.catFull  = atoi(optarg); break;
            default:  usage(argv[0]);
        }
    }
    if (cfg.numBowls <= 0 || cfg.numCats <= 0 || cfg.numMice <= 0 || cfg.catEat <= 0 || cfg.catFull <= 0)
        usage(argv[0]);

    printf("Starting simulation: B=%d C=%d M=%d F=%d N=%d\n",
           cfg.numBowls, cfg.numCats, cfg.numMice, cfg.catEat, cfg.catFull);

    /* ── Initialise shared state ────────────────────────────────────────── */
    SimState state;
    memset(&state, 0, sizeof(state));
    state.cfg         = cfg;
    state.active_cats = 0;

    /* Named semaphores – required on macOS (sem_init is deprecated) */
    sem_unlink("/cat_bowls");
    sem_unlink("/mouse_bowls");
    state.cat_bowls   = sem_open("/cat_bowls",   O_CREAT, 0644, cfg.numBowls);
    state.mouse_bowls = sem_open("/mouse_bowls", O_CREAT, 0644, cfg.numBowls);
    if (state.cat_bowls == SEM_FAILED || state.mouse_bowls == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&state.cat_count_mutex, NULL);
    pthread_cond_init(&state.no_cats,      NULL);
    pthread_cond_init(&state.cats_present, NULL);

    /* ── Spawn cat threads ──────────────────────────────────────────────── */
    pthread_t *cat_threads = malloc(cfg.numCats * sizeof(pthread_t));
    CatArgs   *cat_args    = malloc(cfg.numCats * sizeof(CatArgs));
    for (int i = 0; i < cfg.numCats; i++) {
        cat_args[i].id    = i + 1;
        cat_args[i].state = &state;
        pthread_create(&cat_threads[i], NULL, cat_thread, &cat_args[i]);
    }

    /* ── Spawn mouse threads ────────────────────────────────────────────── */
    pthread_t *mouse_threads = malloc(cfg.numMice * sizeof(pthread_t));
    MouseArgs *mouse_args    = malloc(cfg.numMice * sizeof(MouseArgs));
    for (int i = 0; i < cfg.numMice; i++) {
        mouse_args[i].id    = i + 1;
        mouse_args[i].state = &state;
        pthread_create(&mouse_threads[i], NULL, mouse_thread, &mouse_args[i]);
    }

    /* ── Run until Enter is pressed ─────────────────────────────────────── */
    printf("Simulation running. Press Enter to stop.\n");
    getchar();

    /* ── Cancel and join all threads ────────────────────────────────────── */
    for (int i = 0; i < cfg.numCats; i++) pthread_cancel(cat_threads[i]);
    for (int i = 0; i < cfg.numMice; i++) pthread_cancel(mouse_threads[i]);
    for (int i = 0; i < cfg.numCats; i++) pthread_join(cat_threads[i], NULL);
    for (int i = 0; i < cfg.numMice; i++) pthread_join(mouse_threads[i], NULL);

    /* ── Cleanup ────────────────────────────────────────────────────────── */
    sem_close(state.cat_bowls);
    sem_close(state.mouse_bowls);
    sem_unlink("/cat_bowls");
    sem_unlink("/mouse_bowls");
    pthread_mutex_destroy(&state.cat_count_mutex);
    pthread_cond_destroy(&state.no_cats);
    pthread_cond_destroy(&state.cats_present);

    free(cat_threads); free(cat_args);
    free(mouse_threads); free(mouse_args);

    printf("Simulation stopped.\n");
    return 0;
}