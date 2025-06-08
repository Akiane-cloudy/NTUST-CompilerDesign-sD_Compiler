// ============================================================================
// CodeEmitter.hpp   —   lightweight helper for emitting JVM Jasmin code
// ----------------------------------------------------------------------------
//  • Handles indentation uniformly (4 spaces by default)
//  • Automatically adds newline for emit() so visitor 只需關心指令內容
//  • push()/pop() controls indentation depth
//  • emitRaw() lets you寫 header 或註解而不自動換行
// ============================================================================
#pragma once

#include <ostream>
#include <string>
#include <algorithm>

class CodeEmitter {
public:
    explicit CodeEmitter(std::ostream& output, int indentSpaces = 4)
        : out(output), indentSize(indentSpaces) {}

    // 禁 copy, 也禁 move (因为 out 是引用类型，不能重新绑定)
    CodeEmitter(const CodeEmitter&) = delete;
    CodeEmitter& operator=(const CodeEmitter&) = delete;
    CodeEmitter(CodeEmitter&&) = default;
    CodeEmitter& operator=(CodeEmitter&&) = delete;

    // 產生一行指令 (自動縮排 + 換行)
    void emit(const std::string& line) {
        writeIndent();
        out << line << '\n';
    }

    // 不換行、不自動縮排 (用於 .class header 或自行排版)
    void emitRaw(const std::string& text) { out << text; }

    // 只插入縮排後的換行 (搭配 emitRaw 使用)
    void newline() { out << '\n'; }

    void push() { ++indentLevel; }
    void pop()  { indentLevel = std::max(0, indentLevel - 1); }

    int currentIndent() const { return indentLevel; }

private:
    std::ostream& out;
    int indentLevel = 0;
    int indentSize  = 4;

    void writeIndent() {
        for (int i = 0; i < indentLevel * indentSize; ++i) out.put(' ');
    }
};
