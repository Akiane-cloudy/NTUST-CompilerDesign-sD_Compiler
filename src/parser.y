%code requires {
    #include <memory>
    #include <vector>

    namespace ast {
        struct Type;
        struct Program;
        struct Stmt;
        struct Expr;
        struct Var;
        struct Decl;
        struct DeclList;
        struct VarDecl;
        struct Print;
        struct Println;
        struct Read;
        struct ConstDecl;
        struct FuncDecl;
        struct VarDeclList;
        struct Block;
        struct EmptyStmt;
        using StmtList = std::vector<std::unique_ptr<Stmt>>;
        using ExprList = std::vector<std::unique_ptr<Expr>>;
    }
}

%require "3.0"
%locations

%{
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../include/SemanticAnalyzer.hpp"
#include "../include/CodeGenVisitor.hpp"
using namespace std;

extern int yylex();
extern FILE *yyin;

void yyerror(std::string s); 
void yywarning(std::string s);
ast::Program* parse();

ast::Program* root = nullptr;
std::ofstream outStream;
%}

%union {
    int            ival;
    double         dval;
    std::string*   sval;
    bool           bval;
    char           cval;

    ast::Program*  prog;
    ast::Stmt*     stmt;
    ast::Expr*     expr;
    ast::StmtList* stmt_list;
    ast::ExprList* expr_list;
    ast::Type*     type;
    ast::Var*      var;
    ast::Decl*      decl;
    ast::DeclList*  decl_list;
    ast::VarDecl*   var_decl;
    ast::ConstDecl* const_decl;
    ast::FuncDecl*  func_decl;
    ast::VarDeclList* var_decl_list;
    ast::Block*       block;
    ast::Print*       print;
    ast::Println*     println;
    ast::Read*        read;
    ast::EmptyStmt*   empty_stmt;
}

%token BAD_CHARACTER

/*Define delimiters*/
%token DOT 
%token COMMA
%token COLON 
%token SEMICOLON
%token LEFT_PARENTHESIS
%token RIGHT_PARENTHESIS
%token LEFT_SQUARE_BRACKET
%token RIGHT_SQUARE_BRACKET
%token LEFT_CURLY_BRACKET
%token RIGHT_CURLY_BRACKET

/*Define keywords*/
%token BOOLEAN
%token BREAK
%token CASE
%token CHAR
%token CONST
%token CONTINUE
%token DEFAULT
%token DO
%token DOUBLE
%token ELSE
%token EXTERN
%token FLOAT
%token FOR
%token FOREACH
%token IF
%token INT
%token PRINT
%token PRINTLN
%token READ
%token RETURN
%token STRING
%token SWITCH
%token VOID
%token WHILE

/*Define identifiers*/
%token <sval> IDENTIFIER
%token <ival> INTEGER_CONSTANT 
%token <dval> REAL_CONSTANT 
%token <sval> STRING_CONSTANT
%token <bval> TRUE_CONSTANT FALSE_CONSTANT
%token <cval> CHAR_CONSTANT

/*Declare type of nodes*/
//========Program unit=========
%type <prog> program
//========Program unit=========

//========Statement unit=========
%type <stmt> statement
%type <stmt_list> main
%type <stmt_list> statement_list
%type <block> block
//========Statement unit=========

//========Expression unit=========
%type <expr> expression
%type <expr_list> call_argument_list
%type <expr_list> index_list
//========Expression unit=========

//========Type unit=========
%type <type> type
//========Type unit=========

//========Variable unit=========
%type <var> lvalue
// =======Variable unit=========

//========Declaration unit=========
%type <decl> declaration
%type <decl_list> global_declaration
%type <var_decl_list> argument_list
%type <var_decl> dim_list
%type <func_decl> function_declaration
%type <var_decl_list> init_declarator_list
%type <var_decl> init_declarator
%type <var_decl_list> const_init_list
%type <const_decl> const_init_declarator
//========Declaration unit=========

