/**** ast_decl.cc - ASTs of declarations *****************************/

#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
#include "hashtable.h"
#include "list.h"

Identifier *Decl::id(void)
{
    return id_;
}

Decl::Decl(Identifier *i) : Node(*i->location())
{
    Assert(i != NULL);
    (id_ = i)->set_parent(this);

    return;
}

std::ostream& operator<<(std::ostream& out, Decl *d)
{
    return out << d->id_;
}


void VarDecl::DoCheck(void)
{
    type_->Check();
    if (!type_->is_valid()) {
        // change type to errorType to avoid cascading errors
        type_ = Type::errorType;
    }
    return;
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n)
{
    Assert(n != NULL && t != NULL);
    (type_ = t)->set_parent(this);

    return;
}

Type *VarDecl::type(void)
{
    return type_;
}

void ClassDecl::MergeSymbolTable(ClassDecl *base)
{
    // (1) Conflicting declaration check
    Iterator<Decl*> iter = base->sym_table()->GetIterator();
    Decl *d = iter.GetNextValue();
    while (d != NULL) {
        char *name = d->id()->name();
        Decl *nd = sym_table_->Lookup(name);
        if (nd == NULL) {
            sym_table_->Enter(name, d);
        } else if (dynamic_cast<VarDecl*>(nd) != NULL ||
                   dynamic_cast<VarDecl*>(d) != NULL) {
            // override with decl in superclass
            sym_table_->Enter(name, d);
            ReportError::DeclConflict(nd, d);
        } else {
            // Type checking for function override
            FnDecl *fnBase = dynamic_cast<FnDecl*>(d);
            FnDecl *fnChild = dynamic_cast<FnDecl*>(nd);
            // Self check first to avoid cascading errors
            //fnChild->Check();
            // check return type and formals
            if (!fnChild->IsSigEquivalentTo(fnBase)) {
                sym_table_->Enter(name, fnBase);
                ReportError::OverrideMismatch(fnChild);
            }
        }
        d = iter.GetNextValue();
    }

    return;
}

