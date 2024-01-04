#pragma once

#include <stdlib.h>
#include <inttypes.h>


#define max(a, b) a > b ? a : b


typedef struct {
    void *memory;
    uint64_t taken;
} block;

typedef struct fm {
    void *memory;
    struct fm *next;
} forward_memory;


uint64_t closest_pow2(uint64_t n);
uint64_t closest_n_pow2(uint64_t n);
uint64_t pow2(uint64_t n);

uint64_t closest_pow2(uint64_t n) {
    n -= 1;
    uint64_t result = 1;
    for (; n != 0; n /= 2) result *= 2;
    return result;
}

uint64_t closest_n_pow2(uint64_t n) {
    if (!n) return 0;
    n -= 1;
    uint64_t result = 0;
    for (; n != 0; n /= 2) result += 1;
    return result;
}

uint64_t pow2(uint64_t n) {
    return 1 << n;
}