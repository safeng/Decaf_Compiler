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
    return id;
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
    type->Check();

    return;
}

Type *VarDecl::get_type(void)
{
    return type;
}

void ClassDecl::DoCheck(void)
{
    if (extends != NULL) {
        extends->Check();
        *sym_ = *GetClass(extends)->sym_;
    }
    for (int i = 0; i < members->NumElements(); i++) {
        Decl *newdec = members->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(id);
        if (olddec == NULL) {
            sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    for (int i = 0; i < implements->NumElements(); i++) {
        implements->Nth(i)->Check();
    }

    // TODO: Check declaration conflict for implements

    // TODO: Do implementation completion check

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

    return;
}

VarDecl *ClassDecl::GetMemberVar(char *name)
{
    return dynamic_cast<VarDecl*>(sym_->Lookup(name));
}

FnDecl *ClassDecl::GetMemberFn(char *name)
{
    return dynamic_cast<FnDecl*>(sym_->Lookup(name));
}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n)
{
    Assert(n != NULL && m != NULL);
    (members = m)->SetParentAll(this);
    sym_ = new Hashtable<Decl*>;

    return;
}

void InterfaceDecl::DoCheck(void)
{
    for (int i = 0; i < members->NumElements(); i++) {
        Decl *newdec = members->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(id);
        if (olddec == NULL) {
            sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->Check();
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

    return;
}

void FnDecl::SetFunctionBody(Stmt *b)
{
    (body = b)->SetParent(this);

    return;
}

void FnDecl::DoCheck(void)
{
    for (int i = 0; i < formals->NumElements(); i++) {
        Decl *newdec = formals->Nth(i);
        char *id = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(id);
        if (olddec == NULL) {
            sym_->Enter(id, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    returnType->Check();
    for (int i = 0; i < formals->NumElements(); i++) {
        formals->Nth(i)->Check();
    }
    if (body != NULL) {
        body->Check();
    }

    return;
}

Type *FnDecl::get_return_type(void)
{
    return returnType;
}
