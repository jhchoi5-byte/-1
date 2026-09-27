#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sort.h"

#define TRIALS 30
#define BASE_SEED UINT64_C(2026092701)
#define DUPLICATE_RANGE 16

static const int inputSizes[] = {500, 1000, 2000, 5000, 10000, 20000};

typedef enum {
    DISTRIBUTION_RANDOM,
    DISTRIBUTION_SORTED,
    DISTRIBUTION_REVERSE,
    DISTRIBUTION_DUPLICATE_HEAVY,
    DISTRIBUTION_COUNT
} Distribution;

typedef enum {
    ALGORITHM_SHELL,
    ALGORITHM_MERGE,
    ALGORITHM_LIBRARY,
    ALGORITHM_COUNT
} Algorithm;

typedef void (*SortFunction)(int[], int);

static const char *distributionNames[] = {
    "random",
    "already_sorted",
    "reverse_sorted",
    "duplicate_heavy",
};

static const char *algorithmNames[] = {
    "shell",
    "merge",
    "library",
};

static const SortFunction sortFunctions[] = {
    shellSort,
    mergeSort,
    librarySort,
};

static const Algorithm executionOrders[6][ALGORITHM_COUNT] = {
    {ALGORITHM_SHELL, ALGORITHM_MERGE, ALGORITHM_LIBRARY},
    {ALGORITHM_SHELL, ALGORITHM_LIBRARY, ALGORITHM_MERGE},
    {ALGORITHM_MERGE, ALGORITHM_SHELL, ALGORITHM_LIBRARY},
    {ALGORITHM_MERGE, ALGORITHM_LIBRARY, ALGORITHM_SHELL},
    {ALGORITHM_LIBRARY, ALGORITHM_SHELL, ALGORITHM_MERGE},
    {ALGORITHM_LIBRARY, ALGORITHM_MERGE, ALGORITHM_SHELL},
};

static uint64_t nextRandom(uint64_t *state) {
    uint64_t value = (*state += UINT64_C(0x9e3779b97f4a7c15));
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static uint64_t mixSeed(uint64_t value) {
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static uint64_t seedForCondition(int inputSize, Distribution distribution, int trial) {
    uint64_t value = BASE_SEED;
    value ^= (uint64_t)(unsigned int)inputSize * UINT64_C(0x9e3779b185ebca87);
    value ^= (uint64_t)distribution * UINT64_C(0xc2b2ae3d27d4eb4f);
    value ^= (uint64_t)(unsigned int)trial * UINT64_C(0x165667b19e3779f9);
    return mixSeed(value);
}

static uint64_t randomBelow(uint64_t *state, uint64_t bound) {
    uint64_t threshold = (uint64_t)(-bound) % bound;
    for (;;) {
        uint64_t value = nextRandom(state);
        if (value >= threshold) {
            return value % bound;
        }
    }
}

static void generateInput(int values[], int n, Distribution distribution, uint64_t seed) {
    uint64_t state = seed;

    switch (distribution) {
    case DISTRIBUTION_RANDOM:
        for (int i = 0; i < n; i++) {
            values[i] = i;
        }
        for (int i = n - 1; i > 0; i--) {
            int j = (int)randomBelow(&state, (uint64_t)i + 1);
            int temporary = values[i];
            values[i] = values[j];
            values[j] = temporary;
        }
        break;
    case DISTRIBUTION_SORTED:
        for (int i = 0; i < n; i++) {
            values[i] = i;
        }
        break;
    case DISTRIBUTION_REVERSE:
        for (int i = 0; i < n; i++) {
            values[i] = n - 1 - i;
        }
        break;
    case DISTRIBUTION_DUPLICATE_HEAVY:
        for (int i = 0; i < n; i++) {
            values[i] = (int)randomBelow(&state, DUPLICATE_RANGE);
        }
        break;
    case DISTRIBUTION_COUNT:
        break;
    }
}

static int compareInts(const void *left, const void *right) {
    int leftValue = *(const int *)left;
    int rightValue = *(const int *)right;
    return (leftValue > rightValue) - (leftValue < rightValue);
}

static int isSorted(const int values[], int n) {
    for (int i = 1; i < n; i++) {
        if (values[i - 1] > values[i]) {
            return 0;
        }
    }
    return 1;
}

static int matchesExpected(const int values[], const int expected[], int n) {
    return memcmp(values, expected, (size_t)n * sizeof(*values)) == 0;
}

static uint64_t elapsedNanoseconds(const struct timespec *start,
        const struct timespec *end) {
    int64_t seconds = (int64_t)end->tv_sec - (int64_t)start->tv_sec;
    int64_t nanoseconds = (int64_t)end->tv_nsec - (int64_t)start->tv_nsec;
    return (uint64_t)(seconds * INT64_C(1000000000) + nanoseconds);
}

static int measureSort(SortFunction sort, int values[], int n, uint64_t *elapsed) {
    struct timespec start;
    struct timespec end;

    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        return 0;
    }
    sort(values, n);
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        return 0;
    }
    *elapsed = elapsedNanoseconds(&start, &end);
    return 1;
}

