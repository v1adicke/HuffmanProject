#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

#include "status.h"


/**
 * Parsed command line options.
 */
typedef struct
{
    bool archive;            ///< -a / --archive was given
    bool dearchive;          ///< -d / --dearchive was given
    bool help;               ///< -h / --help was given
    const char* input_path;  ///< value of -f / --file, NULL if missing
    const char* output_path; ///< value of -o / --output, NULL if missing
} CliOptions;

/**
 * Parses and validates command line arguments.
 * Prints a description of the problem to stderr if arguments are invalid.
 * @param argc Argument count from main.
 * @param argv Argument vector from main.
 * @param out  [out] Parsed options, filled only on success.
 * @return HUF_OK on success, HUF_ERR_ARGS if arguments are invalid.
 */
HufStatus cli_parse(int argc, char** argv, CliOptions* out);

/**
 * Prints the help message to stdout.
 */
void cli_print_help(void);

#endif // CLI_H
