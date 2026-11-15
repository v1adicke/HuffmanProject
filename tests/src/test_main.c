#include <stdio.h>
#include <stdlib.h>

#include "test_suites.h"


/**
 * Runs all unit test suites.
 * @return EXIT_SUCCESS if all tests passed, EXIT_FAILURE otherwise.
 */
int main(void)
{
    int failed = 0;

    failed += run_huffman_tests();
    failed += run_bit_io_tests();
    failed += run_format_tests();
    failed += run_cli_tests();

    if (failed != 0)
    {
        fprintf(stderr, "%d unit tests failed\n", failed);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
