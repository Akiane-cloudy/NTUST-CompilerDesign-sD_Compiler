#include "SemanticAnalyzer.hpp"

#include <cstdlib>

// full-path return analysis helpers
bool SemanticAnalyzer::stmtReturns(ast::Stmt* s) {
    if (auto ret = dynamic_cast<ast::ReturnStmt*>(s)) return true;
    if (auto blk = dynamic_cast<ast::Block*>(s)) return allPathsReturn(blk->stmts);
    if (auto iff = dynamic_cast<ast::IfStmt*>(s)) {
        if (!iff->elseStmt) return false;
        return stmtReturns(iff->thenStmt.get()) && stmtReturns(iff->elseStmt.get());
    }
    return false;
}

bool SemanticAnalyzer::allPathsReturn(const std::vector<std::unique_ptr<ast::Stmt>>& stmts) {
    for (auto& st : stmts) {
        if (stmtReturns(st.get())) return true;
    }
    return false;
}

// Constructor: enter global scope
SemanticAnalyzer::SemanticAnalyzer() {
    symtab.enterScope();
}

// Entry point: analyze program and report errors
void SemanticAnalyzer::analyze(ast::Program& prog) {
    prog.accept(*this);
    // Report errors and warnings
    if (!errors.empty()) {
        for (auto& err : errors)
            std::cerr << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!warnings.empty()) {
        for (auto& warn : warnings)
            std::cerr << "Warning at " << warn << std::endl;
    }
}

// Visit Program: process globals and statements, then exit scope
void SemanticAnalyzer::visit(ast::Program& p) {
    for (auto& declPtr : p.globals)
        declPtr->accept(*this);
    for (auto& stmtPtr : p.stmts)
        stmtPtr->accept(*this);
    symtab.hasErrors = errors.size() > 0;
    symtab.exitScope();
}

// Visit variable declaration
void SemanticAnalyzer::visit(ast::VarDecl& d) {
    if (d.init) {
        d.init->accept(*this);
        if (d.init->ty.kind == ast::BasicType::ERROR) {
            d.varType = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
            return;
        }
        if (!(d.init->ty == d.varType) && !(d.varType.kind == ast::BasicType::Double && d.init->ty.kind == ast::BasicType::Float)) {
            error(d.line, "Type mismatch in initialization of '" + d.name + "'" + ", expected " + d.varType.toString() + " but got " + d.init->ty.toString());
        }
    }

    SymEntry entry;
    entry.name = d.name;    
    entry.type = d.varType;
    // Copy array dimensions into symbol entry
    entry.type.dims = d.dims;
    entry.isConst = d.isConst;
    if (d.init) {
        if (auto cv = evalConstExpr(d.init.get()))
            entry.value = *cv;
    }
    // Initialize storage for array variables
    if (!d.dims.empty()) {
        int total = 1;
        for (int dim : d.dims) total *= dim;
        entry.arrayValues = std::vector<ConstValue>(total);
    }
    if (!symtab.insert(entry)) {
        error(d.line, "Redefinition of variable '" + d.name + "'");
        entry.type = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
    }
}

// Visit const declaration
void SemanticAnalyzer::visit(ast::ConstDecl& d) {
    if (!d.init) {
        error(d.line, "Const \'" + d.name + "\' must be initialized");
        d.varType = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
        return;
    } else {
        d.init->accept(*this);
        if (d.init->ty.kind == ast::BasicType::ERROR) {
            d.varType = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
            return;
        }
        if (!(d.init->ty == d.varType))
            error(d.line, "Type mismatch in initialization of '" + d.name + "'" + ", expected " + d.varType.toString() + " but got " + d.init->ty.toString());
        if (!evalConstExpr(d.init.get()))
            error(d.line, "Const initializer must be constant expression for '" + d.name + "'");
    }

    SymEntry entry;
    entry.name = d.name;
    entry.type = d.varType;
    entry.isConst = true;
    if (d.init) {
        if (auto cv = evalConstExpr(d.init.get()))
            entry.value = *cv;
    }
    if (!symtab.insert(entry)) {
        error(d.line, "Redefinition of const '" + d.name + "'");
        entry.type = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
    }
}

