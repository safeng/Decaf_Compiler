/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"

Program::Program(List<Decl*> *decls)
{
    Assert(decls != NULL);
    (decls = decls)->SetParentAll(this);
    sym_table_ = new Hashtable<Decl*>();
}

void Program::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < decls_->NumElements(); i++) {
        Decl *newdecl = decls_->Nth(i);
        char *name = newdecl->get_id()->get_name();
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
    Decl *dec = sym_table_->Lookup(t->get_id()->get_name());
    ClassDecl *olddecl = dynamic_cast<ClassDecl*>(dec);

    return olddecl;
}

FnDecl *Program::GetFn(Identifier *id)
{
    Decl *dec = sym_table_->Lookup(id->get_name());
    FnDecl *olddecl = dynamic_cast<FnDecl*>(dec);

    return olddecl;
}

VarDecl *Program::GetVar(Identifier *id)
{
    Decl *dec = sym_table_->Lookup(id->get_name());
    VarDecl *olddecl = dynamic_cast<VarDecl*>(dec);

    return olddecl;
}

InterfaceDecl *Program::GetInterface(NamedType *t)
{
    char *str = t->get_id()->get_name();
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
	if(dynamic_cast<Stmt*>(parent) == NULL) // no further higher level
	{
		return this;
	}else
		return parent->GetContextStmt();
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
    // (1) Conflicting declaration check
    for (int i = 0; i < decls->NumElements(); i++) {
        Decl *newdecl = decls->Nth(i);
        char *name = newdecl->get_id()->get_name();
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

VarDecl *StmtBlock::GetVar(Identifier *id)
{
    Decl *decl = sym_->Lookup(id->get_name());
    VarDecl *olddecl = dynamic_cast<VarDecl*>(decl);
    if (olddecl != NULL) {
        olddecl->Check();
    } else {
        olddecl = parent->GetVar(id);
    }

    return olddecl;
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

Stmt *LoopStmt::GetContextStmt(void)
{
	return this;
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

void BreakStmt::DoCheck(void)
{
	Stmt *cnt = GetContextStmt();
	if(dynamic_cast<LoopStmt*>(cnt) == NULL) // not a loop stmt
		ReportError::BreakOutsideLoop(this);
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
