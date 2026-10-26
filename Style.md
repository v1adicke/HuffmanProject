# C Code Style Guide

This document defines the official code style rules for the current project.

---

## 1. General Principles
- Code must be **clear, consistent, and readable**.
- Stick to **C11** standard (`-std=c11`).
- Use only standard headers from `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, etc.
- No hidden side effects, minimize global variables.
- Avoid clever or cryptic one-liners; clarity always wins.
- No additional dependencies (except unit tests framework)

---

## 2. Naming Conventions
| Element | Convention | Example |
|----------|-------------|----------|
| **Variables / Functions** | `lower_snake_case` | `int build_huffman_tree();` |
| **Types (`typedef struct`, `enum`)** | `PascalCase` | `typedef struct { ... } Node;` |
| **Constants / Macros** | `UPPER_SNAKE_CASE` | `#define MAX_SYMBOLS 256` |
| **Files** | `lower_snake_case.c/.h` | `bit_writer.c`, `bit_writer.h` |
| **Enum values** | `UPPER_SNAKE_CASE` with module prefix | `HUF_OK`, `HUF_ERR_IO` |

All names must be meaningful.  
Short loop indices like `i`, `j`, `k` are allowed.

---

## 3. Includes

Include order **must** follow these rules:

1. **Standard library** headers (with `< >`)
2. **Third-party** libraries (with `< >`)
3. **Local project** headers (with `" "`)
4. **Corresponding header** for the current source file (with `" "`)

Each group is separated by a blank line.  
After the last include, leave **two blank lines** before the start of code.

Example:

```c++
#include <stdio.h>
#include <stdlib.h>

#include <some_third_party.h>

#include "local_header.h"

#include "corresponding_header.h"


// Code starts here
````

---

## 4. File and Module Structure

* Each module has a header (`.h`) and a source file (`.c`).
* Each `.c` file must include its corresponding `.h`.
* Use **include guards**, not `#pragma once`:

  ```c++
  #ifndef BIT_WRITER_H
  #define BIT_WRITER_H
  // ...
  #endif // BIT_WRITER_H
  ```
* Prefer real includes over forward declarations.

---

## 5. Indentation and Formatting

* **Tab = 4 spaces** (spaces only, no actual tab characters).
* **Maximum line length: 120 characters.**
* **Braces on separate lines**:

  ```c++
  if (condition)
  {
      do_something();
  }
  else
  {
      recover();
  }
  ```
* **Empty line** after a closing brace `}` and before the next instruction,
  except before `else` or `while` in `do { } while`.
    ```c++
    if (condition)
    {
        do_something();
    }
    
    do_something2();    
    ```

---

## 6. Multi-line Function Formatting

For long function signatures or calls, use this format:

```c++
void some_long_function
(
      int first_arg
    , int second_arg
    , int third_arg
)
{
    // Code
}
```

and similarly for function calls:

```c++
result = some_long_function
(
      a
    , b
    , c
);
```

---

## 7. Spacing Rules

* Space after keywords: `if (`, `for (`, `while (`.
* Space around binary operators: `a + b`, `x && y`.
* Commas: space **after**, not **before**.
* Semicolons: no space before.
* Pointer star (`*`) **sticks to the type**, not the variable name:

  ```c++
  int* ptr;
  const char* message;
  ```

---

## 8. Control Flow Rules

* Always use braces `{}` even for single-line statements.

  ```c++
  if (a)
  {
      printf("%d\n", a);
  }
  ```
* `else` and `while` (in `do { } while`) must start on new lines.
    ```c++
    do 
    {
        something();   
    } 
    while (condition);
    ```
* **`goto` is forbidden.**
  Use structured control flow, early returns, or flag variables instead.

---

## 9. Constants and Macros

* **Allowed macros**:

    * `#define` for constants
    * `#define` for include guards
* **All other macros are forbidden** (no function-like macros, no tricky expressions).
* Example:

  ```c++
  #define BUFFER_SIZE 1024
  ```

---

## 10. Error Handling

* Functions that can fail must return an error code (`int` or `enum`)
  and pass results via output pointers:

  ```c++
  int read_file(const char* path, Data** out);
  ```
* Return `0` on success, non-zero on any error.
* `stderr` — for error messages
  `stdout` — for help messages and statistics only

---

## 11. Memory Management

* Every `malloc`/`calloc`/`realloc` must have a corresponding `free`.

---

## 12. Comments and Documentation

* All comments and docstrings must be **in English**.
* Use **Doxygen-style** documentation for all public functions, structs, and enums:

  ```c++
  /**
   * Builds a Huffman code table.
   * @param freq   Frequency table of 256 symbols.
   * @param out    [out] Allocated table pointer (must be freed by caller).
   * @return 0 on success, non-zero on error.
   */
  int build_codes(const uint32_t* freq, Code** out);
  ```
* Inline comments should explain **why**, not **what**.

---

## 13. Switch Statements

* Always include a `default` case.
* Use explicit `/* fallthrough */` comments when falling through:

  ```c++
  switch (state)
  {
      case INIT:
          init();
          /* fallthrough */
      case RUN:
          run();
          break;
      default:
          return ERR_STATE;
  }
  ```

---

## 14. Integer and Size Types

* Use fixed-width types (`uint8_t`, `int32_t`, `uint64_t`) for data storage.
* Use `size_t` for sizes and indices.
* Be careful with signed/unsigned comparisons.
