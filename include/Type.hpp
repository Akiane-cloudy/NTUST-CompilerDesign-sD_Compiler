#pragma once
#include <variant>
#include <vector>

namespace ast{
    enum class BasicType { Bool,
                       Char,
                       Int,
                       Float,
                       Double,
                       String,
                       Void,
                       ERROR };  // UNDEFINED for error handling

struct Type {
    BasicType kind{BasicType::ERROR};
    std::vector<int> dims;  // empty ⇒ scalar

    Type() = default;
    Type(BasicType k, std::vector<int> d = {}) : kind(k), dims(std::move(d)) {}

    std::string toString() const {
        std::string s;
        switch (kind) {
            case BasicType::Bool:
                s = "bool";
                break;
            case BasicType::Char:
                s = "char";
                break;
            case BasicType::Int:
                s = "int";
                break;
            case BasicType::Float:
                s = "float";
                break;
            case BasicType::Double:
                s = "double";
                break;
            case BasicType::String:
                s = "string";
                break;
            case BasicType::Void:
                s = "void";
                break;
            case BasicType::ERROR:
                s = "error";
                break;
        }
        for (int dim : dims) {
            s += "[" + std::to_string(dim) + "]";
        }
        return s;
    }
    bool isScalar() const { return dims.empty(); }
    bool operator==(const Type& rhs) const { return kind == rhs.kind && dims == rhs.dims; }
};
}
