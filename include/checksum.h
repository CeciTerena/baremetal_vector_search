#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

#define FNV_OFFSET_BASIS_32 2166136261U
#define FNV_PRIME_32 16777619U

uint32_t checksum_fnv1a_32(const uint8_t *data, size_t size);

#endif