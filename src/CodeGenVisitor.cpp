#include "CodeGenVisitor.hpp"
#include <sstream>
#include <iostream>

using namespace ast;

static std::string jasmType(const ast::Type& t) {
    using BT = ast::BasicType;
    switch (t.kind) {
        case BT::Int:    return "int";
        case BT::Bool:   return "boolean";
        case BT::String: return "java.lang.String";
        case BT::Void:   return "void";
        default:         return "int";          
    }
}

void CodeGenVisitor::generate(Program& root) { 
    root.accept(*this); 
}

void CodeGenVisitor::visit(Program& n) {
    if (ctx.className.empty()) ctx.className = "example";
    em.emit("class " + ctx.className);
    em.emit("{");
    em.push();

    for (auto& d : n.globals) {
        if (auto* vdl = dynamic_cast<VarDeclList*>(d.get())) {
            for (auto& inner : vdl->decls) {
                auto* vd = inner.get();
                std::string type;
                switch (vd->varType.kind) {
                    case BasicType::Int:    type = "int"; break;
                    case BasicType::Bool:   type = "boolean"; break;
                    case BasicType::String: type = "java.lang.String"; break;
                    default: continue;
                }
                std::string instruction = "field static " + type + " " + vd->name;
                if (vd->init) {
                    // handle literal initializers inline
                    if (auto* il = dynamic_cast<ast::IntLit*>(vd->init.get())) {
                        instruction += " = " + std::to_string(il->value);
                    } else if (auto* bl = dynamic_cast<ast::BoolLit*>(vd->init.get())) {
                        instruction += " = " + std::string(bl->value ? "1" : "0");
                    } else if (auto* sl = dynamic_cast<ast::StringLit*>(vd->init.get())) {
                        instruction += " = \"" + sl->value + "\"";
                    } else {
                        // non-literal initializer: emit separately
                        vd->init->accept(*this);
                        em.emit(instruction);
                        continue;
                    }
                }
                em.emit(instruction);
            }
        } else if (auto* vd = dynamic_cast<VarDecl*>(d.get())) {
            std::string type;
            switch (vd->varType.kind) {
                case BasicType::Int:    type = "int"; break;
                case BasicType::Bool:   type = "boolean"; break;
                case BasicType::String: type = "java.lang.String"; break;
                default: continue;
            }
            std::string instruction = "field static " + type + " " + vd->name;
            if (vd->init) {
                // handle literal initializers inline
                if (auto* il = dynamic_cast<ast::IntLit*>(vd->init.get())) {
                    instruction += " = " + std::to_string(il->value);
                } else if (auto* bl = dynamic_cast<ast::BoolLit*>(vd->init.get())) {
                    instruction += " = " + std::string(bl->value ? "1" : "0");
                } else if (auto* sl = dynamic_cast<ast::StringLit*>(vd->init.get())) {
                    instruction += " = \"" + sl->value + "\"";
                } else {
                    // non-literal initializer: emit separately
                    em.emit(instruction);
                    vd->init->accept(*this);
                    continue;
                }
            }
            em.emit(instruction);
        }
    }

    // function decl (from globals + stmts)
    auto emitFuncs = [&](auto& vec) {
        for (auto& n : vec) {
            if (auto* f = dynamic_cast<FuncDecl*>(n.get())) {
                f->accept(*this);
            }
        }
    };
    emitFuncs(n.globals);
    emitFuncs(n.stmts);

    em.pop();
    em.emit("}");
}

//---------------------------------------------------------------
// Function (only static, simple param list)
//---------------------------------------------------------------
void CodeGenVisitor::visit(ast::FuncDecl& fn)
{
    const SymEntry ent = fn.sym;             
    std::stringstream sig;
    sig << jasmType(ent.returnType.value()) << ' ' << fn.name << '(';

    if (fn.name == "main") {                   
        sig << "java.lang.String[]";
    } else if (ent.paramTypes) {
        for (size_t i = 0; i < ent.paramTypes->size(); ++i) {
            sig << jasmType((*ent.paramTypes)[i]);
            if (i + 1 < ent.paramTypes->size()) sig << ", ";
        }
    }
    sig << ')';

    em.emit("method public static " + sig.str());
    em.emit("max_stack 32");
    em.emit("max_locals 32");
    em.emit("{");
    em.push();

    ctx.resetLocal(ent.paramTypes ? ent.paramTypes->size() : 0);

    if (fn.body) fn.body->accept(*this);
    if (ent.returnType->kind == ast::BasicType::Void)
        em.emit("return");

    em.pop();
    em.emit("}");
}


