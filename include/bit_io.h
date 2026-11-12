#ifndef BIT_IO_H
#define BIT_IO_H

#include <stdint.h>
#include <stdio.h>

#include "status.h"


/**
 * Writes single bits to a file, the first bit goes to the highest bit of a byte.
 */
typedef struct
{
    FILE* file;             ///< output file
    uint8_t buffer;         ///< bits that are not written yet
    uint8_t bit_count;      ///< number of bits in the buffer
    uint64_t bytes_written; ///< number of bytes written to the file
} BitWriter;

/**
 * Reads single bits from a file in the same order as BitWriter writes them.
 */
typedef struct
{
    FILE* file;          ///< input file
    uint8_t buffer;      ///< last byte read from the file
    uint8_t bits_left;   ///< number of unread bits in the buffer
    uint64_t bytes_read; ///< number of bytes read from the file
} BitReader;

/**
 * Prepares a writer for a file.
 * @param writer [out] Writer to initialize.
 * @param file   File opened for writing in binary mode.
 */
void bit_writer_init(BitWriter* writer, FILE* file);

/**
 * Writes one bit.
 * @param writer Writer.
 * @param bit    Bit to write, 0 or 1.
 * @return HUF_OK on success, HUF_ERR_WRITE if writing failed.
 */
HufStatus bit_writer_write_bit(BitWriter* writer, uint8_t bit);

/**
 * Writes the last incomplete byte, the unused bits are filled with zeros.
 * @param writer Writer.
 * @return HUF_OK on success, HUF_ERR_WRITE if writing failed.
 */
HufStatus bit_writer_flush(BitWriter* writer);

/**
 * Prepares a reader for a file.
 * @param reader [out] Reader to initialize.
 * @param file   File opened for reading in binary mode.
 */
void bit_reader_init(BitReader* reader, FILE* file);

/**
 * Reads one bit.
 * @param reader Reader.
 * @param bit    [out] Read bit, 0 or 1.
 * @return HUF_OK on success, HUF_ERR_READ if reading failed, HUF_ERR_BAD_ARCHIVE if the file ended.
 */
HufStatus bit_reader_read_bit(BitReader* reader, uint8_t* bit);

#endif // BIT_IO_H
