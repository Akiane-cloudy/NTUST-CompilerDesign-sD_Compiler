// =============================================================
// CodeGenVisitor.hpp  —  AST to JVM assembly generator (header)
// =============================================================
#pragma once

#include "AST.hpp"            // your existing AST node hierarchy
#include "SymbolTable.hpp"    // resolved symbols with slot / global info
#include "CodeEmitter.hpp"    // pretty printer for assembly lines
#include "CodeGenContext.hpp" // label + slot counters

// -------------------------------------------------------------
// CodeGenVisitor — traverses AST and emits javaa assembly lines
// -------------------------------------------------------------
class CodeGenVisitor : public ast::Visitor {
public:
    CodeGenVisitor(CodeEmitter& emitter, CodeGenContext& context, SymbolTable& sym)
        : em(emitter), ctx(context), symtab(sym) {}

    // top‑level entry helper
    void generate(ast::Program& root);

    // --------------- ASTVisitor overrides ---------------
    void visit(ast::Program&     n) override;
    void visit(ast::FuncDecl&    n) override;
    void visit(ast::Block&       n) override;
    void visit(ast::VarDecl&     n) override;
    void visit(ast::Assign&      n) override;
    void visit(ast::IfStmt&      n) override;
    void visit(ast::WhileStmt&   n) override;
    void visit(ast::ForStmt&     n) override;   
    void visit(ast::ReturnStmt&  n) override;
    void visit(ast::Print&       n) override;
    void visit(ast::Println&     n) override;
    
    // -------- expression nodes --------
    void visit(ast::IntLit&      n) override;
    void visit(ast::BoolLit&     n) override;
    void visit(ast::StringLit&   n) override;
    void visit(ast::Var&         n) override;
    void visit(ast::Binary&      n) override;
    void visit(ast::Unary&       n) override;
    void visit(ast::Call&        n) override;

    // literals
    void visit(ast::RealLit&  ) override;
    void visit(ast::CharLit&  ) override;
    // postfix / range
    void visit(ast::Postfix&  ) override;
    void visit(ast::RangeExpr&) override;
    // stmt / decl still missing
    void visit(ast::Read&       ) override;
    void visit(ast::ForEachStmt&) override;
    void visit(ast::ExprStmt&   ) override;
    void visit(ast::EmptyStmt&  ) override;
    void visit(ast::DeclList&   ) override;
    void visit(ast::VarDeclList&) override;
    void visit(ast::ConstDecl&  ) override;


private:
    CodeEmitter&  em;
    CodeGenContext& ctx;
    SymbolTable&  symtab;

    // -------- helper functions --------
    void emitLoad(const SymEntry* entry);   // iload / getstatic
    void emitStore(const SymEntry* entry);  // istore / putstatic
    void emitBinaryOp(ast::Op op, ast::Type t); // iadd / isub / imul …

    // track目前 gen 的最大 stack 深度 (簡易版本：可先寫常值 15，再做估計演算法)
    int maxStack = 15;
};