void ClassDecl::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < members_->NumElements(); i++) {
        Decl *newdecl = members_->Nth(i);
        char *name = newdecl->id()->name();
        Decl *olddecl = sym_table_->Lookup(name);
        if (olddecl == NULL) {
            sym_table_->Enter(name, newdecl);
        } else {
            ReportError::DeclConflict(newdecl, olddecl);
        }
    }
    if (extends_ != NULL) {
        ClassDecl *base = GetClass(extends_);
        if (base == NULL) {
            ReportError::IdentifierNotDeclared(extends_->id(),
                                               LookingForClass);
        } else {
            base->Check(); // construct sym table for base class
            MergeSymbolTable(base);
        }
    }

    // Check should always follow construction of the symbol table,
    // otherwise any forward declaration will fail.
    for (int i = 0; i < members_->NumElements(); i++) {
        members_->Nth(i)->Check();
    }

    // (3) Incomplete implementation check
    for (int i = 0; i < implements_->NumElements(); i++) {
        NamedType *nt = implements_->Nth(i);
        InterfaceDecl *intd = parent()->GetInterface(nt);
        if (intd == NULL) {
            ReportError::IdentifierNotDeclared(nt->id(),
                                               LookingForInterface);
        } else {
            intd->Check(); // construct sym table for interface
            Hashtable<Decl*> *sym_impl = intd->sym_table();
            Iterator<Decl*> iter = sym_impl->GetIterator();
            Decl *decl = iter.GetNextValue();
            bool hideError = false;
            while (decl != NULL) {
                char *name = decl->id()->name();
                FnDecl *extDecl = GetMemberFn(name);
                if (extDecl == NULL) {
                    if(!hideError) {
                        ReportError::InterfaceNotImplemented(this, nt);
                        hideError = true;
                    }
                } else {
                    FnDecl * implDecl = dynamic_cast<FnDecl*>(decl);
                    if (!extDecl->IsSigEquivalentTo(implDecl)) {
                        ReportError::OverrideMismatch(extDecl);
                        if(!hideError) {
                            ReportError::InterfaceNotImplemented(this, nt);
                            hideError = true;
                        }
                    }
                }
                decl = iter.GetNextValue();
            }
        }
    }
    return;
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ext,
                     List<NamedType*> *impl, List<Decl*> *memb) :
    Decl(n)
{
    // Extends can be NULL. Implements and members may be empty lists,
    // but not NULL.
    Assert(n != NULL && impl != NULL && memb != NULL);
    extends_ = ext;
    if (extends_ != NULL) extends_->set_parent(this);
    (implements_ = impl)->set_parent_all(this);
    (members_ = memb)->set_parent_all(this);
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Hashtable<Decl*> *ClassDecl::sym_table(void)
{
    return sym_table_;
}

ClassDecl *ClassDecl::GetCurrentClass(void)
{
    return this;
}

ClassDecl *ClassDecl::GetClass(NamedType *t)
{
    Decl *d = sym_table_->Lookup(t->id()->name());
    ClassDecl *r = dynamic_cast<ClassDecl*>(d);
    if (d == NULL) {
        r = parent()->GetClass(t); // maybe global scope
    }

    return r;
}

VarDecl *ClassDecl::GetMemberVar(char *n)
{
    return dynamic_cast<VarDecl*>(sym_table_->Lookup(n));
}

VarDecl *ClassDecl::GetVar(Identifier *i)
{
    Decl *d = sym_table_->Lookup(i->name());
    VarDecl *r = dynamic_cast<VarDecl*>(d);
    if (d == NULL) {
        r = parent()->GetVar(i); // maybe global scope
    }

    return r;
}

InterfaceDecl *ClassDecl::GetInterface(NamedType *t)
{
    Decl *d = sym_table_->Lookup(t->id()->name());
    InterfaceDecl *r = dynamic_cast<InterfaceDecl*>(d);
    if (d == NULL) {
        r = parent()->GetInterface(t); // maybe global scope
    }

    return r;
}

FnDecl *ClassDecl::GetMemberFn(char *n)
{
    return dynamic_cast<FnDecl*>(sym_table_->Lookup(n));
}

FnDecl *ClassDecl::GetFn(Identifier *i)
{
    Decl *d = sym_table_->Lookup(i->name());
    FnDecl *r = dynamic_cast<FnDecl*>(d);
    if (d == NULL) {
        r = parent()->GetFn(i); // maybe global scope
    }

    return r;
}

bool ClassDecl::IsTypeCompatibleWith(NamedType *t)
{
    bool comp = false;
    if (extends_ != NULL && GetClass(extends_) != NULL &&
        extends_->IsCompatibleWith(t)) {
        comp = true;
    }
    for (int i = 0; !comp && i < implements_->NumElements(); i++) {
        if (GetInterface(implements_->Nth(i)) != NULL) {
            comp = implements_->Nth(i)->IsEquivalentTo(t);
        }
    }

    return comp;
}

bool ClassDecl::IsSubsetOf(NamedType *t)
{
    bool ss = strcmp(t->id()->name(), id_->name()) == 0;
    if (!ss && extends_ != NULL && GetClass(extends_) != NULL) {
        ss = GetClass(extends_)->IsSubsetOf(t);
    }

    return ss;
}

void InterfaceDecl::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < members_->NumElements(); i++) {
        Decl *newdecl = members_->Nth(i);
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
    for (int i = 0; i < members_->NumElements(); i++) {
        members_->Nth(i)->Check();
    }

    return;
}

