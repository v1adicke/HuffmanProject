#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stdio.h>

#include "status.h"

#define SYMBOL_COUNT 256


/**
 * Counts how many times each byte occurs in a file.
 * Reads the file from the current position to the end.
 * @param input       File opened for reading in binary mode.
 * @param frequencies [out] Array of SYMBOL_COUNT counters.
 * @param total_size  [out] Number of bytes read.
 * @return HUF_OK on success, HUF_ERR_READ if reading failed.
 */
HufStatus huffman_count_frequencies(FILE* input, uint64_t* frequencies, uint64_t* total_size);

#endif // HUFFMAN_H
