#pragma once

#include <string>
#include <stack>

struct CodeGenContext {
    CodeGenContext(std::string className = "example")
        : className(std::move(className)) {}
        
    // ---------------- public data ----------------
    std::string className = "example";   // 由檔名或 Program node 決定

    // ---------------- label helpers -------------
    std::string newLabel() { return "L" + std::to_string(nextLabel++); }

    // ---------------- local slot helpers --------
    int allocLocal()        { return nextLocal++; }
    void resetLocal(int n = 0){ nextLocal = n; }
    int currentLocal() const{ return nextLocal; }

    // ---------------- loop stacks (break/continue)
    void pushLoop(const std::string& beginLbl, const std::string& exitLbl) {
        loopBegin.push(beginLbl);
        loopExit.push(exitLbl);
    }
    void popLoop() {
        loopBegin.pop();
        loopExit.pop();
    }
    std::string topLoopBegin() const { return loopBegin.top(); }
    std::string topLoopExit()  const { return loopExit.top();  }

private:
    int nextLabel = 0;          // global label counter L0, L1, ...
    int nextLocal = 0;          // per‑method local slot counter

    std::stack<std::string> loopBegin;
    std::stack<std::string> loopExit;
};