InterfaceDecl::InterfaceDecl(Identifier *name, List<Decl*> *members) :
    Decl(name)
{
    Assert(name != NULL && members != NULL);
    (members_ = members)->set_parent_all(this);
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Hashtable<Decl*> *InterfaceDecl::sym_table(void)
{
    return sym_table_;
}

FnDecl *InterfaceDecl::GetMemberFn(char *n)
{
    return dynamic_cast<FnDecl*>(sym_table_->Lookup(n));
}

FnDecl *InterfaceDecl::GetFn(Identifier *i)
{
    FnDecl *memFn = GetMemberFn(i->name());
    if (memFn == NULL) {
        memFn = parent()->GetFn(i); // global function
    }

    return memFn;
}


void FnDecl::DoCheck(void)
{
    return_type_->Check();

    // (1) Conflicting declaration check
    for (int i = 0; i < formals_->NumElements(); i++) {
        Decl *newdecl = formals_->Nth(i);
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
    for (int i = 0; i < formals_->NumElements(); i++) {
        formals_->Nth(i)->Check();
    }

    if (body_ != NULL) {
        body_->Check();
    }

    return;
}

FnDecl *FnDecl::GetCurrentFn(void)
{
    return this;
}

FnDecl::FnDecl(Identifier *n, Type *ret, List<VarDecl*> *form) :
    Decl(n)
{
    Assert(n != NULL && ret != NULL && form != NULL);
    (return_type_ = ret)->set_parent(this);
    (formals_ = form)->set_parent_all(this);
    body_ = NULL;
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Type *FnDecl::return_type(void)
{
    return return_type_;
}

List<VarDecl*> *FnDecl::formals(void)
{
    return formals_;
}

void FnDecl::set_body(Stmt *b)
{
    (body_ = b)->set_parent(this);

    return;
}

ClassDecl *FnDecl::GetClass(NamedType *t)
{
    Decl *d = sym_table_->Lookup(t->id()->name());
    ClassDecl *r = dynamic_cast<ClassDecl*>(d);
    if (d == NULL) {
        r = parent()->GetClass(t); // maybe global scope
    }

    return r;
}

VarDecl *FnDecl::GetVar(Identifier *i)
{
    Decl *d = sym_table_->Lookup(i->name());
    VarDecl *r = dynamic_cast<VarDecl*>(d);
    if (d == NULL) {
        r = parent()->GetVar(i); // maybe global scope
    }

    return r;
}

InterfaceDecl *FnDecl::GetInterface(NamedType *t)
{
    Decl *d = sym_table_->Lookup(t->id()->name());
    InterfaceDecl *r = dynamic_cast<InterfaceDecl*>(d);
    if (d == NULL) {
        r = parent()->GetInterface(t); // maybe global scope
    }

    return r;
}

FnDecl *FnDecl::GetFn(Identifier *i)
{
    Decl *d = sym_table_->Lookup(i->name());
    FnDecl *r = dynamic_cast<FnDecl*>(d);
    if (d == NULL) {
        r = parent()->GetFn(i); // maybe global scope
    }

    return r;
}

bool FnDecl::IsSigEquivalentTo(FnDecl *other)
{
    bool eqv = return_type_->IsEquivalentTo(other->return_type());
    List<VarDecl*> *otherFmls = other->formals();
    eqv = eqv && formals_->NumElements() == otherFmls->NumElements();
    for (int i = 0; eqv && i < formals_->NumElements(); i++) {
        VarDecl *thisVar = formals_->Nth(i);
        VarDecl *otherVar = otherFmls->Nth(i);
        eqv = eqv && thisVar->type()->IsEquivalentTo(otherVar->type());
    }

    return eqv;
}

void FnDecl::CheckCallCompatibility(Identifier *caller,
                                    List<Expr*> *actuals)
{
    // Must ensure that we check node first then check type
    int sizeNeed = formals_->NumElements();
    int sizeProvide = actuals->NumElements();
    if (sizeNeed != sizeProvide) {
        ReportError::NumArgsMismatch(caller, sizeNeed, sizeProvide);
    }
    int i = 0, j = 0;
    while(i < sizeNeed && j < sizeProvide) {
        // check formals and actuals
        Expr *actExpr = actuals->Nth(j);
        Type *actType = actExpr->type();
        Type *formType = formals_->Nth(i)->type();
        if(!actType->IsCompatibleWith(formType)) {
            ReportError::ArgMismatch(actExpr, i + 1, actType, formType);
        }
        ++i; ++j;
    }
}

LengthFn::LengthFn(yyltype loc):
    FnDecl(new Identifier(loc, "length"), Type::intType,
           new List<VarDecl*>)
{
    return;
}
