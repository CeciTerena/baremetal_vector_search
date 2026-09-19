#include "checksum.h"

#define FNV_OFFSET_BASIS_32 2166136261U
#define FNV_PRIME_32 16777619U

uint32_t checksum_fnv1a_32(const uint8_t *data, size_t size) {
    uint32_t hash = FNV_OFFSET_BASIS_32;
    for (size_t i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= FNV_PRIME_32;
    }
    return hash;
}