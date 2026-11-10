#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "status.h"


/**
 * Entry point: parses arguments and runs archiving or dearchiving.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on any error.
 */
int main(int argc, char** argv)
{
    CliOptions options;

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

    // TODO: archive / dearchive
    fprintf(stderr, "huffman: not implemented yet\n");
    return EXIT_FAILURE;
}