// Visit a list of declarations
void SemanticAnalyzer::visit(ast::VarDeclList& dl) {
    for (auto& vd : dl.decls)
        vd->accept(*this);
}

// Visit empty statement
void SemanticAnalyzer::visit(ast::EmptyStmt& s) {
    // Nothing to do for empty statements
}

// Visit expression statement
void SemanticAnalyzer::visit(ast::ExprStmt& s) {
    s.expr->accept(*this);
}

// Visit assignment
void SemanticAnalyzer::visit(ast::Assign& a) {
    a.rhs->accept(*this);
    a.lhs->accept(*this);

    auto* ent = symtab.lookup(a.lhs->name);
    if (!ent) {
        error(a.line, "Undeclared variable '" + a.lhs->name + "'");
        a.lhs->ty = ast::Type(ast::BasicType::ERROR);
        a.rhs->ty = ast::Type(ast::BasicType::ERROR); 
        return;
    }
    // Handle array element assignment if indices present
    if (!a.lhs->indices.empty()) {
        const auto& dims = ent->type.dims;
        if (a.lhs->indices.size() != dims.size()) {
            error(a.line, "Dimension mismatch in assignment to '" + a.lhs->name + "'");
            return;
        }
        
        // Evaluate and collect constant index values, allow dynamic indices
        bool dynamicIndex = false;
        std::vector<int> idxVals;
        idxVals.reserve(dims.size());
        for (size_t i = 0; i < dims.size(); ++i) {
            auto& idxExpr = a.lhs->indices[i];
            idxExpr->accept(*this);
            if (idxExpr->ty.kind != ast::BasicType::Int) {
                error(a.line, "Array index must be int in assignment to '" + a.lhs->name + "'");
                return;
            }
            auto cvIdx = evalConstExpr(idxExpr.get());
            if (!cvIdx || !std::holds_alternative<int>(*cvIdx)) {
                dynamicIndex = true;
                break;
            }
            int v = std::get<int>(*cvIdx);
            // Bounds for each dim
            if (v < 0 || v >= dims[i]) {
                error(a.line, "Index out of bounds in assignment to '" + a.lhs->name + "'");
                return;
            }
            idxVals.push_back(v);
        }
        if (!ent->arrayValues) {
            error(a.line, "Variable '" + a.lhs->name + "' is not an array");
            return;
        }
        if (dynamicIndex) {
            // dynamic indexing: cannot track element at compile time, but keep existing tracking
            if (ent->isConst)
                error(a.line, "Cannot assign to const '" + a.lhs->name + "'");
            // Element type is base type of array
            ast::Type elemType(ent->type.kind);
            if (!(a.rhs->ty == elemType))
                error(a.line, "Type mismatch in assignment to '" + a.lhs->name + "'" + ", expected '" + elemType.toString() + "' but got " + a.rhs->ty.toString());
            // assignment expression result is the RHS type
            a.ty = a.rhs->ty;
            return;
        }

        // Compute linear index (row-major)
        size_t n = dims.size();
        std::vector<int> strides(n);
        strides[n - 1] = 1;
        for (int k = int(n) - 2; k >= 0; --k) {
            strides[k] = strides[k + 1] * dims[k + 1];
        }
        int linearIndex = 0;
        for (size_t i = 0; i < n; ++i) linearIndex += idxVals[i] * strides[i];
        auto& arr = *ent->arrayValues;

        // Perform assignment
        if (auto cv = evalConstExpr(a.rhs.get())) {
            arr[linearIndex] = *cv;
        } else {
            // Invalidate element tracking
            arr.clear();
        }
        return;
    }
    if (ent->isConst)
        error(a.line, "Cannot assign to const '" + a.lhs->name + "'");
    if (!(a.rhs->ty == ent->type))
        error(a.line, "Type mismatch in assignment to '" + a.lhs->name + "'" +
                          ", expected \'" + ent->type.toString() + "\' but got " + a.rhs->ty.toString());
    if (auto cv = evalConstExpr(a.rhs.get()))
        ent->value = *cv;
    else
        ent->value.reset();

    // assignment expression result is the RHS type. for instance: a = b -> <Type_of_b>
    a.ty = a.rhs->ty;
}

