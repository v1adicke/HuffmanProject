#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

#include "cli.h"


/**
 * Checks whether an argument is the short or the long form of an option.
 * @param arg        Command line argument.
 * @param short_name Short form, e.g. "-a".
 * @param long_name  Long form, e.g. "--archive".
 * @return true if the argument matches one of the forms.
 */
static bool is_option(const char* arg, const char* short_name, const char* long_name)
{
    return strcmp(arg, short_name) == 0 || strcmp(arg, long_name) == 0;
}

/**
 * Sets a flag option, rejecting repeated flags.
 * @param flag   [in,out] Flag to set.
 * @param option Option as written by the user, used in the error message.
 * @return HUF_OK on success, HUF_ERR_ARGS if the flag is already set.
 */
static HufStatus set_flag(bool* flag, const char* option)
{
    if (*flag)
    {
        fprintf(stderr, "huffman: option '%s' is given more than once\n", option);
        return HUF_ERR_ARGS;
    }

    *flag = true;
    return HUF_OK;
}

/**
 * Reads the file name that follows an option like -f or -o.
 * @param argc  Argument count.
 * @param argv  Argument vector.
 * @param index [in,out] Index of the option, moved to its value on success.
 * @param out   [in,out] Where to store the file name, must be NULL if the option was not seen yet.
 * @return HUF_OK on success, HUF_ERR_ARGS if the value is missing or the option is repeated.
 */
static HufStatus read_path(int argc, char** argv, int* index, const char** out)
{
    const char* option = argv[*index];

    if (*out != NULL)
    {
        fprintf(stderr, "huffman: option '%s' is given more than once\n", option);
        return HUF_ERR_ARGS;
    }

    if (*index + 1 >= argc || argv[*index + 1][0] == '\0')
    {
        fprintf(stderr, "huffman: option '%s' requires a file name\n", option);
        return HUF_ERR_ARGS;
    }

    *index += 1;
    *out = argv[*index];
    return HUF_OK;
}

/**
 * Checks that the parsed options describe a valid run.
 * @param options Parsed options.
 * @return HUF_OK if options are consistent, HUF_ERR_ARGS otherwise.
 */
static HufStatus validate(const CliOptions* options)
{
    if (options->archive == options->dearchive)
    {
        fprintf(stderr, "huffman: exactly one of -a and -d must be given\n");
        return HUF_ERR_ARGS;
    }

    if (options->input_path == NULL)
    {
        fprintf(stderr, "huffman: input file is not given (-f)\n");
        return HUF_ERR_ARGS;
    }

    if (options->output_path == NULL)
    {
        fprintf(stderr, "huffman: output file is not given (-o)\n");
        return HUF_ERR_ARGS;
    }

    // Opening the output would truncate the input before it is read.
    if (strcmp(options->input_path, options->output_path) == 0)
    {
        fprintf(stderr, "huffman: input and output must be different files\n");
        return HUF_ERR_ARGS;
    }

    return HUF_OK;
}

HufStatus cli_parse(int argc, char** argv, CliOptions* out)
{
    CliOptions options = { false, false, false, NULL, NULL };
    HufStatus status = HUF_OK;

    for (int i = 1; i < argc && status == HUF_OK; i++)
    {
        const char* arg = argv[i];

        if (is_option(arg, "-a", "--archive"))
        {
            status = set_flag(&options.archive, arg);
        }
        else if (is_option(arg, "-d", "--dearchive"))
        {
            status = set_flag(&options.dearchive, arg);
        }
        else if (is_option(arg, "-h", "--help"))
        {
            status = set_flag(&options.help, arg);
        }
        else if (is_option(arg, "-f", "--file"))
        {
            status = read_path(argc, argv, &i, &options.input_path);
        }
        else if (is_option(arg, "-o", "--output"))
        {
            status = read_path(argc, argv, &i, &options.output_path);
        }
        else
        {
            fprintf(stderr, "huffman: unknown argument '%s'\n", arg);
            status = HUF_ERR_ARGS;
        }
    }

    if (status == HUF_OK && !options.help)
    {
        status = validate(&options);
    }

    if (status == HUF_OK)
    {
        *out = options;
    }

    return status;
}

void cli_print_help(void)
{
    printf
    (
        "Usage: huffman (-a | -d) -f <input> -o <output>\n"
        "\n"
        "Compress or decompress a file with Huffman coding.\n"
        "\n"
        "Options:\n"
        "  -a, --archive         compress the input file\n"
        "  -d, --dearchive       decompress the input file\n"
        "  -f, --file <path>     input file\n"
        "  -o, --output <path>   output file\n"
        "  -h, --help            show this message\n"
        "\n"
        "After success three sizes in bytes are printed:\n"
        "  -a: source size, compressed data size, extra data size\n"
        "  -d: compressed data size, result size, extra data size\n"
    );
}