//---------------------------------------------------------------
void CodeGenVisitor::visit(Block& b) {
    for (auto& s : b.stmts) {
        s->accept(*this);
    }
}    

//---------------------------------------------------------------
// Variable decl / assign
//---------------------------------------------------------------
void CodeGenVisitor::visit(VarDecl& d) {
    if (d.init) {
        d.init->accept(*this);
        emitStore(d.sym);
    }
}

void CodeGenVisitor::visit(Assign& a) {
    a.rhs->accept(*this);
    auto e = a.lhs->sym;
    emitStore(a.lhs->sym);
}    

//---------------------------------------------------------------
// Print / Println
//---------------------------------------------------------------
static std::string sig(const Type& t) {
    switch (t.kind) {
        case BasicType::Int:    return "(int)";
        case BasicType::Bool:   return "(boolean)";
        case BasicType::String: return "(java.lang.String)";
        default:                return "(int)";
    }
}

void CodeGenVisitor::visit(Print& p) {
    em.emit("getstatic java.io.PrintStream java.lang.System.out");
    p.expr->accept(*this);
    em.emit("invokevirtual void java.io.PrintStream.print" + sig(p.expr->ty));
}

void CodeGenVisitor::visit(Println& p) {
    em.emit("getstatic java.io.PrintStream java.lang.System.out");
    p.expr->accept(*this);
    em.emit("invokevirtual void java.io.PrintStream.println" + sig(p.expr->ty));
}

//---------------------------------------------------------------
// Control: if / while
//---------------------------------------------------------------
void CodeGenVisitor::visit(IfStmt& s) {
    auto L = ctx.newLabel();
    s.cond->accept(*this);
    em.emit("ifeq " + L);
    s.thenStmt->accept(*this);
    
    if (s.elseStmt) {
        auto Lend = ctx.newLabel();
        em.emit("goto " + Lend);
        em.emit(L + ":");
        s.elseStmt->accept(*this);
        em.emit(Lend + ":");
    } else {
        em.emit(L + ":");
    }
}

void CodeGenVisitor::visit(WhileStmt& s) {
    auto L1 = ctx.newLabel(), L2 = ctx.newLabel();
    em.emit(L1 + ":");
    s.cond->accept(*this);
    em.emit("ifeq " + L2);
    s.body->accept(*this);
    em.emit("goto " + L1);
    em.emit(L2 + ":");
}

//---------------------------------------------------------------
// Literals & Var
//---------------------------------------------------------------
void CodeGenVisitor::visit(IntLit& n) { 
    int v = n.value;
    if (v >= -1 && v <= 5) {
        switch (v) {
            case -1: em.emit("iconst_m1"); break;
            case 0:  em.emit("iconst_0");  break;
            case 1:  em.emit("iconst_1");  break;
            case 2:  em.emit("iconst_2");  break;
            case 3:  em.emit("iconst_3");  break;
            case 4:  em.emit("iconst_4");  break;
            case 5:  em.emit("iconst_5");  break;
        }
    }
    else if (v >= -128 && v <= 127) {
        em.emit("bipush " + std::to_string(v));
    }
    else {
        em.emit("ldc " + std::to_string(v));
    }
}

void CodeGenVisitor::visit(BoolLit& n) { 
    em.emit(std::string("iconst_") + (n.value ? "1" : "0")); 
}

void CodeGenVisitor::visit(StringLit& n) { 
    em.emit("ldc \"" + n.value + "\""); 
}

void CodeGenVisitor::visit(Var& v) { 
    auto e = v.sym;
    emitLoad(e);
}

//---------------------------------------------------------------
// Unary
//---------------------------------------------------------------
void CodeGenVisitor::visit(Unary& u) { 
    u.rhs->accept(*this); 
    if (u.op == Op::Minus) {
        em.emit("ineg");
    } else if (u.op == Op::Not) {
        auto L = ctx.newLabel(), Lend = ctx.newLabel();
        em.emit("ifeq " + L);
        em.emit("iconst_0");
        em.emit("goto " + Lend);
        em.emit(L + ":");
        em.emit("iconst_1");
        em.emit(Lend + ":");
    }
}