static int runBenchmark(void) {
    const size_t sizeCount = sizeof(inputSizes) / sizeof(inputSizes[0]);
    int maximumSize = inputSizes[sizeCount - 1];
    int *original = malloc((size_t)maximumSize * sizeof(*original));
    int *expected = malloc((size_t)maximumSize * sizeof(*expected));
    int *work = malloc((size_t)maximumSize * sizeof(*work));
    if (original == NULL || expected == NULL || work == NULL) {
        fprintf(stderr, "benchmark error: unable to allocate input buffers\n");
        free(original);
        free(expected);
        free(work);
        return 1;
    }

    puts("algorithm,input_size,distribution,trial,seed,elapsed_time_ns,correct");

    for (size_t sizeIndex = 0; sizeIndex < sizeCount; sizeIndex++) {
        int n = inputSizes[sizeIndex];
        for (int distributionIndex = 0;
                distributionIndex < DISTRIBUTION_COUNT; distributionIndex++) {
            Distribution distribution = (Distribution)distributionIndex;
            for (int trial = 1; trial <= TRIALS; trial++) {
                uint64_t seed = seedForCondition(n, distribution, trial);
                generateInput(original, n, distribution, seed);
                memcpy(expected, original, (size_t)n * sizeof(*expected));
                qsort(expected, (size_t)n, sizeof(*expected), compareInts);

                const Algorithm *order = executionOrders[(trial - 1) % 6];
                for (int orderIndex = 0; orderIndex < ALGORITHM_COUNT; orderIndex++) {
                    Algorithm algorithm = order[orderIndex];
                    SortFunction sort = sortFunctions[algorithm];
                    uint64_t elapsed;
                    memcpy(work, original, (size_t)n * sizeof(*work));
                    if (!measureSort(sort, work, n, &elapsed)) {
                        fprintf(stderr, "benchmark error: clock_gettime failed\n");
                        free(original);
                        free(expected);
                        free(work);
                        return 1;
                    }

                    int correct = matchesExpected(work, expected, n) && isSorted(work, n);
                    printf("%s,%d,%s,%d,%" PRIu64 ",%" PRIu64 ",%d\n",
                            algorithmNames[algorithm], n, distributionNames[distribution],
                            trial, seed, elapsed, correct);
                    if (!correct) {
                        fprintf(stderr,
                                "incorrect result: algorithm=%s input_size=%d distribution=%s trial=%d seed=%" PRIu64 "\n",
                                algorithmNames[algorithm], n,
                                distributionNames[distribution], trial, seed);
                        free(original);
                        free(expected);
                        free(work);
                        return 1;
                    }
                }
            }
        }
    }

    free(original);
    free(expected);
    free(work);
    return 0;
}

int main(void) {
    return runBenchmark();
}
