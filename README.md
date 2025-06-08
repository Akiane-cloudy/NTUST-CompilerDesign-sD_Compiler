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



## Implementation Details (NEW)
### Assignment
In global assignment, I implement the assigment expression will initial expression statement.
```c
  int global_i = 10;
  int global_j = i + i; // Initial statement with expression
  void main(){/* ... */}
```

### Simple statement with return in a function
In a function, I implement the return statement in a single statement
```c
  int func(){
    if (!true)
      /* nothing */
    else 
      return 1; // Simple statement with return statement
  }
```