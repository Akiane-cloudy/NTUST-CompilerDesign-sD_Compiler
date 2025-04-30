#include "../include/SymbolTable.hpp"

void SymbolTable::enterScope() {
    scopes.push_back(std::unordered_map<std::string, SymEntry>());
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    } else {
        std::cerr << "Error: No scope to exit." << std::endl;
    }
}

SymEntry* SymbolTable::lookup(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto entry = it->find(name);
        if (entry != it->end()) {
            return &entry->second;
        }
    }
    return nullptr;  // Not found
}

bool SymbolTable::insert(const SymEntry& entry) {
    if (scopes.empty()) {
        std::cerr << "Error: No scope to insert into." << std::endl;
        return false;
    }
    auto& currentScope = scopes.back();
    auto [result, ok] = currentScope.emplace(entry.name, entry);
    return ok;  // ok :), not ok :(
}

void SymbolTable::dump() const {
    auto last = *scopes.rbegin();
    size_t index = 1;

    std::cout << "============ Dump Start ============" << std::endl;
    if (!last.size()) std::cout << "No symbols be declared in this scope." << std::endl;
    for (const auto& [name, _] : last) {
        std::cout << index++ << ". "
                  << '\"' << name << "\", "<< _.type.toString() 
                  << ((_.isConst) ? ", Const" : "")
                  << ((_.type.dims.size()) ? (", Array with " +  std::to_string(_.type.dims.size()) + "dims") : "")
                  << ((_.isFunc) ? ", Function" : "")
                  << std::endl;
    }
    std::cout << "============ Dump End ==============\n" << std::endl;
}