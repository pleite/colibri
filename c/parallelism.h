#ifndef COLIBRI_PARALLELISM_H
#define COLIBRI_PARALLELISM_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline int coli_parse_positive_int(const char *value, int *out) {
    char *end = NULL;
    errno = 0;
    long requested = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || requested <= 0 || requested > INT_MAX) {
        return 0;
    }
    *out = (int)requested;
    return 1;
}

static inline int coli_detect_thread_count(void) {
    const char *env = getenv("COLI_CPU_THREADS");
    if (!env || !*env) env = getenv("COLI_THREADS");
    if (!env || !*env) env = getenv("OMP_NUM_THREADS");
    if (env && *env) {
        int parsed = 0;
        if (coli_parse_positive_int(env, &parsed)) return parsed;
    }
    long procs = sysconf(_SC_NPROCESSORS_ONLN);
    if (procs < 0 || procs > INT_MAX) {
        fprintf(stderr, "warning: unable to detect CPU count; using 1 thread\n");
        return 1;
    }
    return (int)procs;
}

#endif
