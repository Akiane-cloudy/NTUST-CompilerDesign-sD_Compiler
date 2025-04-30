%option noyywrap
%option yylineno
%{
    // Includes and macros stay as C++ style here
    #include <iostream>
    #include <fstream>
    #include "../include/y.tab.hpp"
    #include "../include/SymbolTable.hpp"

    static FILE* token_out = fopen("token.txt", "w");
    #undef printf
    #define printf(fmt, ...) fprintf(token_out, fmt, ##__VA_ARGS__)
    
    #define DEBUG 1
    #define APPEND_BUFFER     strcat(buf,yytext)
    #define token(t, s) {APPEND_BUFFER; printf("<%s>\n", s); return t;}
    #define tokenInteger(t, i) {APPEND_BUFFER; yylval.ival = i; return t;}
    #define tokenReal(t, r) {APPEND_BUFFER; yylval.dval = r; return t;}
    #define tokenBool(t, b) {APPEND_BUFFER; const char* str = (b) ? "true" : "false"; printf("<BOOL_CONSTANT>: %s\n", str); yylval.bval = b; return t;}
    #define tokenChar(t, c) {APPEND_BUFFER; yylval.cval = c; return t;}
    #define tokenString(t, s) {APPEND_BUFFER; yylval.sval = new std::string(s); return t;}

    #define MAX_LINE_LENGTH 256
    char buf[MAX_LINE_LENGTH];
    std::string str_buf;
    #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

%x COMMENT_STATE
%x STRING_STATE

/* White space */
whitespace [ \t]*

/* Digits */
digit [0-9]
optional_exponent ([eE][+-]?({digit}+))?

/* Integer */
integer {digit}+

/* Letters */
letter [a-zA-Z]

/* Characters */
esc_char   [abfnrtv0\'\"\\]
cchr       (\\{esc_char}|[^\\'\n])
char        \'{cchr}\'

/* Float */
fraction ("."{integer})
real {digit}+{fraction}{optional_exponent}

/* Identifier */
identifier ({letter})({letter}|{digit})*

%%
"."    { token(DOT,                 "\'.\'"); }
","    { token(COMMA,               "\',\'"); }
":"    { token(COLON,               "\':\'"); }
";"    { token(SEMICOLON,           "\';\'"); }
"("    { token(LEFT_PARENTHESIS,    "\'(\'"); }
")"    { token(RIGHT_PARENTHESIS,   "\')\'"); }
"["    { token(LEFT_SQUARE_BRACKET, "\'[\'"); }
"]"    { token(RIGHT_SQUARE_BRACKET, "\']\'"); }
"{"    { token(LEFT_CURLY_BRACKET,   "\'{\'"); }
"}"    { token(RIGHT_CURLY_BRACKET,  "\'}\'"); }

"\+"   { token(ADDITION,            "\'+\'"); }
"\+\+" { token(DOUBLE_ADDITION,     "\'++\'"); }
"\-"   { token(SUBTRACTION,         "\'-\'"); }
"\-\-" { token(DOUBLE_SUBTRACTION,  "\'--\'"); }
"\*"   { token(MULTIPLICATION,      "\'*\'"); }
"\/"   { token(DIVISION,            "\'/\'"); }
"\%"   { token(MODULUS,             "\'%\'"); }
"\="   { token(ASSIGNMENT,          "\'=\'"); }

"\<"   { token(LESS_THAN,               "\'<\'"); }
"\<\=" { token(LESS_THAN_OR_EQUAL,      "\'<=\'"); }
"\>"   { token(GREATER_THAN,            "\'>\'"); }
"\>\=" { token(GREATER_THAN_OR_EQUAL,   "\'>=\'"); }
"\=\=" { token(EQUAL,                   "\'==\'"); }
"\!\=" { token(NOT_EQUAL,               "\'!=\'"); }

"\&\&" { token(AND,                    "\'&&\'"); }
"\|\|" { token(OR,                     "\'||\'"); }
"\!"   { token(NOT,                    "\'!\'"); }

"bool"     { token(BOOLEAN,        "BOOL"); }
"break"    { token(BREAK,          "BREAK"); }
"case"     { token(CASE,           "CASE"); }
"char"     { token(CHAR,           "CHAR"); }
"const"    { token(CONST,          "CONST"); }
"continue" { token(CONTINUE,       "CONTINUE"); }
"default"  { token(DEFAULT,        "DEFAULT"); }
"do"       { token(DO,             "DO"); }
"double"   { token(DOUBLE,         "DOUBLE"); }
"else"     { token(ELSE,           "ELSE"); }
"extern"   { token(EXTERN,         "EXTERN"); }
"false"    { tokenBool(FALSE_CONSTANT, false); }
"float"    { token(FLOAT,          "FLOAT"); }
"for"      { token(FOR,            "FOR"); }
"foreach"  { token(FOREACH,        "FOREACH"); }
"if"       { token(IF,             "IF"); }
"int"      { token(INT,            "INT"); }
"print"    { token(PRINT,          "PRINT"); }
"println"  { token(PRINTLN,        "PRINTLN"); }
"read"     { token(READ,           "READ"); }
"return"   { token(RETURN,         "RETURN"); }
"string"   { token(STRING,         "STRING"); }
"switch"   { token(SWITCH,         "SWITCH"); }
"true"     { tokenBool(TRUE_CONSTANT, true); }
"void"     { token(VOID,           "VOID"); }
"while"    { token(WHILE,          "WHILE"); }

{identifier} {
    printf("<ID>: %s\n", yytext);
    tokenString(IDENTIFIER, yytext);
}

{integer} {
    printf("<INT_CONSTANT>: %s\n", yytext);
    tokenInteger(INTEGER_CONSTANT, std::stoi(yytext));
}
{real} {
    printf("<REAL_CONSTANT>: %s\n", yytext);
    tokenReal(REAL_CONSTANT, std::stod(yytext));
}
{char} {
    printf("<CHAR_CONSTANT>: %s\n", yytext);
    char c;
    if (yytext[1] == '\\') {
        switch (yytext[2]) {
          case 'a':  c = '\a'; break;
          case 'b':  c = '\b'; break;
          case 'f':  c = '\f'; break;
          case 'n':  c = '\n'; break;
          case 'r':  c = '\r'; break;
          case 't':  c = '\t'; break;
          case 'v':  c = '\v'; break;
          case '0':  c = '\0'; break;
          case '\\': c = '\\'; break;
          case '\'': c = '\''; break;
          case '\"': c = '\"'; break;
          default:   c = yytext[2]; break;
        }
    } else {
        c = yytext[1];
    }
    tokenChar(CHAR_CONSTANT, c);
}

\" {
    APPEND_BUFFER;
    BEGIN(STRING_STATE);
}
<STRING_STATE>\"\" {
    APPEND_BUFFER;
    str_buf += '\"';
}
<STRING_STATE>\" {
    std::string str_copy = str_buf;
    printf("<STRING_CONSTANT>: %s\n", str_copy.c_str());
    str_buf = std::string();
    BEGIN(INITIAL);
    tokenString(STRING_CONSTANT, str_copy.c_str());
} 
<STRING_STATE>. {
    APPEND_BUFFER;
    str_buf += std::string(yytext);
}

"//".* {APPEND_BUFFER;}
"/*" {
    APPEND_BUFFER;
    BEGIN(COMMENT_STATE);
}
<COMMENT_STATE>. {APPEND_BUFFER;}
<COMMENT_STATE>\n {
    APPEND_BUFFER;
    printf("%d: %s", yylineno - 1, buf);
    buf[0] = '\0';
}
<COMMENT_STATE>"*/" {
    APPEND_BUFFER;
    BEGIN(INITIAL);
}


\n {
    APPEND_BUFFER;
    printf("%d: %s", yylineno - 1, buf);
    buf[0] = '\0';
}

{whitespace} {APPEND_BUFFER;}

. {
    APPEND_BUFFER;
    printf("%d: %s\n", yylineno - 1, buf);
    printf("bad character: '%s'\n", yytext);
    return BAD_CHARACTER;
}

<<EOF>>  {
    if (buf[0] != '\0') {
        printf("%d: %s\n", yylineno - 1, buf);
        buf[0] = '\0';
    }
    return 0;
}

%%