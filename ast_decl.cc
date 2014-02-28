/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */

#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"

Decl::Decl(Identifier *n) : Node(*n->GetLocation())
{
    Assert(n != NULL);
    (id=n)->SetParent(this);

    return;
}

Identifier *Decl::get_id(void)
{
    return this->id;
}

std::ostream& operator<<(std::ostream& out, Decl *d)
{
    return out << d->id;
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n)
{
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);

    return;
}

void VarDecl::DoCheck(void)
{
    this->type->DoCheck(void);

    return;
}


ClassDecl::ClassDecl(Identifier *n, NamedType *ex,
                     List<NamedType*> *imp, List<Decl*> *m) :
    Decl(n)
{
    // Extends can be NULL. Implements and members may be empty lists,
    // but not NULL.
    Assert(n != NULL && imp != NULL && m != NULL);
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    sym_ = new Hashtable<Decl*>;
    for (int i = 0; i < m->NumElements(); i++) {
        Decl *newdec = m->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = this->sym_->Lookup(id);
        if (olddec == NULL) {
            this->sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }

    return;
}

void ClassDecl::DoCheck(void)
{
    if (this->extends != NULL) {
        this->extends->Check();
    }

    // TODO: Check declaration conflict for extend

    for (int i = 0; i < this->implements->NumElements(); i++) {
        this->implements->Nth(i)->Check();
    }

    // TODO: Check declaration conflict for implements

    // TODO: Do implementation completion check

    return;
}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n)
{
    Assert(n != NULL && m != NULL);
    (members = m)->SetParentAll(this);
    sym_ = new Hashtable<Decl*>;
    for (int i = 0; i < m->NumElements(); i++) {
        Decl *newdec = m->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = this->sym_->Lookup(id);
        if (olddec == NULL) {
            this->sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }

    return;
}

void InterfaceDecl::DoCheck(void)
{
    for (int i = 0; i < this->members->NumElements(); i++) {
        this->members->Nth(i)->DoCheck();
    }

    return;
}


FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n)
{
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType = r)->SetParent(this);
    (formals = d)->SetParentAll(this);
    body = NULL;
    sym_ = new Hashtable<Decl*>;
    for (int i = 0; i < d->NumElements(); i++) {
        Decl *newdec = d->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = this->sym_->Lookup(id);
        if (olddec == NULL) {
            this->sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }

    return;
}

void FnDecl::SetFunctionBody(Stmt *b)
{
    (body = b)->SetParent(this);

    return;
}

void FnDecl::DoCheck(void)
{
    this->returnType->DoCheck();
    for (int i = 0; i < this->formals->NumElements(); i++) {
        this->formals->Nth(i)->Check();
    }
    if (body != NULL) {
        this->body->Check();
    }

    return;
}