//---------------------------------------------------------------
// Binary (int算術 & bool/logical)
//---------------------------------------------------------------
void CodeGenVisitor::visit(Binary& b) { 
    b.lhs->accept(*this); 
    b.rhs->accept(*this); 
    switch (b.op) {
        case Op::Plus: 
            em.emit("iadd"); 
            break;
        case Op::Minus: 
            em.emit("isub"); 
            break;
        case Op::Mul: 
            em.emit("imul"); 
            break;
        case Op::Div: 
            em.emit("idiv"); 
            break;
        case Op::Mod: 
            em.emit("irem"); 
            break;
        case Op::Less: 
        case Op::LessEq: 
        case Op::Greater: 
        case Op::GreaterEq: 
        case Op::Equal: 
        case Op::NotEqual: {
            std::string Ltrue = ctx.newLabel(), Lend = ctx.newLabel();
            em.emit("isub");
            switch (b.op) {
                case Op::Less:       
                    em.emit("iflt " + Ltrue); 
                    break;
                case Op::LessEq:     
                    em.emit("ifle " + Ltrue); 
                    break;
                case Op::Greater:    
                    em.emit("ifgt " + Ltrue); 
                    break;
                case Op::GreaterEq:  
                    em.emit("ifge " + Ltrue); 
                    break;
                case Op::Equal:      
                    em.emit("ifeq " + Ltrue); 
                    break;
                case Op::NotEqual:   
                    em.emit("ifne " + Ltrue); 
                    break;
                default: 
                    break;
            }
            em.emit("iconst_0");
            em.emit("goto " + Lend);
            em.emit(Ltrue + ":");
            em.emit("iconst_1");
            em.emit(Lend + ":");
            break; 
        }
        case Op::And: {
            em.emit("iand"); 
            break;
        } 
        case Op::Or: 
            em.emit("ior"); 
            break;
        default: 
            break; 
    }
}

//---------------------------------------------------------------
// Return (只支援 void / int / bool / string)
//---------------------------------------------------------------
void CodeGenVisitor::visit(ReturnStmt& r) { 
    if (r.expr) { 
        r.expr->accept(*this); 
        em.emit("ireturn"); 
    } else {
        em.emit("return"); 
    }
}

//---------------------------------------------------------------
// Call (static, same class, void / int / bool / string)
//---------------------------------------------------------------
void CodeGenVisitor::visit(ast::Call& c)
{
    for (auto& arg : c.args) arg->accept(*this);

    const SymEntry fn = c.sym;                     
    std::stringstream sig;
    sig << '(';
    if (fn.paramTypes) {
        for (size_t i = 0; i < fn.paramTypes->size(); ++i) {
            sig << jasmType((*fn.paramTypes)[i]);
            if (i + 1 < fn.paramTypes->size()) sig << ", ";
        }
    }
    sig << ')';
    em.emit("invokestatic " + jasmType(fn.returnType.value()) + ' ' + ctx.className + '.' + fn.name + sig.str());
}


// ----------------------------------------------------------------
// Helper methods for loading/storing variables
// ----------------------------------------------------------------
void CodeGenVisitor::emitLoad(const SymEntry entry) {
    if (entry.isGlobal) {
        std::string desc = (entry.type.kind == BasicType::String ? "java.lang.String" :
                            (entry.type.kind == BasicType::Bool ? "boolean" : "int"));
        em.emit("getstatic " + desc + ' ' + ctx.className + "." + entry.name + " ");
    } else {
        em.emit("iload " + std::to_string(entry.slot));
    }
}

void CodeGenVisitor::emitStore(const SymEntry entry) {
    if (entry.isGlobal) {
        std::string desc = (entry.type.kind == BasicType::String ? "java.lang.String" :
                            (entry.type.kind == BasicType::Bool ? "boolean" : "int"));
        em.emit("putstatic " + desc + ' ' + ctx.className + "." + entry.name + " ");
    } else {
        em.emit("istore " + std::to_string(entry.slot));
    }
}

// ----------------------------------------------------------------
// Other AST node visitors (stubs/minimal implementations)
// ----------------------------------------------------------------
void CodeGenVisitor::visit(ast::ForStmt& s) {
    if (s.init) s.init->accept(*this);
    auto lblStart = ctx.newLabel(), lblEnd = ctx.newLabel();
    em.emit(lblStart + ":");
    if (s.cond) { 
        s.cond->accept(*this); 
        em.emit("ifeq " + lblEnd); 
    }
    if (s.body) s.body->accept(*this);
    if (s.step) s.step->accept(*this);
    em.emit("goto " + lblStart);
    em.emit(lblEnd + ":");
}

