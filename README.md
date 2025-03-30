# simple-lexer-c

A lightweight and easy-to-use lexer library written in C. This library—named **simple-lexer**—is designed to simplify the creation of lexers by providing a set of utility functions and a flexible API for iterating over tokens.

## Features

- **Simple API:** Easily define token types (e.g. punctuations, keywords) and iterate over them.
- **Customizable:** Supports custom token rules, single-line and multi-line comments.
- **Portable:** Written in standard C with minimal dependencies.
- **Extensible:** Designed to be adapted and extended for different projects.

## Files

- **simple-lexer.h**: The main header file containing the public API.
- **simple-lexer.c**: The implementation file (enable by defining `SIMPLELEXER_IMPLEMENTATION`).
- **example.c**: An example usage of the library demonstrating how to build and use the lexer.

## Getting Started

### Prerequisites

A C compiler (e.g. `gcc`, `clang`) and a standard POSIX environment (or any environment supporting standard C).

### Building the Example

1. Clone the repository:
   ```bash
   git clone https://github.com/BaseMax/simple-lexer-c.git
   cd simple-lexer-c
   ```

2. Compile the example using gcc:

    ```bash
    gcc -o simple_lexer_example example.c simple-lexer.c
    ```

3. Run the example:

    ```bash
    ./simple_lexer_example
    ```

4. Integrating Into Your Project

Simply include the header in your source files:

```c
#include "simple-lexer.h"
```

Then compile the implementation file (simple-lexer.c) along with your project. Customize token types, comment markers, and other settings as needed.

## Example

Below is a short excerpt of how to initialize and use the lexer:

```c
#include <string.h>

#include "simple-lexer.h"

int main(void) {
    const char *file_path = "example.input";
    const char *content =
        "#include <stdio.h>\n"
        "if (a == 180*2 + 50) { // single line comment\n"
        "    /* multi-line\n"
        "       comment */\n"
        "    return b;\n"
        "}\n";

    SimpleLexer lexer = simplelexer_create(file_path, content, strlen(content));
    SimpleLexer_Token token = {0};
    while (simplelexer_get_token(&lexer, &token)) {
        lexer.diagf(token.loc, "TOKEN", "%s: %.*s",
                    simplelexer_kind_name(SIMPLELEXER_KIND(token.id)),
                    (int)(token.end - token.begin), token.begin);
    }
    return 0;
}
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome! Feel free to fork this repository and submit pull requests. For major changes, please open an issue first to discuss what you would like to change.

