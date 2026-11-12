#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "format.h"
#include "status.h"


/**
 * Compresses a file with Huffman coding.
 * If something goes wrong, the output file is removed.
 * @param input_path  Path to the file to compress.
 * @param output_path Path to the archive to create.
 * @param stats       [out] Sizes of the source data, compressed data and header.
 * @return HUF_OK on success, error code otherwise.
 */
HufStatus archive_file(const char* input_path, const char* output_path, ArchiveStats* stats);

#endif // ARCHIVE_H
