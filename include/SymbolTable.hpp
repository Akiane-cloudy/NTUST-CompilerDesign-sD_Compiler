/**
 * @file SymbolTable.hpp
 * @brief Symbol table implementation for compiler semantic analysis
 * 
 * This file contains the symbol table infrastructure for managing scopes,
 * variable declarations, function symbols, and managing memory slots.
 */
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <stack>

#include "AST.hpp"       // AST type definitions

/**
 * @brief Type for constant values that can be stored in the symbol table
 * Supports integers, doubles, strings, booleans, and characters
 */
using ConstValue = std::variant<int, double, std::string, bool, char>;

/**
 * @brief Symbol table entry representing a variable, constant, or function
 * 
 * Each entry stores information about an identifier including its type,
 * code generation metadata, and semantic analysis information.
 */
struct SymEntry {
    std::string name;          // Identifier name
    ast::Type   type;          // Type information: variable type or function return type

    bool isConst = false;      // Whether this is a constant variable
    bool isFunc  = false;      // Whether this is a function symbol

    /**
     * Code generation metadata
     */
    bool isGlobal = false;     // Global (true) or local (false) scope
    int  slot     = -1;        // JVM local variable slot (-1 for globals/functions)

    /**
     * Language semantics tracking
     */
    std::optional<ConstValue>              value;        // Constant value (if known)
    std::optional<std::vector<ConstValue>> arrayValues;  // Array element values (if const array)

    /**
     * Function information
     */
    std::optional<std::vector<ast::Type>> paramTypes;   // Parameter types list
    std::optional<ast::Type>              returnType;   // Function return type (same as type field)
};

/**
 * @brief Symbol table managing nested scopes and slot allocation
 * 
 * The symbol table maintains a stack of scopes and handles:
 * - Scope entry/exit with special handling for function scopes
 * - Symbol insertion and lookup across nested scopes
 * - JVM local variable slot allocation
 */
class SymbolTable {
public:
    /**
     * @brief Constructor that initializes with a global scope
     */
    SymbolTable();

    /**
     * @brief Scope management methods
     */
    void enterScope(bool isFunctionScope = false);  // Creates a new scope; resets slot counter for functions
    void exitScope();                               // Removes the current scope

    /**
     * @brief Symbol insertion and lookup methods
     */
    bool insert(const SymEntry& entry);             // Adds symbol to current scope; returns false if duplicate
    SymEntry*       lookup(const std::string& name);  // Non-const lookup
    const SymEntry* lookup(const std::string& name) const;  // Const lookup

    /**
     * @brief Variable slot management methods
     */
    int  allocateSlot();        // Allocates and returns the next available slot
    int  currentLocal() const;  // Returns current slot counter value
    void resetLocal(int base = 0);  // Resets slot counter to specified base

    /**
     * @brief Helper methods
     */
    bool atGlobalScope() const { return scopes.size() == 1; }  // Checks if we're in global scope

private:
    using Scope = std::unordered_map<std::string, SymEntry>;
    std::vector<Scope> scopes;   // Stack of scopes (scopes[0] is global)

    int nextLocal = 0;                 // Next available slot in current function
    std::stack<int> savedNextLocal;    // Saved slot counters for outer function scopes
};

#endif // SYMBOL_TABLE_HPP