// Visit if statement
void SemanticAnalyzer::visit(ast::IfStmt& s) {
    s.cond->accept(*this);
    if (s.cond->ty.kind != ast::BasicType::Bool) {
        error(s.line, "Condition in if statement must be boolean");
    }
    
    s.thenStmt->accept(*this);
    
    if (s.elseStmt) {
        s.elseStmt->accept(*this);
    }
}

// Visit while statement
void SemanticAnalyzer::visit(ast::WhileStmt& s) {
    // Type check condition
    s.cond->accept(*this);
    if (s.cond->ty.kind != ast::BasicType::Bool) {
        error(s.line, "Condition in while statement must be boolean");
    }
    
    // Check body
    s.body->accept(*this);
}

// Visit for statement
void SemanticAnalyzer::visit(ast::ForStmt& s) {
    symtab.enterScope();
    // Check initialization
    s.init->accept(*this);
    
    // Type check condition
    s.cond->accept(*this);
    if (s.cond->ty.kind != ast::BasicType::Bool) {
        error(s.line, "Condition in for statement must be boolean");
    }
    
    // Check step
    s.step->accept(*this);
    
    // Check body
    ++skipBlockScopeOnce;
    s.body->accept(*this);

    symtab.hasErrors = errors.size() > 0;
    symtab.exitScope();
}

// Visit foreach statement
void SemanticAnalyzer::visit(ast::ForEachStmt& s) {
    // Check variable
    s.var->accept(*this);
    
    // Check collection/range expression
    s.collection->accept(*this);
    if (s.collection->ty.kind == ast::BasicType::ERROR) {
        error(s.line, "Invalid collection in foreach loop");
        return;
    }
    // If this is a range expression (start..end), both sides should be integers
    if (auto* range = dynamic_cast<ast::RangeExpr*>(s.collection.get())) {
        if (range->start->ty.kind != ast::BasicType::Int ||
            range->end->ty.kind != ast::BasicType::Int) {
            error(s.line, "Range bounds in foreach must be integers");
        }
    } else {
        // Only support integer ranges for now
        error(s.line, "Only integer ranges are supported in foreach loops");
    }
    
    // Check body
    s.body->accept(*this);
}

// Visit return statement
void SemanticAnalyzer::visit(ast::ReturnStmt& s) {
    if (!currentFunctionReturnType) {
        error(s.line, "Return statement outside of function.");
        return;
    }
    const auto& expectedType = currentFunctionReturnType.value();
    if (expectedType.kind == ast::BasicType::Void) {
        if (s.expr) {
            s.expr->accept(*this);
            error(s.line, "Cannot return a value from a void function.");
        }
    } else {
        if (!s.expr) {
            error(s.line, "Return statement missing expression in non-void function.");
        } else {
            s.expr->accept(*this);
            if (s.expr->ty.kind != ast::BasicType::ERROR) {
                if (!(s.expr->ty == expectedType)) {
                    error(s.line, "Return type mismatch: expected '" + expectedType.toString() + "' but got '" + s.expr->ty.toString() + "'.");
                }
            }
        }
    }
}

