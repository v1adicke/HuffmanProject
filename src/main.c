#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "archive.h"
#include "cli.h"
#include "dearchive.h"
#include "format.h"
#include "status.h"


/**
 * Prints three sizes to stdout, one per line.
 * @param first  First size.
 * @param second Second size.
 * @param third  Third size.
 */
static void print_sizes(uint64_t first, uint64_t second, uint64_t third)
{
    printf("%" PRIu64 "\n%" PRIu64 "\n%" PRIu64 "\n", first, second, third);
}

/**
 * Entry point: parses arguments and runs archiving or dearchiving.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on any error.
 */
int main(int argc, char** argv)
{
    CliOptions options;
    ArchiveStats stats;
    HufStatus status = HUF_OK;

    if (cli_parse(argc, argv, &options) != HUF_OK)
    {
        fprintf(stderr, "Try 'huffman --help' for more information.\n");
        return EXIT_FAILURE;
    }

    if (options.help)
    {
        cli_print_help();
        return EXIT_SUCCESS;
    }

    if (options.archive)
    {
        status = archive_file(options.input_path, options.output_path, &stats);
    }
    else
    {
        status = dearchive_file(options.input_path, options.output_path, &stats);
    }

    if (status != HUF_OK)
    {
        fprintf(stderr, "huffman: %s\n", status_message(status));
        return EXIT_FAILURE;
    }

    if (options.archive)
    {
        print_sizes(stats.original_size, stats.data_size, stats.extra_size);
    }
    else
    {
        print_sizes(stats.data_size, stats.original_size, stats.extra_size);
    }

    return EXIT_SUCCESS;
}
