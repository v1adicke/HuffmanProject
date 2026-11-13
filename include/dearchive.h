#ifndef DEARCHIVE_H
#define DEARCHIVE_H

#include "format.h"
#include "status.h"


/**
 * Restores a file from an archive made by archive_file.
 * If something goes wrong, the output file is removed.
 * @param input_path  Path to the archive.
 * @param output_path Path to the file to create.
 * @param stats       [out] Sizes of the restored data, compressed data and header.
 * @return HUF_OK on success, error code otherwise.
 */
HufStatus dearchive_file(const char* input_path, const char* output_path, ArchiveStats* stats);

#endif // DEARCHIVE_H