// Visit variable usage
void SemanticAnalyzer::visit(ast::Var& v) {
    auto* ent = symtab.lookup(v.name);
    if (!ent) {
        error(v.line, "Undeclared variable '" + v.name + "'");
        v.ty = ast::Type(ast::BasicType::ERROR);
        return;
    }

    // base type (might be array)
    ast::Type base = ent->type;
    v.ty = base;

    /*───────────── Array-specific checks ─────────────*/
    if (!v.indices.empty()) {
        // Too many indices is an error
        if (v.indices.size() > base.dims.size()) {
            error(v.line,
                  "Too many indices for array '" + v.name +
                  "' (expected at most " + std::to_string(base.dims.size()) +
                  ", got " + std::to_string(v.indices.size()) + ")");
            v.ty = ast::Type(ast::BasicType::ERROR);
            return;
        }

        // Evaluate and check each index
        for (size_t i = 0; i < v.indices.size(); ++i) {
            auto& idx = v.indices[i];
            idx->accept(*this);
            if (idx->ty.kind != ast::BasicType::Int) {
                error(v.line, "Array index must be int in '" + v.name + "', index #" + std::to_string(i));
                v.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
        }
        // Compute remaining dimensions after indexing
        std::vector<int> remainingDims(base.dims.begin() + v.indices.size(), base.dims.end());
        v.ty = ast::Type(base.kind);
        v.ty.dims = remainingDims;
    }
}


// Visit literals
void SemanticAnalyzer::visit(ast::IntLit& e) { e.ty = ast::Type(ast::BasicType::Int); }
void SemanticAnalyzer::visit(ast::RealLit& e) { e.ty = ast::Type(ast::BasicType::Float); }
void SemanticAnalyzer::visit(ast::StringLit& e) { e.ty = ast::Type(ast::BasicType::String); }
void SemanticAnalyzer::visit(ast::BoolLit& e) { e.ty = ast::Type(ast::BasicType::Bool); }
void SemanticAnalyzer::visit(ast::CharLit& e) { e.ty = ast::Type(ast::BasicType::Char); }

// Visit unary expressions
void SemanticAnalyzer::visit(ast::Unary& u) {
    u.rhs->accept(*this);

    // Check for ERROR type before proceeding
    if (u.rhs->ty.kind == ast::BasicType::ERROR) {
        u.ty = ast::Type(ast::BasicType::ERROR);  // Mark as error
        return;
    }

    switch (u.op) {
        case ast::Op::Minus:
            if (u.rhs->ty.kind != ast::BasicType::Char && u.rhs->ty.kind != ast::BasicType::Int && u.rhs->ty.kind != ast::BasicType::Float && u.rhs->ty.kind != ast::BasicType::Double) {
                error(u.line, "Unary \'-\' requires int, char, float, or double!");
                u.ty = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
                return;
            }
            u.ty = u.rhs->ty;
            break;
        case ast::Op::Not:
            if (u.rhs->ty.kind != ast::BasicType::Bool) {
                error(u.line, "Unary \'!\' requires bool!");
                u.ty = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
                return;
            }
            u.ty = ast::Type(ast::BasicType::Bool);
            break;
        default:
            error(u.line, "Unknown unary operator");
            u.ty = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
            break;
    }
}

// Visit binary expressions
void SemanticAnalyzer::visit(ast::Binary& b) {
    b.lhs->accept(*this);
    b.rhs->accept(*this);
    auto charIntFloatDoubleBool = [](const std::unique_ptr<ast::Expr>& e1, const std::unique_ptr<ast::Expr>& e2)->bool{
        return (e1->ty.kind == ast::BasicType::Char || e1->ty.kind == ast::BasicType::Int || e1->ty.kind == ast::BasicType::Float || e1->ty.kind == ast::BasicType::Double || e1->ty.kind == ast::BasicType::Bool) &&
               (e2->ty.kind == ast::BasicType::Char || e2->ty.kind == ast::BasicType::Int || e2->ty.kind == ast::BasicType::Float || e2->ty.kind == ast::BasicType::Double || e2->ty.kind == ast::BasicType::Bool);
    };

    auto isBool = [](const std::unique_ptr<ast::Expr>& e1, const std::unique_ptr<ast::Expr>& e2)->bool{
        return (e1->ty.kind == ast::BasicType::Bool) && (e2->ty.kind == ast::BasicType::Bool);
    };

    // Check for ERROR type before proceeding
    if (b.lhs->ty.kind == ast::BasicType::ERROR || b.rhs->ty.kind == ast::BasicType::ERROR) {
        b.ty = ast::Type(ast::BasicType::ERROR);
        return;
    }

    switch (b.op) {
        case ast::Op::Plus: {
            if (b.rhs->ty == b.lhs->ty) {
                b.ty = b.lhs->ty;
            } 
            else{
                error(b.line, "Binary '+' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Minus:{
            if (b.rhs->ty == b.lhs->ty) {
                if (!charIntFloatDoubleBool(b.lhs, b.rhs)) {
                    error(b.line, "Binary '-' requires char, int, float, double or bool!");
                    b.ty = ast::Type(ast::BasicType::ERROR);
                    return;
                }
                b.ty = b.lhs->ty;
            } 
            else{
                error(b.line, "Binary '-' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Mul:{
            if (b.rhs->ty == b.lhs->ty) {
                if (!charIntFloatDoubleBool(b.lhs, b.rhs)) {
                    error(b.line, "Binary '*' requires char, int, float, double or bool!");
                    b.ty = ast::Type(ast::BasicType::ERROR);
                    return;
                }
                b.ty = b.lhs->ty;
            } 
            else{
                error(b.line, "Binary '*' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Div: {
            if (b.rhs->ty == b.lhs->ty) {
                if (!charIntFloatDoubleBool(b.lhs, b.rhs)) {
                    error(b.line, "Binary '/' requires char, int, float, double or bool!");
                    b.ty = ast::Type(ast::BasicType::ERROR);
                    return;
                }
                b.ty = b.lhs->ty;
            } 
            else{
                error(b.line, "Binary '/' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Mod:{
            if (b.lhs->ty.kind != ast::BasicType::Int || b.rhs->ty.kind != ast::BasicType::Int) {
                error(b.line, "Binary '%' requires int!");
                b.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
            b.ty = ast::Type(ast::BasicType::Int);
            break;
        }
        case ast::Op::Less:{
            if (b.rhs->ty == b.lhs->ty) {
                b.ty = ast::BasicType::Bool;
            } 
            else{
                error(b.line, "Binary '<' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::LessEq:{
            if (b.rhs->ty == b.lhs->ty) {
                b.ty = ast::BasicType::Bool;
            } 
            else{
                error(b.line, "Binary '<=' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Greater:{
            if (b.rhs->ty == b.lhs->ty) {
                b.ty = ast::BasicType::Bool;
            } 
            else{
                error(b.line, "Binary '>' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::GreaterEq:{
            if (b.rhs->ty == b.lhs->ty) {
                b.ty = ast::BasicType::Bool;
            } 
            else{
                error(b.line, "Binary '>=' requires same types!");
                b.ty = ast::Type(ast::BasicType::ERROR);
            }
            break;
        }
        case ast::Op::Equal:{
            if (!(b.lhs->ty == b.rhs->ty)) {
                error(b.line, "Binary '==' requires same type!");
                b.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
            b.ty = ast::Type(ast::BasicType::Bool);
            break;
        }
        case ast::Op::NotEqual:{
            if (!(b.lhs->ty == b.rhs->ty)) {
                error(b.line, "Binary '!=' requires same type!");
                b.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
            b.ty = ast::Type(ast::BasicType::Bool);
            break;
        }
        case ast::Op::And:{
            if (!isBool(b.lhs, b.rhs)) {
                error(b.line, "Binary '&&' requires bool!");
                b.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
            b.ty = ast::Type(ast::BasicType::Bool);
            break;
        }
        case ast::Op::Or:{
            if (!isBool(b.lhs, b.rhs)) {
                error(b.line, "Binary '||' requires bool!");
                b.ty = ast::Type(ast::BasicType::ERROR);
                return;
            }
            b.ty = ast::Type(ast::BasicType::Bool);
            break;
        }
        default:
            // Additional binary operators to implement...
            error(b.line, "Operator not implemented");
            b.ty = ast::Type(ast::BasicType::ERROR);
            break;
    }
}

// Visit postfix (++/--)
void SemanticAnalyzer::visit(ast::Postfix& p) {
    p.operand->accept(*this);

    // Check for ERROR type before proceeding
    if (p.operand->ty.kind == ast::BasicType::ERROR) {
        error(p.line, "Invalid expression with ERROR type in postfix operation");
        p.ty = ast::Type(ast::BasicType::ERROR);  // Mark as error
        return;
    }

    if (p.operand->ty.kind == ast::BasicType::Int || p.operand->ty.kind == ast::BasicType::Char || p.operand->ty.kind == ast::BasicType::Float || p.operand->ty.kind == ast::BasicType::Double) {
        if (p.op == ast::Op::Inc || p.op == ast::Op::Dec) {
            p.ty = p.operand->ty;
        } else {
            error(p.line, "Postfix operator not applicable to type, only \'++\' and \'--\' are allowed");
            p.ty = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
            return;
        }
    } else {
        error(p.line, "Postfix operator is \'" + p.operand->ty.toString() + "\' type, and that is not applicable to type.");
        p.ty = ast::Type(ast::BasicType::ERROR);  // Set to ERROR for error handling
        return;
    }
    p.ty = p.operand->ty;
}

// Visit function call
void SemanticAnalyzer::visit(ast::Call& c) {
    for (auto& arg : c.args)
        arg->accept(*this);

    // Lookup function symbol
    auto* ent = symtab.lookup(c.callee);
    if (!ent || !ent->isFunc) {
        error(c.line, "Undeclared function '" + c.callee + "'");
        c.ty = ast::Type(ast::BasicType::ERROR);
        return;
    }
    
    // Check parameter count and types
    if (ent->paramTypes && c.args.size() != ent->paramTypes->size()) {
        error(c.line, "Parameter count mismatch in call to '" + c.callee + "'");
    } else if (ent->paramTypes) {
        for (size_t i = 0; i < c.args.size(); ++i)
            if (!(c.args[i]->ty == (*ent->paramTypes)[i]))
                error(c.line, "Parameter type mismatch in call to '" + c.callee + "'");
    }
    
    // Set call expression type to function's return type
    c.ty = ent->returnType.value_or(ast::Type(ast::BasicType::Void));
    
    // If return type is error, report
    if (c.ty.kind == ast::BasicType::ERROR) {
        error(c.line, "Function '" + c.callee + "' has error return type");
    }
}

// Visit print statement
void SemanticAnalyzer::visit(ast::Print& s) {
    s.expr->accept(*this);
    if (s.expr->ty.kind == ast::BasicType::ERROR || s.expr->ty.kind == ast::BasicType::Void) {
        error(s.line, "Invalid argument type in print statement");
        return;
    }
}

// Visit println statement
void SemanticAnalyzer::visit(ast::Println& s) {
    s.expr->accept(*this);
    if (s.expr->ty.kind == ast::BasicType::ERROR || s.expr->ty.kind == ast::BasicType::Void) {
        error(s.line, "Invalid argument type in println statement");
        return;
    }
}

// Visit read statement
void SemanticAnalyzer::visit(ast::Read& s) {
    s.var->accept(*this);
    if (s.var->ty.kind == ast::BasicType::ERROR || s.var->ty.kind == ast::BasicType::Void) {
        error(s.line, "Invalid identifier type in read statement");
        return;
    }
}

// Visit block statement
void SemanticAnalyzer::visit(ast::Block& b) {
    bool merged = false;
    if (skipBlockScopeOnce > 0) {
        --skipBlockScopeOnce;
        merged = true;
    }
    
    if (!merged) symtab.enterScope();

    for (auto& stmt : b.stmts) {
        stmt->accept(*this);
    }

    symtab.hasErrors = errors.size() > 0;
    if (!merged) symtab.exitScope();
}

// Visit declaration list
void SemanticAnalyzer::visit(ast::DeclList& dl) {
    for (auto& decl : dl.decls) {
        decl->accept(*this);
    }
}

// Visit function declaration
void SemanticAnalyzer::visit(ast::FuncDecl& fd) {
    // Add function to symbol table first so recursion works
    SymEntry funcEntry;
    funcEntry.name = fd.name;
    funcEntry.isFunc = true;
    funcEntry.returnType = fd.returnType;
    funcEntry.type = fd.returnType;  // record return type as entry type
    
    // Process parameter types
    std::vector<ast::Type> paramTypes;
    for (auto& param : fd.params) {
        paramTypes.push_back(param->varType);
    }
    funcEntry.paramTypes = paramTypes;
    
    if (!symtab.insert(funcEntry)) {
        error(fd.line, "Redefinition of function '" + fd.name + "'");
        // Don't proceed with analyzing the body if redefinition error
        return; 
    }
    
    // --- Start analyzing function body ---
    currentFunctionReturnType = fd.returnType;

    // Enter function scope
    symtab.enterScope();
    
    // Process parameters (add them to the function's scope)
    for (auto& param : fd.params) {
        param->accept(*this);
    }
    
    // Process function body (cast to Block to access statements)
    ++skipBlockScopeOnce;
    if (fd.body) {
        fd.body->accept(*this);
    }
    
    // --- Finish analyzing function body ---

    // Check if non-void function has at least one return path
    if (fd.returnType.kind != ast::BasicType::Void) {
        // Cast body to Block to get access to the statements
        if (auto* block = dynamic_cast<ast::Block*>(fd.body.get())) {
            if (!allPathsReturn(block->stmts)) {
                warning(fd.line, "Non-void function '" + fd.name + "' might not return on all paths.");
            }
        }
    }

    // Exit function scope
    symtab.hasErrors = errors.size() > 0;
    symtab.exitScope();

    // Restore outer context
    currentFunctionReturnType = std::nullopt; // restore previous context
}

// Visit range expression
void SemanticAnalyzer::visit(ast::RangeExpr& r) {
    r.start->accept(*this);
    r.end->accept(*this);
    // Both bounds must be integers
    if (r.start->ty.kind != ast::BasicType::Int || r.end->ty.kind != ast::BasicType::Int) {
        error(r.line, "Range bounds must be integers");
        r.ty = ast::Type(ast::BasicType::ERROR);
        return;
    }
    // Set expression type (unused outside foreach)
    r.ty = ast::Type(ast::BasicType::Int);
}

// Record an error message
void SemanticAnalyzer::error(int line, const std::string& msg) {
    errors.push_back("line " + std::to_string(line) + ": " + msg);
}

// Record a warning message
void SemanticAnalyzer::warning(int line, const std::string& msg) {
    warnings.push_back("line " + std::to_string(line) + ": " + msg);
}

// Basic constant evaluator
std::optional<ConstValue> evalConstExpr(ast::Expr* e) {
    if (auto lit = dynamic_cast<ast::IntLit*>(e))
        return lit->value;
    if (auto lit = dynamic_cast<ast::RealLit*>(e))
        return lit->value;
    if (auto lit = dynamic_cast<ast::StringLit*>(e))
        return lit->value;
    if (auto lit = dynamic_cast<ast::BoolLit*>(e))
        return lit->value;
    if (auto lit = dynamic_cast<ast::CharLit*>(e))
        return lit->value;
    return std::nullopt;
}
