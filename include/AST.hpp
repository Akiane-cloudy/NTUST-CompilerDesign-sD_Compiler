// -------------------------------- AST.hpp ---------------------------------
// Construct a simple AST to maintain the structure of the language
// and provide a base for semantic analysis (and code generation in the future).
// ---------------------------------------------------------------------------
#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>

namespace ast {

//--------------------------------------------------------------
// 1.  Base Node (with line number)
//--------------------------------------------------------------
struct Node {
    int line{0};
    explicit Node(int l = 0) : line(l) {}
    virtual void accept(struct Visitor&) = 0;
    virtual ~Node() = default;
};

//--------------------------------------------------------------
// 2.  Forward declarations (include Expr before usage)
//--------------------------------------------------------------
struct Expr;
struct Program;
struct IntLit;
struct RealLit;
struct StringLit;
struct BoolLit;
struct CharLit;
struct Var;
struct Unary;
struct Binary;
struct Postfix;
struct Call;
struct Assign;
struct Print;
struct Println;
struct Read;
struct Block;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct ForEachStmt;
struct ReturnStmt;
struct ExprStmt;
struct EmptyStmt;
struct DeclList;
struct VarDecl;
struct VarDeclList;
struct ConstDecl;
struct FuncDecl;
struct RangeExpr;

//--------------------------------------------------------------
// 3.  Visitor interface (multi‑pass ready)
//--------------------------------------------------------------
struct Visitor {
    // Expr
    virtual void visit(IntLit&) = 0;
    virtual void visit(RealLit&) = 0;
    virtual void visit(StringLit&) = 0;
    virtual void visit(BoolLit&) = 0;
    virtual void visit(CharLit&) = 0;
    virtual void visit(Var&) = 0;
    virtual void visit(Unary&) = 0;
    virtual void visit(Binary&) = 0;
    virtual void visit(Postfix&) = 0;
    virtual void visit(Call&) = 0;
    virtual void visit(Assign&) = 0;
    virtual void visit(RangeExpr&) = 0;
    virtual void visit(Print&) = 0;
    virtual void visit(Println&) = 0;
    virtual void visit(Read&) = 0;
    // Stmt / Decl / Program
    virtual void visit(Block&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(ForStmt&) = 0;
    virtual void visit(ForEachStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(EmptyStmt&) = 0;
    virtual void visit(DeclList&) = 0;
    virtual void visit(VarDecl&) = 0;
    virtual void visit(VarDeclList&) = 0;
    virtual void visit(ConstDecl&) = 0;
    virtual void visit(FuncDecl&) = 0;
    virtual void visit(Program&) = 0;
    virtual ~Visitor() = default;
};

//--------------------------------------------------------------
// 4.  Type system (scalar + array dims)
//--------------------------------------------------------------
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

//--------------------------------------------------------------
// 5.  Operators (for Unary/Binary/Postfix)
//--------------------------------------------------------------
enum class Op {
    // binary
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    Less,
    LessEq,
    GreaterEq,
    Greater,
    Equal,
    NotEqual,
    And,
    Or,
    // unary
    Neg,
    Not,
    // postfix
    Inc,
    Dec
};

//--------------------------------------------------------------
// 6.  Expression hierarchy
//--------------------------------------------------------------
struct Expr : Node {
    Type ty;  // filled by TypeChecker pass
    using Node::Node;
};

struct IntLit : Expr {
    int value;
    IntLit(int v, int line = 0) : Expr(line), value(v) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct RealLit : Expr {
    double value;
    RealLit(double v, int line = 0) : Expr(line), value(v) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct StringLit : Expr {
    std::string value;
    StringLit(std::string v, int line = 0) : Expr(line), value(std::move(v)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct BoolLit : Expr {
    bool value;
    BoolLit(bool v, int line = 0) : Expr(line), value(v) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct CharLit : Expr {
    char value;
    CharLit(char v, int line = 0) : Expr(line), value(v) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Var : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> indices;  // for arrays
    explicit Var(std::string n, int line = 0) : Expr(line), name(std::move(n)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Unary : Expr {
    Op op;
    std::unique_ptr<Expr> rhs;
    Unary(Op o, std::unique_ptr<Expr> e, int line = 0)
        : Expr(line), op(o), rhs(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct Binary : Expr {
    Op op;
    std::unique_ptr<Expr> lhs, rhs;
    Binary(Op o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, int line = 0)
        : Expr(line), op(o), lhs(std::move(l)), rhs(std::move(r)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct Postfix : Expr {
    Op op;
    std::unique_ptr<Expr> operand;
    Postfix(Op o, std::unique_ptr<Expr> e, int line = 0)
        : Expr(line), op(o), operand(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
struct Call : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    Call(std::string c, std::vector<std::unique_ptr<Expr>> a, int line = 0)
        : Expr(line), callee(std::move(c)), args(std::move(a)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct RangeExpr : Expr {
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    RangeExpr(std::unique_ptr<Expr> s, std::unique_ptr<Expr> e, int line = 0)
        : Expr(line), start(std::move(s)), end(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Assign : Expr {
    std::unique_ptr<Var> lhs;
    std::unique_ptr<Expr> rhs;
    Assign(std::unique_ptr<Var> l, std::unique_ptr<Expr> r, int line = 0)
        : Expr(line), lhs(std::move(l)), rhs(std::move(r)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

//--------------------------------------------------------------
// 7.  Statement / Declaration hierarchy
//--------------------------------------------------------------
struct Stmt : Node {
    using Node::Node;
};

struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    Block(std::vector<std::unique_ptr<Stmt>> s = {}, int line = 0)
        : Stmt(line), stmts(std::move(s)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e, int line = 0) : Stmt(line), expr(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct EmptyStmt : Stmt {
    explicit EmptyStmt(int line = 0) : Stmt(line) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Decl : Stmt {
    bool isConst{false};
    using Stmt::Stmt;
};

struct DeclList : Decl {
    std::vector<std::unique_ptr<Decl>> decls;
    DeclList(std::vector<std::unique_ptr<Decl>> d = {}, int line = 0)
        : Decl(line), decls(std::move(d)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> thenStmt;
    std::unique_ptr<Stmt> elseStmt;  // may be nullptr
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e = {}, int line = 0)
        : Stmt(line), cond(std::move(c)), thenStmt(std::move(t)), elseStmt(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b, int line = 0)
        : Stmt(line), cond(std::move(c)), body(std::move(b)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> step;
    std::unique_ptr<Stmt> body;
    ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> c, std::unique_ptr<Stmt> s, std::unique_ptr<Stmt> b, int line = 0)
        : Stmt(line), init(std::move(i)), cond(std::move(c)), step(std::move(s)), body(std::move(b)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ForEachStmt : Stmt {
    std::unique_ptr<Var> var;
    std::unique_ptr<Expr> collection;
    std::unique_ptr<Stmt> body;
    ForEachStmt(std::unique_ptr<Var> v, std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b, int line = 0)
        : Stmt(line), var(std::move(v)), collection(std::move(c)), body(std::move(b)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> expr;  // may be nullptr
    ReturnStmt(std::unique_ptr<Expr> e = {}, int line = 0)
        : Stmt(line), expr(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct VarDecl : Decl {
    Type varType;
    std::string name;
    std::unique_ptr<Expr> init;  // may be nullptr
    std::vector<int> dims;       // repeated for convenience
    VarDecl(Type t, std::string n, std::unique_ptr<Expr> i = {}, bool isC = false, int line = 0)
        : Decl(line), varType(t), name(std::move(n)), init(std::move(i)) { isConst = isC; }
    void accept(Visitor& v) override { v.visit(*this); }
};

struct VarDeclList : VarDecl {
    std::vector<std::unique_ptr<VarDecl>> decls;
    VarDeclList(Type t = BasicType::ERROR, std::vector<std::unique_ptr<VarDecl>> d = {}, int line = 0)
        : VarDecl(t, "", nullptr, false, line), decls(std::move(d)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ConstDecl : VarDecl {
    ConstDecl(Type t, std::string n, std::unique_ptr<Expr> i, int line = 0)
        : VarDecl(t, std::move(n), std::move(i), true, line) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct FuncDecl : Decl {
    Type returnType;
    std::string name;
    std::vector<std::unique_ptr<VarDecl>> params;
    std::unique_ptr<Stmt> body;
    FuncDecl(Type r, std::string n, std::vector<std::unique_ptr<VarDecl>> p, std::unique_ptr<Stmt> b, int line = 0)
        : Decl(line), returnType(r), name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Print : Stmt {
    std::unique_ptr<Expr> expr;
    Print(std::unique_ptr<Expr> e, int line = 0) : Stmt(line), expr(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Println : Stmt {
    std::unique_ptr<Expr> expr;
    Println(std::unique_ptr<Expr> e, int line = 0) : Stmt(line), expr(std::move(e)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct Read : Stmt {
    std::unique_ptr<Var> var;
    Read(std::unique_ptr<Var> v, int line = 0) : Stmt(line), var(std::move(v)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

//--------------------------------------------------------------
// 8.  Program root
//--------------------------------------------------------------
struct Program : Node {
    std::vector<std::unique_ptr<Decl>> globals;
    std::vector<std::unique_ptr<Stmt>> stmts;
    Program(std::vector<std::unique_ptr<Decl>> g, std::vector<std::unique_ptr<Stmt>> s, int line = 0)
        : Node(line), globals(std::move(g)), stmts(std::move(s)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};
} 

#endif