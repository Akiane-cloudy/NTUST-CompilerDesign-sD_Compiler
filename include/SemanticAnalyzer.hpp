#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP
#include "AST.hpp"
#include "SymbolTable.hpp"

// Constant folding evaluator prototype
std::optional<ConstValue> evalConstExpr(ast::Expr* e);

// SemanticAnalyzer performs semantic checks and type resolution
class SemanticAnalyzer : public ast::Visitor {
   public:
    SemanticAnalyzer(SymbolTable& st) : symtab(st) {};
    void analyze(ast::Program& prog);

    // Visitor overrides
    void visit(ast::Program& p) override;
    void visit(ast::VarDecl& d) override;
    void visit(ast::ConstDecl& d) override;
    void visit(ast::VarDeclList& dl) override;
    void visit(ast::ExprStmt& s) override;
    void visit(ast::EmptyStmt& s) override;
    void visit(ast::Assign& a) override;
    void visit(ast::IfStmt& s) override;
    void visit(ast::WhileStmt& s) override;
    void visit(ast::ForStmt& s) override;
    void visit(ast::ForEachStmt& s) override;
    void visit(ast::ReturnStmt& s) override;
    void visit(ast::Var& v) override;
    void visit(ast::IntLit& e) override;
    void visit(ast::RealLit& e) override;
    void visit(ast::StringLit& e) override;
    void visit(ast::BoolLit& e) override;
    void visit(ast::CharLit& e) override;
    void visit(ast::Unary& u) override;
    void visit(ast::Binary& b) override;
    void visit(ast::RangeExpr& r) override;
    void visit(ast::Postfix& p) override;
    void visit(ast::Call& c) override;
    void visit(ast::Print& p) override;
    void visit(ast::Println& p) override;
    void visit(ast::Read& r) override;
    void visit(ast::Block& b) override;
    void visit(ast::DeclList& dl) override;
    void visit(ast::FuncDecl& fd) override;

   private:
    SymbolTable& symtab;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::optional<ast::Type> currentFunctionReturnType; // Track current function's return type

    // full path return analysis
    bool stmtReturns(ast::Stmt* s);
    bool allPathsReturn(const std::vector<std::unique_ptr<ast::Stmt>>& stmts);

    void error(int line, const std::string& msg);
    void warning(int line, const std::string& msg);

    int skipBlockScopeOnce{0};  // Skip block scope once
};

#endif
