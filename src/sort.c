#include "sort.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int value;
    int occupied;
} LibrarySlot;

static size_t libraryInsertionTarget(const LibrarySlot slots[], size_t capacity, int value) {
    size_t low = 0;
    size_t high = capacity;

    while (low < high) {
        size_t middle = low + (high - low) / 2;
        if (slots[middle].occupied) {
            if (value < slots[middle].value) {
                high = middle;
            } else {
                low = middle + 1;
            }
            continue;
        }

        size_t left = middle;
        while (left > 0 && !slots[left - 1].occupied) {
            left--;
        }
        size_t right = middle;
        while (right < capacity && !slots[right].occupied) {
            right++;
        }

        if (left > 0 && right < capacity) {
            if (value < slots[left - 1].value) {
                high = left - 1;
            } else if (value < slots[right].value) {
                return middle;
            } else {
                low = right + 1;
            }
        } else if (left > 0) {
            if (value < slots[left - 1].value) {
                high = left - 1;
            } else {
                low = middle + 1;
            }
        } else if (right < capacity) {
            if (value < slots[right].value) {
                high = middle;
            } else {
                low = right + 1;
            }
        } else {
            return middle;
        }
    }

    return low;
}

static size_t libraryInsertionTargetOrCenter(const LibrarySlot slots[], size_t capacity,
        size_t count, int value) {
    if (count == 0) {
        return capacity / 2;
    }
    return libraryInsertionTarget(slots, capacity, value);
}

static size_t nearestEmptySlot(const LibrarySlot slots[], size_t capacity,
        size_t target, size_t *distance) {
    for (size_t offset = 0; offset < capacity; offset++) {
        if (target >= offset && target - offset < capacity
                && !slots[target - offset].occupied) {
            *distance = offset;
            return target - offset;
        }
        if (target < capacity && offset > 0
                && offset <= capacity - 1 - target
                && !slots[target + offset].occupied) {
            *distance = offset;
            return target + offset;
        }
    }
    *distance = capacity;
    return capacity;
}

static void rebalanceLibrarySlots(LibrarySlot slots[], size_t capacity,
        size_t count, int values[]) {
    size_t next = 0;
    for (size_t i = 0; i < capacity; i++) {
        if (slots[i].occupied) {
            values[next++] = slots[i].value;
            slots[i].occupied = 0;
        }
    }

    size_t denominator = count + 1;
    size_t step = capacity / denominator;
    size_t remainder = capacity % denominator;
    size_t position = 0;
    size_t error = 0;
    for (size_t i = 0; i < count; i++) {
        position += step;
        error += remainder;
        if (error >= denominator) {
            position++;
            error -= denominator;
        }
        slots[position].value = values[i];
        slots[position].occupied = 1;
    }
}

static int insertLibraryValue(LibrarySlot slots[], size_t capacity,
        size_t count, int value, int values[]) {
    size_t target = libraryInsertionTargetOrCenter(slots, capacity, count, value);
    size_t distance;
    size_t empty = nearestEmptySlot(slots, capacity, target, &distance);
    if (empty == capacity) {
        return 0;
    }

    if (distance > 2) {
        rebalanceLibrarySlots(slots, capacity, count, values);
        target = libraryInsertionTargetOrCenter(slots, capacity, count, value);
        empty = nearestEmptySlot(slots, capacity, target, &distance);
        if (empty == capacity) {
            return 0;
        }
    }

    if (empty == target) {
        slots[target].value = value;
        slots[target].occupied = 1;
    } else if (empty < target) {
        for (size_t i = empty; i + 1 < target; i++) {
            slots[i] = slots[i + 1];
        }
        slots[target - 1].value = value;
        slots[target - 1].occupied = 1;
    } else {
        for (size_t i = empty; i > target; i--) {
            slots[i] = slots[i - 1];
        }
        slots[target].value = value;
        slots[target].occupied = 1;
    }
    return 1;
}

static void merge(int a[], int temp[], int left, int mid, int right) {
    int i = left;
    int j = mid;
    int k = left;

    while (i < mid && j < right) {
        if (a[i] <= a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
    }
    while (i < mid) {
        temp[k++] = a[i++];
    }
    while (j < right) {
        temp[k++] = a[j++];
    }
    for (i = left; i < right; i++) {
        a[i] = temp[i];
    }
}

static void mergeSortRange(int a[], int temp[], int left, int right) {
    if (right - left < 2) {
        return;
    }

    int mid = left + (right - left) / 2;
    mergeSortRange(a, temp, left, mid);
    mergeSortRange(a, temp, mid, right);
    merge(a, temp, left, mid, right);
}

void bubbleSort(int a[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        /* 한 번 훑을 때마다 가장 큰 값이 뒤로 밀려 자리를 잡는다. */
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
                swapped = 1;
            }
        }
        /* 한 바퀴 동안 교환이 없었다면 이미 정렬된 것이다. */
        if (!swapped) {
            return;
        }
    }
}

void mergeSort(int a[], int n) {
    if (n < 2) {
        return;
    }

    int *temp = malloc((size_t)n * sizeof(*temp));
    if (temp == NULL) {
        return;
    }
    mergeSortRange(a, temp, 0, n);
    free(temp);
}

void shellSort(int a[], int n) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int value = a[i];
            int j = i;
            while (j >= gap && a[j - gap] > value) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = value;
        }
    }
}

void librarySort(int a[], int n) {
    if (n < 2) {
        return;
    }

    size_t length = (size_t)n;
    if (length > (SIZE_MAX - 1) / 2) {
        return;
    }
    size_t capacity = 2 * length + 1;
    if (capacity > SIZE_MAX / sizeof(LibrarySlot)
            || length > SIZE_MAX / sizeof(int)) {
        return;
    }

    LibrarySlot *slots = calloc(capacity, sizeof(*slots));
    int *values = malloc(length * sizeof(*values));
    if (slots == NULL || values == NULL) {
        free(slots);
        free(values);
        return;
    }

    size_t count = 0;
    size_t processed = 0;
    size_t batch = 1;
    while (processed < length) {
        size_t remaining = length - processed;
        size_t end = processed + (batch < remaining ? batch : remaining);
        while (processed < end) {
            if (!insertLibraryValue(slots, capacity, count, a[processed], values)) {
                free(slots);
                free(values);
                return;
            }
            count++;
            processed++;
        }
        rebalanceLibrarySlots(slots, capacity, count, values);
        if (batch <= length / 2) {
            batch *= 2;
        } else {
            batch = length;
        }
    }

    size_t output = 0;
    for (size_t i = 0; i < capacity; i++) {
        if (slots[i].occupied) {
            a[output++] = slots[i].value;
        }
    }
    free(slots);
    free(values);
}