/*Declare end*/

/*Precedence*/
%left       COMMA
%right      ASSIGNMENT
%left       OR
%left       AND
%right      NOT
%left       LESS_THAN LESS_THAN_OR_EQUAL EQUAL GREATER_THAN_OR_EQUAL GREATER_THAN NOT_EQUAL
%left       ADDITION SUBTRACTION
%left       MULTIPLICATION DIVISION MODULUS
%left       DOUBLE_ADDITION DOUBLE_SUBTRACTION
%right      UMINUS
%nonassoc   LOWER_THAN_ELSE
%right      ELSE

%%
program:
      global_declaration main {
        $$ = new ast::Program(std::move($1->decls), std::move(*$2), @$.first_line);
        root = $$;
    }
    | main {
        auto emptyDecls = new ast::DeclList();
        $$ = new ast::Program(std::move(emptyDecls->decls), std::move(*$1), @$.first_line);
        root = $$;
        delete emptyDecls;
    }
    | BAD_CHARACTER {
          yyerror("Syntax error. Unknown token!");
    }
    ;

global_declaration:
      global_declaration declaration SEMICOLON {
        $1->decls.push_back(std::unique_ptr<ast::Decl>($2));
        $$ = $1;
      }
    | declaration SEMICOLON {
        auto tmp = new ast::DeclList();
        tmp->decls.push_back(std::unique_ptr<ast::Decl>($1));
        $$ = tmp;
      }
    | global_declaration function_declaration {
        $1->decls.push_back(std::unique_ptr<ast::FuncDecl>($2));
        $$ = $1;
      }
    | function_declaration {
        auto tmp = new ast::DeclList();
        tmp->decls.push_back(std::unique_ptr<ast::FuncDecl>($1));
        $$ = tmp;
      }
    ;

main: 
    function_declaration {
        if ($1->name != "main") {
            yywarning("Main function not found!");
        }
        auto tmp = new std::vector<std::unique_ptr<ast::Stmt>>();
        tmp->push_back(std::unique_ptr<ast::Stmt>($1));
        $$ = tmp;
    }
    ;

statement_list:
      statement_list statement{
        $1->push_back(std::unique_ptr<ast::Stmt>($2));
        $$ = $1;
      }
    | statement{
        auto tmp = new std::vector<std::unique_ptr<ast::Stmt>>();
        tmp->push_back(std::unique_ptr<ast::Stmt>($1));
        $$ = tmp;
      }
    ;

block:
      LEFT_CURLY_BRACKET statement_list RIGHT_CURLY_BRACKET{
        // Create a new Block with moved statement list
        auto blk = new ast::Block({}, @$.first_line);
        blk->stmts = std::move(*$2);
        $$ = blk;
        delete $2;
      }
    | LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET{
        $$ = new ast::Block({}, @$.first_line);
      }
    ;

