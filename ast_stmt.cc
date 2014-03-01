/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"

Program::Program(List<Decl*> *d)
{
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
    sym_ = new Hashtable<Decl*>();
}

void Program::DoCheck(void)
{
    for (int i = 0; i < decls->NumElements(); i++) {
        Decl *newdec = decls->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(id);
        if (olddec == NULL) {
            sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Check();
    }

    return;
}

ClassDecl *Program::GetClass(NamedType *t)
{
    Decl *dec = sym_->Lookup(t->get_id()->get_name());
    ClassDecl *olddec = dynamic_cast<ClassDecl*>(dec);
    if (olddec != NULL) {
        olddec->Check();
    }

    return olddec;
}

FnDecl *Program::GetFn(char *name)
{
    Decl *dec = sym_->Lookup(name);
    FnDecl *olddec = dynamic_cast<FnDecl*>(dec);
    if (olddec != NULL) {
        olddec->Check();
    }

    return olddec;
}

VarDecl *Program::GetVar(char *name)
{
    Decl *dec = sym_->Lookup(name);
    VarDecl *olddec = dynamic_cast<VarDecl*>(dec);
    if (olddec != NULL) {
        olddec->Check();
    }

    return olddec;
}

InterfaceDecl *Program::GetInterface(char *name)
{
    Decl *dec = sym_->Lookup(name);
    InterfaceDecl *olddec = dynamic_cast<InterfaceDecl*>(dec);
    if (olddec != NULL) {
        olddec->Check();
    }

    return olddec;
}

Stmt::Stmt(void) : Node()
{
    return;
}

Stmt::Stmt(yyltype loc) : Node(loc)
{
    return;
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s)
{
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
    sym_ = new Hashtable<Decl*>();

    return;
}

void StmtBlock::DoCheck(void)
{
    for (int i = 0; i < decls->NumElements(); i++) {
        Decl *newdec = decls->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(id);
        if (olddec == NULL) {
            sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Check();
    }
    for (int i = 0; i < stmts->NumElements(); i++) {
        stmts->Nth(i)->Check();
    }

    return;
}

VarDecl *StmtBlock::GetVar(char *name)
{
    Decl *dec = sym_->Lookup(name);
    VarDecl *olddec = dynamic_cast<VarDecl*>(dec);
    if (olddec != NULL) {
        olddec->Check();
    } else {
        olddec = parent->GetVar(name);
    }

    return olddec;
}


ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b)
{
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this);
    (body=b)->SetParent(this);
}

void ConditionalStmt::DoCheck(void)
{
    test->Check();
    body->Check();

    return;
}

LoopStmt::LoopStmt(Expr *testExpr, Stmt *body) :
    ConditionalStmt(testExpr, body)
{
    return;
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b)
{
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::DoCheck(void)
{
    init->Check();
    test->Check();
    step->Check();
    body->Check();

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
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::DoCheck(void)
{
    test->Check();
    body->Check();
    if (elseBody != NULL) {
        elseBody->Check();
    }

    return;
}

BreakStmt::BreakStmt(yyltype loc) : Stmt(loc)
{
    return;
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc)
{
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::DoCheck(void)
{
    expr->Check();

    return;
}

PrintStmt::PrintStmt(List<Expr*> *a)
{
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::DoCheck(void)
{
    for (int i = 0; i < args->NumElements(); i++) {
        args->Nth(i)->Check();
    }

    return;
}
