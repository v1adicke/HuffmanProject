#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>
#include <stdio.h>

#include "status.h"

#define ARCHIVE_MAGIC "HUFF"
#define ARCHIVE_MAGIC_SIZE 4


/**
 * Sizes printed after archiving or dearchiving.
 */
typedef struct
{
    uint64_t original_size; ///< size of the not compressed data
    uint64_t data_size;     ///< size of the compressed data without the header
    uint64_t extra_size;    ///< size of the header
} ArchiveStats;

/**
 * Writes the archive header: magic, number of used symbols and the frequency of each used symbol.
 * @param output      File opened for writing in binary mode.
 * @param frequencies Array of SYMBOL_COUNT counters.
 * @param header_size [out] Number of bytes written.
 * @return HUF_OK on success, HUF_ERR_WRITE if writing failed.
 */
HufStatus format_write_header(FILE* output, const uint64_t* frequencies, uint64_t* header_size);

/**
 * Reads and checks the archive header written by format_write_header.
 * @param input       File opened for reading in binary mode.
 * @param frequencies [out] Array of SYMBOL_COUNT counters.
 * @param total_size  [out] Size of the original data, the sum of all frequencies.
 * @param header_size [out] Number of bytes read.
 * @return HUF_OK on success, HUF_ERR_BAD_ARCHIVE if the header is broken, HUF_ERR_READ if reading failed.
 */
HufStatus format_read_header(FILE* input, uint64_t* frequencies, uint64_t* total_size, uint64_t* header_size);

#endif // FORMAT_H
