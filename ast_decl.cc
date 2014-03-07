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

Decl::Decl(Identifier *id) : Node(*id->GetLocation())
{
    Assert(id != NULL);
    (id_ = id)->SetParent(this);

    return;
}

Identifier *Decl::get_id(void)
{
    return id_;
}

std::ostream& operator<<(std::ostream& out, Decl *d)
{
    return out << d->id_;
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n)
{
    Assert(n != NULL && t != NULL);
    (type_ = t)->SetParent(this);

    return;
}

void VarDecl::DoCheck(void)
{
    type_->Check();

    return;
}

Type *VarDecl::get_type(void)
{
    return type_;
}

void ClassDecl::MergeSymbolTable(ClassDecl *base)
{
    // (1) Conflicting declaration check
    Iterator<Decl*> iter = base->get_sym_table()->GetIterator();
    Decl *d = iter.GetNextValue();
    while (d != NULL) {
        char *name = d->get_id()->get_name();
        Decl *nd = sym_table_->Lookup(name);
        if (nd == NULL) {
            sym_table_->Enter(name, d);
        } else if (dynamic_cast<VarDecl*>(nd) != NULL ||
                   dynamic_cast<VarDecl*>(d) != NULL) {
			sym_table_->Enter(name, d); // override with decl in superclass
            ReportError::DeclConflict(nd, d);
        }else // Type checking for function override
		{
			FnDecl * fnBase = dynamic_cast<FnDecl*>(d);
			FnDecl * fnChild = dynamic_cast<FnDecl*>(nd);
			// self check first then type checking (avoid cascading errors)
			fnChild->Check(); 
			// check return type and formals
			if(!fnChild->IsSigEquivalentTo(fnBase))
			{
				ReportError::OverrideMismatch(fnChild);
				// replace it with definition in superclass
				sym_table_->Enter(name, d);
			}
		}

		d = iter.GetNextValue();
    }

    return;
}

