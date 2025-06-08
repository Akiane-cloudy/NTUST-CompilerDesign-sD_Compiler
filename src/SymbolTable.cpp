/**
 * @file SymbolTable.cpp
 * @brief Implementation of the symbol table functionality
 * 
 * This file contains the implementation of the SymbolTable class methods
 * for managing symbol scopes and lookup.
 */
#include "SymbolTable.hpp"
#include <cassert>

/**
 * @brief Constructor that creates the initial global scope
 */
SymbolTable::SymbolTable() {
    scopes.emplace_back();   // Create global scope
}

/**
 * @brief Creates a new scope for variables and symbols
 * 
 * @param isFunctionScope If true, saves the current slot counter and resets for new function scope
 */
void SymbolTable::enterScope(bool isFunctionScope) {
    scopes.emplace_back();  // Create new scope
    if (isFunctionScope) {
        savedNextLocal.push(nextLocal);  // Save outer scope's slot counter
        nextLocal = 0;                   // Reset for new function's parameters and locals
    }
}

/**
 * @brief Removes the current scope and restores previous scope's state
 * 
 * When exiting a function scope, restores the outer scope's slot counter.
 */
void SymbolTable::exitScope() {
    assert(scopes.size() > 1 && "cannot pop global scope");
    scopes.pop_back();  // Remove current scope
    if (!savedNextLocal.empty()) {  // If leaving a function scope
        nextLocal = savedNextLocal.top();  // Restore outer scope's slot counter
        savedNextLocal.pop();
    }
}

/**
 * @brief Inserts a symbol into the current scope
 * 
 * @param inEntry The symbol entry to insert
 * @return true if successful, false if a duplicate exists in current scope
 */
SymEntry* SymbolTable::insert(const SymEntry& inEntry) {
    Scope& cur = scopes.back();
    if (cur.find(inEntry.name) != cur.end())
        return nullptr;

    SymEntry entry = inEntry;
    if (atGlobalScope()) {
        entry.isGlobal = true;
        entry.slot = -1;
    } else if (!entry.isFunc) {
        entry.isGlobal = false;
        entry.slot = allocateSlot();
    }

    auto res = cur.emplace(entry.name, std::move(entry));
    if (!res.second) return nullptr;

    return &res.first->second;
}

/**
 * @brief Finds a symbol by name in all accessible scopes
 * 
 * Searches from innermost to outermost scope for the first matching symbol.
 * 
 * @param name Symbol name to look up
 * @return Pointer to the entry if found, nullptr otherwise
 */
SymEntry* SymbolTable::lookup(const std::string& name) {
    // Search through scopes from inner to outer
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return &f->second;
    }
    return nullptr;  // Symbol not found
}

/**
 * @brief Const version of symbol lookup
 * 
 * @param name Symbol name to look up
 * @return Const pointer to the entry if found, nullptr otherwise
 */
const SymEntry* SymbolTable::lookup(const std::string& name) const {
    // Search through scopes from inner to outer
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return &f->second;
    }
    return nullptr;  // Symbol not found
}

/**
 * @brief Slot allocation helper methods
 */

/**
 * @brief Allocates a new slot for a local variable
 * @return The allocated slot number
 */
int SymbolTable::allocateSlot() { 
    return nextLocal++; 
}

/**
 * @brief Gets the current local slot counter value
 * @return Current value of nextLocal
 */
int SymbolTable::currentLocal() const { 
    return nextLocal; 
}

/**
 * @brief Resets the local slot counter to a specified base
 * @param base The value to reset the counter to (defaults to 0)
 */
void SymbolTable::resetLocal(int base) { 
    nextLocal = base; 
}
