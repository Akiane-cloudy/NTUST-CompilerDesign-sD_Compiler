# Compiler Design
## Project 2 -- Syntax Analysis ( Parser )

- Personal Info:
  - Name: YAN RU, CHEN（陳彥儒）
  - STUID: B11115004
  - Class: 四資工三乙班
  - Mail: B11115004@mail.ntust.edu.tw
  
- Development Environment:
  - OS: macOS Sequoia 15.3.1
  - IDE: Visual Studio code
  - Compiler:
    - flex 2.6.4 Apple(flex-35)
    - bison (GNU Bison) 3.8.2
    - g++ (std c++17)
  
- Project Structure:
```
/Project
  |--- Makefile
  |--- README.md
  |--- /src
  |     |--- scanner.l
  |     |--- parser.y
  |     |--- SemanticAnalyzer.cpp
  |     |--- SymbolTable.cpp
  |     
  |--- /include
  |     |--- SymbolTable.hpp
  |     |--- SemanticAnalyzer.hpp
  |     |--- AST.hpp
  |     
  |--- /example (some cases for testing)
  |    
  |--- /build
        |--- (Generated Object Files by Makefile)
```

- How to Compile:
  - Open Terminal
  - Change Directory to Project Directory
  - Type `make` and Press Enter
  - The Executable File will be Generated in Project Directory

- How to Run:
  - Open a terminal.
  - Navigate to the project directory.
  - Execute the command `./parser <SOURCE_FILE>`, where `<SOURCE_FILE>` is a `.sd` file.
  - The output will be displayed in the terminal, and scanned tokens will be saved to `token.txt`.
  - If the grammar is correct and no semantic conflicts are detected, the message `"Parsing completed successfully!"` be shown. Otherwise, error or warning messages will be displayed.
  - After parsing every scope, the symbol table of identifiers will be dumped.
    
- How to Clean:
  - Use `make clean` to Clean the Generated Files



## Implementation Details (NEW)
```
In this section, I will introduce some implementation details of my project. Some aspects may not be explicitly defined in the assignment description, so I have implemented them based on my understanding of C++. Additionally, I have included some extra features that go beyond the original requirements.
```
---
### Types

- **`char`**  
  - Can get the token from the scanner as `CHAR_CONSTANT`, e.g., `'c'`.

- **`double`**  
  - Can get the token from the scanner as `REAL_CONSTANT`, e.g., `4.0e10`.

---

### Binary Operation Expression Statement

- Supported operators: `+`, `-`, `*`, `/`, `%`, `<`, `<=`, `>=`, `>`, `==`, `!=`, `&&` and `||`
- If `lhs` and `rhs` are of **different types**, the parser will issue a **error** and perform an error message.

- For `+`, `<`, `<=`, `>=`, `>`, `==` and  `!=`, accept type:
  - `int`, `char`, `float`, `double`, `bool`, `string`

- For `-`, `*` and `/`,  accept type:
  - `int`, `char`, `float`, `double`, `bool`

- For `%`:
  - Both `lhs` and `rhs` must be of type `int`; otherwise, the parser will raise an **error** and stop compilation.

    INT % INT -> ✅
    INT % FLOAT -> ❌

- For `&&` and `||`:
  - Both `lhs` and `rhs` must be of type `bool`; otherwise, the parser will raise an **error** and stop compilation.

    BOOL && BOOL -> ✅
    INT || FLOAT -> ❌

---

### Array

- Two arrays are considered **the same type** only if they have the same base type, number of dimensions, and size in each dimension.

Example:
```c
    int func(int arr[2][3][4]) { /* ... */ }

    void main() {
      int arr[1][2][3];
      func(arr); // ERROR: Array types do not match.
    }
```

- Array access expressions **will not** perform boundary checks at compile time, since the exact index values are not known during compilation.
```c
    int arr[10];
    arr[10]; // Parsing succeeds without error.
```
---

### Function

- Functions can return any type, including `void`.
- A `void` function **cannot** have a `return` statement with a value.
- The return type of a function must match the declared type.

Examples:
```c
    int func() {
      return 1; // ✅
    }

    void func2() {
      return 1; // ❌ ERROR: void function cannot return a value.
    }

    double func3() {
      return 1; // ❌ ERROR: return type mismatch.
    }
```
- Every function must have at least **one `return` statement on all possible paths**. Otherwise, the parser will raise an error.
```c
    int func(bool a) {
      if (a) return 1;
      // Missing return here
    }
    // ❌ ERROR: missing return statement on some code paths.
```
- The `main` function **must** be declared as `void main()`.
  - If `main` is missing, the parser will raise an error.

---

### Statement

- **Empty statement**:
```c
    void main() {
      ; // ✅ Parsing succeeds.
    }
```
- **Block statement**:
```c
    void main() {
      int a;
      {
        int a; // ✅ Parsing succeeds.
      }
    }
```
- In a `for` loop, the initializer block is treated as part of the loop body’s block (same scope).

Example:
```c
    for (int i = 0; i < 10; i++) {
      int i; // ❌ ERROR: redefinition of 'i'
    }
```
- After processing a scope, the parser will **dump the symbol table of the current scope**.

Example:
```c
    void main() {
      int a;
      {
        int a;
      }
    }
    /*
    ============ Dump Start ============
    1. "a", int
    ============ Dump End ==============

    ============ Dump Start ============
    1. "a", int
    ============ Dump End ==============
    */
```
---
### Assigment
- Chain assigment
```c
  int a;
  int b = (a = 3); // ✅ Parsing succeeds.
```

- Const declaration with expression
```c
  int a, b;
  const int c =  a + b; // ✅ Parsing succeeds.
```