void ClassDecl::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < members->NumElements(); i++) {
        Decl *newdecl = members->Nth(i);
        char *name = newdecl->get_id()->get_name();
        Decl *olddecl = sym_table_->Lookup(name);
        if (olddecl == NULL) {
            sym_table_->Enter(name, newdecl);
        } else {
            ReportError::DeclConflict(newdecl, olddecl);
        }
    }
    if (extends != NULL) {
        ClassDecl *base = GetClass(extends);
        if (base == NULL) {
            ReportError::IdentifierNotDeclared(extends->get_id(),
                                               LookingForClass);
        } else {
            base->Check(); // construct sym table for base class
            MergeSymbolTable(base);
        }
    }

    // Check should always follow construction of the symbol table,
    // otherwise any forward declaration will fail.
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->Check();
    }

    // (3) Incomplete implementation check
    for (int i = 0; i < implements->NumElements(); i++)
    {
        NamedType *nt = implements->Nth(i);
        InterfaceDecl *intd = parent->GetInterface(nt);
		intd->Check(); // construct sym table for interface
        if (intd == NULL) {
            ReportError::IdentifierNotDeclared(nt->get_id(),
                                               LookingForInterface);
        } else {
            Hashtable<Decl*> *sym_impl = intd->get_sym_table();
            Iterator<Decl*> iter = sym_impl->GetIterator();
            Decl *decl = iter.GetNextValue();
			bool hideError = false;
            while (decl != NULL) {
                char *name = decl->get_id()->get_name();
                FnDecl *extDecl = GetMemberFn(name);
                if (extDecl == NULL) {
					if(!hideError)
					{
						ReportError::InterfaceNotImplemented(this, nt);
						hideError = true;
					}
                } else {
					FnDecl * implDecl = dynamic_cast<FnDecl*>(decl);
					if(!extDecl->IsSigEquivalentTo(implDecl))
					{
						ReportError::OverrideMismatch(extDecl);
						// keep the signiture from Interface
						sym_table_->Enter(name, decl);
					}
                }
				decl = iter.GetNextValue();
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
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Hashtable<Decl*> *ClassDecl::get_sym_table(void)
{
    return sym_table_;
}

ClassDecl *ClassDecl::GetCurrentClass(void)
{
    return this;
}

VarDecl *ClassDecl::GetVar(Identifier *id)
{
	return GetMemberVar(id->name());
}

VarDecl *ClassDecl::GetMemberVar(char *name)
{
    return dynamic_cast<VarDecl*>(sym_table_->Lookup(name));
}

FnDecl *ClassDecl::GetMemberFn(char *name)
{
    return dynamic_cast<FnDecl*>(sym_table_->Lookup(name));
}

FnDecl *ClassDecl::GetFn(Identifier *id)
{
	return GetMemberFn(id->name());
}

bool ClassDecl::IsTypeCompatibleWith(NamedType *baseClass)
{
	// check extends and impl list
	if(extends)
	{
		if(extends->IsCompatibleWith(baseClass)) // recursively check all ancestor classes
			return true;
	}

    for (int i = 0; i < implements->NumElements(); i++)
	{
		if(implements->Nth(i)->IsEquivalentTo(baseClass))
			return true;
	}

	return false;
}

void InterfaceDecl::DoCheck(void)
{
    // (1) Conflicting declaration check
    for (int i = 0; i < members_->NumElements(); i++) {
        Decl *newdecl = members_->Nth(i);
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
    for (int i = 0; i < members_->NumElements(); i++) {
        members_->Nth(i)->Check();
    }

    return;
}

InterfaceDecl::InterfaceDecl(Identifier *name, List<Decl*> *members) :
    Decl(name)
{
    Assert(name != NULL && members != NULL);
    (members_ = members)->SetParentAll(this);
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Hashtable<Decl*> *InterfaceDecl::get_sym_table(void)
{
    return sym_table_;
}


void FnDecl::DoCheck(void)
{
    returnType_->Check();

    // (1) Conflicting declaration check
    for (int i = 0; i < formals_->NumElements(); i++) {
        Decl *newdecl = formals_->Nth(i);
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
    for (int i = 0; i < formals_->NumElements(); i++) {
        formals_->Nth(i)->Check();
    }

    if (body_ != NULL) {
        body_->Check();
    }

    return;
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n)
{
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType_ = r)->SetParent(this);
    (formals_ = d)->SetParentAll(this);
    body_ = NULL;
    sym_table_ = new Hashtable<Decl*>;

    return;
}

Type *FnDecl::get_return_type(void)
{
    return returnType_;
}

List<VarDecl*> *FnDecl::get_formals(void)
{
	return formals_;
}

void FnDecl::SetFunctionBody(Stmt *b)
{
    (body_ = b)->SetParent(this);

    return;
}

VarDecl *FnDecl::GetVar(Identifier *id)
{
    char *name = id->get_name();
    VarDecl *decl = dynamic_cast<VarDecl*>(sym_table_->Lookup(name));
    if (decl == NULL) {
        decl = parent->GetVar(id);
    }

    return decl;
}

FnDecl *FnDecl::GetCurrentFn(void)
{
	return this;
}

bool FnDecl::IsSigEquivalentTo(FnDecl *other)
{
	// check return type
	if(returnType_->IsEquivalentTo(other->get_return_type()))
	{
		// check formals
		List<VarDecl*> *otherFmls = other->get_formals();
		if(formals_->NumElements() != otherFmls->NumElements())
			return false;
		else
		{
			for(int i = 0; i<formals_->NumElements(); ++i)
			{
				VarDecl * thisVar = formals_->Nth(i);
				VarDecl * otherVar = otherFmls->Nth(i);
				if(!this->get_type()->IsEquivalentTo(otherVar->get_type()))
				{
					return false;
				}
			}
		}
	}else
		return false;

	return true;
}
