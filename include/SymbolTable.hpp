#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <iostream>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "AST.hpp"

using ConstValue = std::variant<int, float, double, std::string, bool, char>;
/*
    Symbol table entry
    - identifier: Variable, Constant Variable, Function
*/
struct SymEntry {
    std::string name;  // identifier name
    ast::Type type;    // type of identifier

    /* For constant variables */
    bool isConst;                     // if const
    std::optional<ConstValue> value;  // const value or value of variable
    
    // For array variables: optional vector of element values
    std::optional<std::vector<ConstValue>> arrayValues;

    /* For functions */
    bool isFunc;
    std::optional<ast::Type> returnType;               // return type
    std::optional<std::vector<ast::Type>> paramTypes;  // param types

    
};

class SymbolTable {
   private:
    std::vector<std::unordered_map<std::string, SymEntry>> scopes;

   public:
    SymbolTable() { enterScope(); }
    void enterScope();
    void exitScope();
    SymEntry* lookup(const std::string&);
    bool insert(const SymEntry&);
    void dump() const;
    bool hasErrors{false};
};

#endif
