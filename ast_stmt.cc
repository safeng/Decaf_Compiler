/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */

#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"

Program::Program(List<Decl*> *dec)
{
    Assert(dec != NULL);
    (decls_ = dec)->set_parent_all(this);
    sym_table_ = new Hashtable<Decl*>();
}

void Program::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < decls_->NumElements(); i++) {
        Decl *newdecl = decls_->Nth(i);
        char *name = newdecl->id()->name();
        Decl *olddecl = sym_table_->Lookup(name);
        if (olddecl == NULL) {
            sym_table_->Enter(name, newdecl);
        } else {
            ReportError::DeclConflict(newdecl, olddecl);
        }
    }

    // Check should always follow construction of the symbol table,
    // otherwise any forward declaration will fail.
    for (int i = 0; i < decls_->NumElements(); i++) {
        decls_->Nth(i)->Check();
    }

    return;
}

ClassDecl *Program::GetClass(NamedType *t)
{
    Decl *dec = sym_table_->Lookup(t->id()->name());
    ClassDecl *olddecl = dynamic_cast<ClassDecl*>(dec);

    return olddecl;
}

FnDecl *Program::GetFn(Identifier *id)
{
    Decl *dec = sym_table_->Lookup(id->name());
    FnDecl *olddecl = dynamic_cast<FnDecl*>(dec);

    return olddecl;
}

VarDecl *Program::GetVar(Identifier *id)
{
    Decl *dec = sym_table_->Lookup(id->name());
    VarDecl *olddecl = dynamic_cast<VarDecl*>(dec);

    return olddecl;
}

InterfaceDecl *Program::GetInterface(NamedType *t)
{
    char *str = t->id()->name();
    Decl *dec = sym_table_->Lookup(str);
    InterfaceDecl *olddecl = dynamic_cast<InterfaceDecl*>(dec);

    return olddecl;
}

Stmt::Stmt(void) : Node()
{
    return;
}

Stmt::Stmt(yyltype loc) : Node(loc)
{
    return;
}

Stmt *Stmt::GetContextStmt(void)
{
    Stmt *cnt = this;
    if (dynamic_cast<Stmt*>(parent()) != NULL) {
        cnt = dynamic_cast<Stmt*>(parent())->GetContextStmt();
    }

    return cnt;
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s)
{
    Assert(d != NULL && s != NULL);
    (decls=d)->set_parent_all(this);
    (stmts=s)->set_parent_all(this);
    sym_ = new Hashtable<Decl*>();

    return;
}

void StmtBlock::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < decls->NumElements(); i++) {
        Decl *newdecl = decls->Nth(i);
        char *name = newdecl->id()->name();
        Decl *olddecl = sym_->Lookup(name);
        if (olddecl == NULL) {
            sym_->Enter(name, newdecl);
        } else {
            ReportError::DeclConflict(newdecl, olddecl);
        }
    }

    // Check should always follow construction of the symbol table,
    // otherwise any forward declaration will fail.
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Check();
    }

    for (int i = 0; i < stmts->NumElements(); i++) {
        stmts->Nth(i)->Check();
    }

    return;
}

VarDecl *StmtBlock::GetVar(Identifier *i)
{
    Decl *decl = sym_->Lookup(i->name());
    VarDecl *olddecl = dynamic_cast<VarDecl*>(decl);
    if (olddecl != NULL) {
        olddecl->Check();
    } else {
        olddecl = parent()->GetVar(i);
    }

    return olddecl;
}


ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b)
{
    Assert(t != NULL && b != NULL);
    (test=t)->set_parent(this);
    (body=b)->set_parent(this);
}

void ConditionalStmt::DoCheck(void)
{
    test->Check();
    body->Check();

    // testExpr must be boolean type
    if (test->type() != Type::boolType &&
        test->type() != Type::errorType) {
        ReportError::TestNotBoolean(test);
    }

    return;
}

LoopStmt::LoopStmt(Expr *testExpr, Stmt *body) :
    ConditionalStmt(testExpr, body)
{
    return;
}

Stmt *LoopStmt::GetContextStmt(void)
{
    return this;
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b)
{
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->set_parent(this);
    (step=s)->set_parent(this);
}

void ForStmt::DoCheck(void)
{
    init->Check();
    test->Check();
    step->Check();
    body->Check();
    ConditionalStmt::DoCheck(); // check non-boolean test expr

    return;
}

WhileStmt::WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body)
{
    return;
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb)
{
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->set_parent(this);
}

void IfStmt::DoCheck(void)
{
    test->Check();
    body->Check();
    if (elseBody != NULL) {
        elseBody->Check();
    }
    ConditionalStmt::DoCheck(); // check non-boolean test expr
    return;
}

BreakStmt::BreakStmt(yyltype loc) : Stmt(loc)
{
    return;
}

void BreakStmt::DoCheck(void)
{
    Stmt *cnt = GetContextStmt();
    if(dynamic_cast<LoopStmt*>(cnt) == NULL) // not a loop stmt
        ReportError::BreakOutsideLoop(this);

    return;
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc)
{
    Assert(e != NULL);
    (expr=e)->set_parent(this);
}

void ReturnStmt::DoCheck(void)
{
    expr->Check();
    // Try to find function declaration
    FnDecl *fnd = GetCurrentFn();
    if (fnd != NULL) {
        // check type
        Type *rType = expr->type();
        Type *expt = fnd->return_type();
        if (!rType->IsCompatibleWith(expt)) {
            ReportError::ReturnMismatch(this, rType, expt);
        }
    } // return not used in function

    return;
}

PrintStmt::PrintStmt(List<Expr*> *a)
{
    Assert(a != NULL);
    (args=a)->set_parent_all(this);
}

void PrintStmt::DoCheck(void)
{
    for (int i = 0; i < args->NumElements(); i++) {
        args->Nth(i)->Check();
    }

    // type checking. Print can only print string, int or bool
    for (int i = 0; i < args->NumElements(); i++) {
        Expr * arg = args->Nth(i);	
        Type * argType = arg->type();
        if (!argType->IsEquivalentTo(Type::stringType) &&
            !argType->IsEquivalentTo(Type::intType) &&
            !argType->IsEquivalentTo(Type::boolType)) {
            ReportError::PrintArgMismatch(arg, i + 1, argType);
        }
    }

    return;
}
