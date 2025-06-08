# Compiler Design
## sD Compiler

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
  |     |--- CodeGenVisitor.cpp
  |     
  |--- /include
  |     |--- SymbolTable.hpp
  |     |--- SemanticAnalyzer.hpp
  |     |--- AST.hpp
  |     |--- CodeEmitter.hpp
  |     |--- CodeGenContext.hpp
  |     |--- CodeGenVisitor.hpp
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
  - The output is `<SOURCE_FILE>.jasm`, and scanned tokens will be saved to `token.txt`.
  - If the grammar is correct and no semantic conflicts are detected, the message `"Parsing completed successfully!"` be shown. Otherwise, error or warning messages will be displayed.
  - After parsing, use `javaa <SOURCE_FILE>.jasm` to generate the `.class` file
  - Use `java <SOURCE_FILE_NAME>` to run the result on the java runtime.
    
- How to Clean:
  - Use `make clean` to Clean the Generated Files