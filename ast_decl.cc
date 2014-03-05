/**** ast_decl.cc - ASTs of declarations ******************************
 *
 * Copyright © 2014 Shuang Feng, Hongjiu Zhang
 *
 * All rights reserved.                                              */

#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
#include "hashtable.h"
#include "list.h"

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

void ClassDecl::MergeSymTable(ClassDecl base)
{
    // (1) Conflicting declaration check
    Iterator<Decl*> iter = base->sym_->GetIterator();
    Decl *d = iter.GetNextValue();
    while (d != NULL) {
        char *name = d->get_id()->get_name();
        Decl *nd = sym_->Lookup(name);
        if (nd == NULL) {
            sym_->Enter(name, newdec);
        } else if (dynamic_cast<VarDecl*>(nd) != NULL) {
            ReportError::DeclConflict(newdec, olddec);
        } // TODO: Type checking for function override.
    }

    return;
}

void ClassDecl::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < members->NumElements(); i++) {
        Decl *newdec = members->Nth(i);
        char *name = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(name);
        newdec->Check();
        if (olddec == NULL) {
            sym_->Enter(name, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
    }
    if (extends != NULL) {
        ClassDecl *base = GetClass(extends);
        if (base == NULL) {
            ReportError::IdentifierNotDeclared(extends->get_id(),
                                               LookingForClass);
        } else {
            base->Check();
            MergeSymTable(base);
        }
    }

    for (int i = 0; i < implements->NumElements(); i++)
    {
        InterfaceDecl *intd = parent->GetInterface(implements->Nth(i)); // find interface from the scope of the program
        if (intd == NULL) {
            ReportError::IdentifierNotDeclared(implements->Nth(i)->get_id(), LookingForInterface);
        } else {
            Hashtable<Decl*> *sym_impl = intd->sym_;
            Iterator<Decl*> iter = sym_impl->GetIterator();	
            Decl* decl = NULL;
            while((decl = iter.GetNextValue()))
            {
                // PROBLEM:: Should look up only function declarations
                FnDecl * extDecl = GetMemberFn(decl->get_id()->get_name());
                if(extDecl == NULL)
                {
                    ReportError::InterfaceNotImplemented(this, implements->Nth(i));	
                    break;
                }else
                {
                    // TODO: type checking
                }
            }
        }
    }
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

ClassDecl *ClassDecl::GetCurrentClass(void)
{
    return this;
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
    // (1) Conflicting declaration check
    for (int i = 0; i < members->NumElements(); i++) {
        Decl *newdec = members->Nth(i);
        char *name = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(name);
        newdec->Check();
        if (olddec == NULL) {
            sym_->Enter(name, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
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
    returnType->Check();

    // (1) Conflicting declaration check
    for (int i = 0; i < formals->NumElements(); i++) {
        Decl *newdec = formals->Nth(i);
        char *name = newdec->get_id()->get_name();
        Decl *olddec = sym_->Lookup(name);
        newdec->Check();
        if (olddec == NULL) {
            sym_->Enter(name, newdec);
        } else {
            ReportError::DeclConflict(newdec, olddec);
        }
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
