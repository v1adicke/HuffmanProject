# Requirements

## Build
- `gcc` with C11 support
- `make`

The program itself has no other dependencies.

## Tests
`make test` runs the unit tests and then the bash tests from `tests/run_tests.sh`.

- [cmocka](https://cmocka.org/) — library for unit tests, tested with version 2.0.2
- [valgrind](https://valgrind.org/) — used to check memory leaks
- `bash`, `cmp`, `stat` — used by the bash tests

### Installation
Arch Linux:
```shell
sudo pacman -S cmocka valgrind
```

Ubuntu / Debian:
```shell
sudo apt install libcmocka-dev valgrind
```

Fedora:
```shell
sudo dnf install libcmocka-devel valgrind
```

## Usage
```shell
make
make test
sudo make install
huffman -a -f file.txt -o file.huf
huffman -d -f file.huf -o file_new.txt
sudo make uninstall
make clean
```
