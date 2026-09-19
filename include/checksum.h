#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

uint32_t checksum_fnv1a_32(const uint8_t *data, size_t size);

#endif