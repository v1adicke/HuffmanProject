# Huffman archiver

## Description

Your task is to implement cli utility for archiving that uses a two-pass [Huffman](https://en.wikipedia.org/wiki/Huffman_coding) algorithm.

Although this algorithm is usually used for text compression, in this project it can be used for any kind of data.

## CLI application
- In the project you should create a cli application with the following keys:
  1. `-a` or `--archive` - archive file
  2. `-d` or `--dearchive` - dearchive file
  3. `-f` or `--file` - file to archive or dearchive
  4. `-o` or `--output` - output file name
  5. `-h` or `--help` - show help

- Your application should never fail. If any error occurs, it should print the error message to the standard error output and exit with a non-zero exit code.
In addition, in case of any error, your program should free all allocated memory.
- It's also your task to validate the correctness of the input arguments.
- The program should display compression/decompression statistics: the size of the source data, the size of the received 
data, and the size that was used to store auxiliary data in the output file (for example, tables).
No additional text should be displayed, only the dimensions. All sizes are in bytes.
    ```shell
    $ huffman -a -f myfile.txt -o result.bin
    15678
    6172
    482
    ```
    The size of the source file (source data): 15678 bytes, the size of the compressed data (without additional information): 6172 bytes, the size of the additional data: 482 bytes. The size of the entire compressed file is 6172 + 482 = 6654 bytes.
    ```shell
    $ ./huffman -d -f result.bin -o myfile_new.txt
    6172
    15678
    482
    ```
    The size of the decompressed file (received data): 15678 bytes, the size of the compressed data (without additional information): 6172 bytes, the size of the additional data: 482 bytes. The size of the entire compressed source file is 6172 + 482 = 6654 bytes.
- You can use any archive format you want, but it should follow the conditions:
  1. File before archiving and after archiving-dearchiving process should be identical (all bytes should be the same)
  2. The archived data (without additional information) should generally be smaller than the source data for non-uniform distributions, but you should not rely on this as a strict rule
  3. Your program should implement the huffman algorithm (so just copy-pasting the file is not a solution). 
- If both `-a` and `-d` flags are specified or none of them your program should exit with non-zero exit code and print an error message.
- Flags `-f` and `-o` are obligate, so if any of them is missing, your program should exit with non-zero exit code and print an error message.
- All error messages should be printed to the standard error output.
- File statistics as well as help message should be printed to the standard output.
## Project structure
- Unlike our usual home tasks, the project should be divided into several folders and files:
  1. `src` for all source files (except tests)
  2. `include` for all header files (except tests)
  3. `tests` for all tests files (if you are testing your code with C (not bash) in `tests` folder you also should have `include` and `src` folders)
- Your source code should be divided into a few source files (for example):
  1. `main.c` with a main function
  2. `archive.c` with functions for archiving
  3. `dearchive.c` with functions for dearchiving
  4. `huffman.c` with functions for Huffman algorithm
- The example above is not obligate, you can divide your code in any way you want, but each source file should be logically separated from the others.
- No binary, object or IDE-related files should be pushed to GitHub.
- Test assets should be placed in `tests/assets` folder (you can push them to GitHub)

## Code style and code patterns
- You should avoid using global variables, but in case you need to use them, you should explicitly comment (write in code) why you need them.
- You must describe all of your functions and structures (and any other user-defined data types) with a Doxygen-style docstring (see [Doxygen](https://www.doxygen.nl/index.html)).
- In case if a function can result in an error, the function should be written in C style (returning an error code and the actual result is returned via a mutable pointer).

## Testing
- You must provide autotests for your code.
  - Tests for each possible scenario of usage of your executable
  - Tests for memory leaks
  - Tests for correctness of the output
  - Tests for incorrect input (both files and flags)
- We haven't covered unit testing yet, but if you know what it is, you can implement them for additional points.
  - Library for unit tests (the name and the installation method) should be described in `Requirements.md` file. 
- All tests should be executed with `make test` command.

## Build
- Your program should be built with `make` command.
- Also, your Makefile should provide `install` target for installing your program.
    ```shell
    $ sudo make install
    $ huffman -h
    <YOUR HELP MESSAGE>  
    ```
- And `uninstall` target for removing your program.
    ```shell
    $ sudo make uninstall
    $ huffman -h
    command not found: huffman 
    ```
- Your Makefile should provide `clean` target to clean project dir from any build artifacts (calling `clean` shouldn't uninstall the program)
- Your Makefile should be designed in a way that if you change any of the source files, then only that file will be recompiled.

## Codestyle guidance
- In this project you must use the code style provided in [Style.md](./Style.md) file.

## Marking criteria
- Working project: 50 points 
- Project structure: 10 points 
- Code style: 10 points 
- Testing: 15 points (+10 points for unit testing, but not more than 100 points in total)
- Correct makefile: 15 points
- Total: 100 points
- If your project doesn't work correctly, you will get 0 points for it (even if all other criteria are met)

## Deadlines
- You have three weeks to complete the project.
- The deadline for submitting the project is 23:59 16.11.2026 (UTC+2)
- You also will have 2 soft deadlines: 23:59 02.11.2026 (UTC+2) and 23:59 09.11.2026 (UTC+2)
  - By submitting the project for the soft deadline allows you to get feedback from the mentor without affecting your grade.
- The final mark for your project will be published on 24.11.2026