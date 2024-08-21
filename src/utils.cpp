#include <bls12-381/bls12-381.hpp>

uint64_t countLeadingZeros(uint64_t x) {
    if (x == 0) return 64;

    uint64_t n = 0;
    if ((x >> 32) == 0) {
        n += 32;
        x <<= 32;
    }
    if ((x >> 48) == 0) {
        n += 16;
        x <<= 16;
    }
    if ((x >> 56) == 0) {
        n += 8;
        x <<= 8;
    }
    if ((x >> 60) == 0) {
        n += 4;
        x <<= 4;
    }
    if ((x >> 62) == 0) {
        n += 2;
        x <<= 2;
    }
    if ((x >> 63) == 0) { n += 1; }
    return n;
}