void CodeGenVisitor::visit(ast::ForEachStmt& s) {
    auto* range = dynamic_cast<ast::RangeExpr*>(s.collection.get());
    if (!range) return;

    const SymEntry& idxSym = s.var->sym;        // Loop variable i

    range->start->accept(*this);                // push start
    emitStore(idxSym);                          // istore idxSlot

    // Determine ascending or descending order
    range->start->accept(*this);                // push start
    range->end->accept(*this);                  // push end
    std::string L_ascCond  = ctx.newLabel();
    std::string L_descCond = ctx.newLabel();
    std::string L_end      = ctx.newLabel();
    em.emit("if_icmple " + L_ascCond);          // start <= end → ascending
    em.emit("goto " + L_descCond);              // or descending

    // ascending
    em.emit(L_ascCond + ":");
    {
        std::string L_body = ctx.newLabel();
        em.emit("goto " + L_body + "_cond");

        // body
        em.emit(L_body + ":");
        s.body->accept(*this);

        // i = i + 1
        emitLoad(idxSym);
        em.emit("iconst_1");
        em.emit("iadd");
        emitStore(idxSym);

        // condition
        em.emit(L_body + "_cond:");
        emitLoad(idxSym);           // push i
        range->end->accept(*this);  // push end
        em.emit("if_icmple " + L_body);   // i <= end → 進下一輪
        em.emit("goto " + L_end);         // 否則結束
    }

    // acending
    em.emit(L_descCond + ":");
    {
        std::string L_body = ctx.newLabel();
        em.emit("goto " + L_body + "_cond");

        // body
        em.emit(L_body + ":");
        s.body->accept(*this);

        // i = i - 1
        emitLoad(idxSym);
        em.emit("iconst_1");
        em.emit("isub");
        emitStore(idxSym);

        // condiction
        em.emit(L_body + "_cond:");
        emitLoad(idxSym);           // push i
        range->end->accept(*this);  // push end
        em.emit("if_icmpge " + L_body);   // i >= end → 進下一輪
        em.emit("goto " + L_end);
    }

    // Exit loop
    em.emit(L_end + ":");
}

void CodeGenVisitor::visit(ast::VarDeclList& dl) {
    for (auto& d : dl.decls) {
        d->accept(*this);
    }
}

void CodeGenVisitor::visit(ast::DeclList& dl) {
    for (auto& d : dl.decls) {
        d->accept(*this);
    }
}

void CodeGenVisitor::visit(ast::ConstDecl& d) {
    visit(static_cast<ast::VarDecl&>(d)); // treat as VarDecl
}

void CodeGenVisitor::visit(ast::ExprStmt& s) {
    if (s.expr) {
        s.expr->accept(*this);
    }
}

void CodeGenVisitor::visit(ast::EmptyStmt& s) {
    // no-op
}

void CodeGenVisitor::visit(ast::Read& r) {
    // stub: no code
}

void CodeGenVisitor::visit(ast::CharLit& c) {
    em.emit("ldc '" + std::string(1, c.value) + "'");
}

void CodeGenVisitor::visit(ast::RealLit& r) {
    em.emit("ldc2_w " + std::to_string(r.value));
}

void CodeGenVisitor::visit(ast::Postfix& p) {
    const auto& sym = p.operand->sym;
    std::string desc = jasmType(p.ty);
    std::string field = ctx.className + "." + sym.name;

    if (sym.isGlobal) {
        em.emit("getstatic " + desc + " " + field);
        em.emit("dup");            
        em.emit("iconst_1");       
        if (p.op == Op::Inc) em.emit("iadd"); else em.emit("isub");
        em.emit("putstatic " + desc + " " + field);
    } else {
        int slot = sym.slot;
        em.emit("iload " + std::to_string(slot)); 
        em.emit("dup");                             
        em.emit("iconst_1");                        
        if (p.op == Op::Inc) em.emit("iadd"); else em.emit("isub");
        em.emit("istore " + std::to_string(slot));  
    }
}

void CodeGenVisitor::visit(ast::RangeExpr& r) {
    r.start->accept(*this);
    r.end->accept(*this);
}
