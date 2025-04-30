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
  |--- /testcase (some cases for testing)
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
  - After parsing `"{}"`, the symbol table of identifiers will be dumped.
    
- How to Clean:
  - Use `make clean` to Clean the Generated Files