statement:
      expression SEMICOLON{ $$ = new ast::ExprStmt(std::unique_ptr<ast::Expr>($1), @$.first_line); }
    | declaration SEMICOLON{ $$ = $1; }
    | block{ $$ = $1; }
    | /* Empty statement */ SEMICOLON { $$ = new ast::EmptyStmt(@$.first_line); }
    | PRINT expression SEMICOLON{ $$ = new ast::Print(std::unique_ptr<ast::Expr>($2), @$.first_line); }
    | PRINTLN expression SEMICOLON{ $$ = new ast::Println(std::unique_ptr<ast::Expr>($2), @$.first_line); }
    | READ lvalue SEMICOLON{ $$ = new ast::Read(std::unique_ptr<ast::Var>($2), @$.first_line); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS statement %prec LOWER_THAN_ELSE { 
        $$ = new ast::IfStmt(std::unique_ptr<ast::Expr>($3),
                             std::unique_ptr<ast::Stmt>($5),
                             nullptr,
                             @1.first_line); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS statement ELSE statement { 
        $$ = new ast::IfStmt(std::unique_ptr<ast::Expr>($3),
                             std::unique_ptr<ast::Stmt>($5), 
                             std::unique_ptr<ast::Stmt>($7),
                             @1.first_line); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS statement {
        $$ = new ast::WhileStmt(std::unique_ptr<ast::Expr>($3), 
                                std::unique_ptr<ast::Stmt>($5), 
                                @1.first_line); }
    | FOR LEFT_PARENTHESIS expression SEMICOLON expression SEMICOLON expression RIGHT_PARENTHESIS statement {
        $$ = new ast::ForStmt(
            std::unique_ptr<ast::Stmt>(new ast::ExprStmt(std::unique_ptr<ast::Expr>($3), @1.first_line)),
            std::unique_ptr<ast::Expr>($5),
            std::unique_ptr<ast::Stmt>(new ast::ExprStmt(std::unique_ptr<ast::Expr>($7), @1.first_line)),
            std::unique_ptr<ast::Stmt>($9),
            @1.first_line
        );
      }
    | FOR LEFT_PARENTHESIS declaration SEMICOLON expression SEMICOLON expression RIGHT_PARENTHESIS statement {
        $$ = new ast::ForStmt(
            std::unique_ptr<ast::Stmt>($3),
            std::unique_ptr<ast::Expr>($5),
            std::unique_ptr<ast::Stmt>(new ast::ExprStmt(std::unique_ptr<ast::Expr>($7), @1.first_line)),
            std::unique_ptr<ast::Stmt>($9),
            @1.first_line
        );
      }
    | FOR LEFT_PARENTHESIS expression SEMICOLON expression SEMICOLON declaration RIGHT_PARENTHESIS statement {
        $$ = new ast::ForStmt(
            std::unique_ptr<ast::Stmt>(new ast::ExprStmt(std::unique_ptr<ast::Expr>($3), @1.first_line)),
            std::unique_ptr<ast::Expr>($5),
            std::unique_ptr<ast::Stmt>($7),
            std::unique_ptr<ast::Stmt>($9),
            @1.first_line
        );
      }
    | FOR LEFT_PARENTHESIS declaration SEMICOLON expression SEMICOLON declaration RIGHT_PARENTHESIS statement {
        $$ = new ast::ForStmt(
            std::unique_ptr<ast::Stmt>($3),
            std::unique_ptr<ast::Expr>($5),
            std::unique_ptr<ast::Stmt>($7),
            std::unique_ptr<ast::Stmt>($9),
            @1.first_line
        );
      }
    | FOREACH LEFT_PARENTHESIS IDENTIFIER COLON expression DOT DOT expression RIGHT_PARENTHESIS statement {
        auto var = new ast::Var(*$3, @1.first_line);
        
        // Create a RangeExpr to represent the range (start..end)
        auto rangeExpr = new ast::RangeExpr(
            std::unique_ptr<ast::Expr>($5),
            std::unique_ptr<ast::Expr>($8),
            @1.first_line
        );
        
        $$ = new ast::ForEachStmt(
            std::unique_ptr<ast::Var>(var), 
            std::unique_ptr<ast::Expr>(rangeExpr), 
            std::unique_ptr<ast::Stmt>($10), 
            @1.first_line);
            
        delete $3;
      }
    | RETURN SEMICOLON { $$ = new ast::ReturnStmt(nullptr, @$.first_line); }
    | RETURN expression SEMICOLON { $$ = new ast::ReturnStmt(std::unique_ptr<ast::Expr>($2), @$.first_line); }
    ;

lvalue
    : IDENTIFIER { $$ = new ast::Var(*$1, @$.first_line); delete $1; }
    | IDENTIFIER index_list {
          auto tmp = new ast::Var(*$1, @$.first_line);
          for (auto& e : *$2) tmp->indices.push_back(std::move(e));
          $$ = tmp;
          delete $1; delete $2;
      }
    ;

expression:
      expression ADDITION expression        { $$ = new ast::Binary(ast::Op::Plus,     std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression SUBTRACTION expression     { $$ = new ast::Binary(ast::Op::Minus,    std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression MULTIPLICATION expression  { $$ = new ast::Binary(ast::Op::Mul,      std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression DIVISION expression        { $$ = new ast::Binary(ast::Op::Div,      std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression MODULUS expression         { $$ = new ast::Binary(ast::Op::Mod,      std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression LESS_THAN expression       { $$ = new ast::Binary(ast::Op::Less,     std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression LESS_THAN_OR_EQUAL expression    { $$ = new ast::Binary(ast::Op::LessEq,    std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression GREATER_THAN_OR_EQUAL expression { $$ = new ast::Binary(ast::Op::GreaterEq, std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression GREATER_THAN expression    { $$ = new ast::Binary(ast::Op::Greater,  std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression EQUAL expression           { $$ = new ast::Binary(ast::Op::Equal,    std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression NOT_EQUAL expression       { $$ = new ast::Binary(ast::Op::NotEqual, std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression AND expression             { $$ = new ast::Binary(ast::Op::And,      std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | expression OR  expression             { $$ = new ast::Binary(ast::Op::Or,       std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | NOT expression                        { $$ = new ast::Unary( ast::Op::Not,   std::unique_ptr<ast::Expr>($2), @$.first_line); }
    | SUBTRACTION expression %prec UMINUS   { $$ = new ast::Unary( ast::Op::Minus, std::unique_ptr<ast::Expr>($2), @$.first_line); }
    | lvalue DOUBLE_ADDITION                { $$ = new ast::Postfix(ast::Op::Inc,  std::unique_ptr<ast::Var>($1), @$.first_line); }
    | lvalue DOUBLE_SUBTRACTION             { $$ = new ast::Postfix(ast::Op::Dec,  std::unique_ptr<ast::Var>($1), @$.first_line); }
    | LEFT_PARENTHESIS expression RIGHT_PARENTHESIS                    { $$ = $2; }
    | IDENTIFIER LEFT_PARENTHESIS call_argument_list RIGHT_PARENTHESIS { $$ = new ast::Call(*$1, std::move(*$3), @$.first_line); delete $1; }
    | IDENTIFIER LEFT_PARENTHESIS RIGHT_PARENTHESIS { $$ = new ast::Call(*$1, {}, @$.first_line); delete $1; }
    | lvalue ASSIGNMENT expression          { $$ = new ast::Assign(std::unique_ptr<ast::Var>($1), std::unique_ptr<ast::Expr>($3), @$.first_line); }
    | lvalue                                { $$ = $1; }
    | INTEGER_CONSTANT                      { $$ = new ast::IntLit($1, @$.first_line); }
    | REAL_CONSTANT                         { $$ = new ast::RealLit($1, @$.first_line); }
    | STRING_CONSTANT                       { $$ = new ast::StringLit(*$1, @$.first_line); delete $1; }
    | TRUE_CONSTANT                         { $$ = new ast::BoolLit(true,  @$.first_line); }
    | FALSE_CONSTANT                        { $$ = new ast::BoolLit(false, @$.first_line); }
    | CHAR_CONSTANT                         { $$ = new ast::CharLit($1, @$.first_line); }

call_argument_list:
      expression                          { $$ = new ast::ExprList(); $$->push_back(std::unique_ptr<ast::Expr>($1)); }
    | call_argument_list COMMA expression { $$ = $1; $1->push_back(std::unique_ptr<ast::Expr>($3)); }
    ;

index_list:
      LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET            { auto tmp = new ast::ExprList(); tmp->push_back(std::unique_ptr<ast::Expr>($2)); $$ = tmp; }
    | index_list LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET { $1->push_back(std::unique_ptr<ast::Expr>($3)); $$ = $1; }
    ;

declaration:
      type init_declarator_list {
        for (auto& decl : $2->decls) { decl->varType = *$1; }
        $$ = $2;
        delete $1;
      }
    | CONST type const_init_list {
        for (auto& decl : $3->decls) { decl->varType = *$2; }
        $$ = $3;
        delete $2;
      }
    ;

init_declarator_list:
      init_declarator {
        auto tmp = new ast::VarDeclList();
        tmp->decls.push_back(std::unique_ptr<ast::VarDecl>($1));
        $$ = tmp;
      }
    | init_declarator_list COMMA init_declarator {
        $1->decls.push_back(std::unique_ptr<ast::VarDecl>($3));
        $$ = $1;
      }
    ;

init_declarator:
      IDENTIFIER {
        auto type = new ast::Type(ast::BasicType::Void);
        auto decl = new ast::VarDecl(*type, *$1, nullptr, false, @$.first_line);
        delete $1; delete type;
        $$ = decl;
      }
    | IDENTIFIER ASSIGNMENT expression {
        auto type = new ast::Type(ast::BasicType::Void);
        auto decl = new ast::VarDecl(*type, *$1, std::unique_ptr<ast::Expr>($3), false, @$.first_line);
        delete $1; delete type;
        $$ = decl;
      }
    | IDENTIFIER dim_list {
        auto decl = $2;
        decl->name = *$1;
        delete $1;
        $$ = decl;
      }
    | IDENTIFIER dim_list ASSIGNMENT expression {
        auto decl = $2;
        decl->name = *$1;
        decl->init = std::unique_ptr<ast::Expr>($4);
        delete $1;
        $$ = decl;
      }
    ;

const_init_list:
      const_init_declarator {
        auto tmp = new ast::VarDeclList();
        tmp->decls.push_back(std::unique_ptr<ast::ConstDecl>($1));
        $$ = tmp;
      }
    | const_init_list COMMA const_init_declarator {
        $1->decls.push_back(std::unique_ptr<ast::VarDecl>($3));
        $$ = $1;
      }
    ;
const_init_declarator:
      IDENTIFIER ASSIGNMENT expression {
        $$ = new ast::ConstDecl(
                 ast::BasicType::Void,
                 *$1,                       /* 變數名稱 */
                 std::unique_ptr<ast::Expr>($3),
                 @$.first_line
             );
        delete $1;
      }
    | IDENTIFIER dim_list ASSIGNMENT expression {
        auto cd = new ast::ConstDecl(ast::BasicType::Void, *$1, std::unique_ptr<ast::Expr>($4), @$.first_line);
        cd->dims = $2->dims;
        $$ = cd;
        delete $1;
        delete $2;
      }
    ;

type:
      BOOLEAN{ $$ = new ast::Type(ast::BasicType::Bool); }
    | CHAR{ $$ = new ast::Type(ast::BasicType::Char); }
    | INT{ $$ = new ast::Type(ast::BasicType::Int); }
    | FLOAT{ $$ = new ast::Type(ast::BasicType::Float); }
    | DOUBLE{ $$ = new ast::Type(ast::BasicType::Double); }
    | STRING{ $$ = new ast::Type(ast::BasicType::String); }
    ;

dim_list:
      LEFT_SQUARE_BRACKET INTEGER_CONSTANT RIGHT_SQUARE_BRACKET{
        auto type = new ast::Type(ast::BasicType::Void);
        type->dims.push_back($2);
        auto tmp = new ast::VarDecl(*type, "", nullptr, false, @$.first_line);
        tmp->dims.push_back($2);
        $$ = tmp;
      }
    | dim_list LEFT_SQUARE_BRACKET INTEGER_CONSTANT RIGHT_SQUARE_BRACKET{
        auto tmp = $1;
        tmp->varType.dims.push_back($3);
        tmp->dims.push_back($3);
        $$ = tmp;
    }
    ;
function_declaration:
      VOID IDENTIFIER LEFT_PARENTHESIS argument_list RIGHT_PARENTHESIS block{
        std::vector<std::unique_ptr<ast::VarDecl>> params;
        for (auto& decl : $4->decls) { params.push_back(std::move(decl)); }
        $$ = new ast::FuncDecl(ast::Type(ast::BasicType::Void), *$2, std::move(params), std::unique_ptr<ast::Stmt>($6), @$.first_line);
        delete $2; delete $4;
    }
    | type IDENTIFIER LEFT_PARENTHESIS argument_list RIGHT_PARENTHESIS block{
        std::vector<std::unique_ptr<ast::VarDecl>> params;
        for (auto& decl : $4->decls) { params.push_back(std::move(decl)); }
        $$ = new ast::FuncDecl(*$1, *$2, std::move(params), std::unique_ptr<ast::Stmt>($6), @$.first_line);
        delete $1; delete $2; delete $4;
    }
    ;
argument_list:
      /* empty */ {
        auto tmp = new ast::VarDeclList();
        $$ = tmp;
      }
    | type IDENTIFIER {
        auto tmp = new ast::VarDeclList();
        auto var_decl = new ast::VarDecl(*$1, *$2, nullptr, false, @$.first_line);
        tmp->decls.push_back(std::unique_ptr<ast::VarDecl>(var_decl));
        $$ = tmp;
        delete $1; delete $2;
      }
    | type IDENTIFIER dim_list {
        auto tmp = new ast::VarDeclList();
        $3->varType.kind = $1->kind;
        $3->name = *$2;
        tmp->decls.push_back(std::unique_ptr<ast::VarDecl>($3));
        $$ = tmp;
        delete $1; delete $2;
      }
    | argument_list COMMA type IDENTIFIER {
        auto tmp = $1;
        auto var_decl = new ast::VarDecl(*$3, *$4, nullptr, false, @$.first_line);
        tmp->decls.push_back(std::unique_ptr<ast::VarDecl>(var_decl));
        $$ = tmp;
        delete $3; delete $4;
      }
    | argument_list COMMA type IDENTIFIER dim_list {
        auto tmp = $1;
        $5->varType.kind = $3->kind;
        $5->name = *$4;
        tmp->decls.push_back(std::unique_ptr<ast::VarDecl>($5));
        $$ = tmp;
        delete $3; delete $4;
      }
    ;
%%
void yyerror(std::string s) {
    extern YYLTYPE yylloc;
    cerr << "line " << yylloc.first_line << ": " << s << endl;
    exit(EXIT_FAILURE);
}

void yywarning(std::string s) {
    cerr << "Warning: " << s << endl;
    exit(EXIT_FAILURE);
}

ast::Program* parse() {
    if (yyparse() == 1) yyerror("Parsing error !");
    return root;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf ("Usage: parser <FILE_NAME>\n");
        exit(1);
    }
    yyin = fopen(argv[1], "r");

    auto program_name = std::string(argv[1]);
    program_name = program_name.substr(0, program_name.find_last_of('.'));

    std::string outputFilename = program_name + ".j";
    outStream.open(outputFilename);
    if (!outStream.is_open()) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        exit(1);
    }

    // Parse the input file and generate the AST
    auto AbstractSyntaxTree = parse();

    // Parse the AST and do the semantic analysis
    SymbolTable symtab;
    SemanticAnalyzer semanticAnalyzer(symtab);
    semanticAnalyzer.analyze(*AbstractSyntaxTree);

    CodeEmitter emitter(outStream);
    CodeGenContext ctx(program_name);
    CodeGenVisitor codegen(emitter, ctx, symtab); 
    codegen.generate(*AbstractSyntaxTree);

    cout << "Parsing completed successfully!" << endl;   

    outStream.close();
    return 0;
}