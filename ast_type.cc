/**** ast_type.cc - ASTs for types ***********************************/

#include <string.h>

#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"

/*** class Type ******************************************************/

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error");

Type::Type(void) : Node()
{
    is_valid_ = true;

    return;
}

Type::Type(const char *n)
{
    Assert(n);
    name_ = strdup(n);
    is_valid_ = true;

    return;
}

Type::Type(yyltype loc) : Node(loc)
{
    is_valid_ = true;

    return;
}

char *Type::name(void)
{
    return name_;
}

std::ostream& operator<<(std::ostream& out, Type *t)
{
    out << t->name();

    return out;
}

bool Type::is_valid(void)
{
    return is_valid_;
}

bool Type::IsEquivalentTo(Type *other)
{
    bool result = false;

    if(this == Type::errorType ||
       other == Type::errorType ||
       strcmp(name_, other->name()) == 0) {
        result = true;
    }

    return result;
}

bool Type::IsCompatibleWith(Type *other)
{
    return IsEquivalentTo(other);
}

/*** class NamedType *************************************************/

void NamedType::DoCheck(void)
{
    if (GetClass(this) == NULL) {
        ReportError::IdentifierNotDeclared(id_, LookingForType);
        is_valid_ = false;
    }

    return;
}

NamedType::NamedType(Identifier *i) : Type(*i->location())
{
    Assert(i != NULL);
    (id_ = i)->set_parent(this);
    name_ = id_->name();
    is_valid_ = true;

    return;
}

Identifier *NamedType::id(void)
{
    return id_;
}

bool NamedType::IsCompatibleWith(Type *other)
{
    // Assume that NamedType has been checked
    if(IsEquivalentTo(other)) {
        // consider error type
        return true;
    }
    // check null type
    NamedType *B = dynamic_cast<NamedType*>(other);
    if(B == NULL) {
        return false;
    } else {
        if (this == Type::nullType) {
            // null type is compatible with any NamedType
            return true;
        } else {
            ClassDecl * c = GetClass(this);
            if (c != NULL) {
                // Search base class and interfaces
                return c->IsTypeCompatibleWith(B);
            } else {
                // Named type is not declared. But we consider it matched
                return true;
            }
        }
    }
}

/*** class ArrayType *************************************************/

void ArrayType::DoCheck(void)
{
    elem_->Check();
    if (elem_->is_valid()) {
        is_valid_ = false;
    }

    return;
}

ArrayType::ArrayType(Type *t) : Type()
{
    Assert(t != NULL);
    (elem_ = t)->set_parent(this);
    name_ = strdup(elem_->name());
    name_ = (char*)realloc(name_, strlen(name_) + 3);
    if (name_ == NULL) {
        exit(137);
    }
    strcat(name_, "[]"); // construct names

    return;
}

ArrayType::ArrayType(yyltype loc, Type *t) : Type(loc)
{
    Assert(t != NULL);
    (elem_ = t)->set_parent(this);
    name_ = strdup(elem_->name());
    name_ = (char*)realloc(name_, strlen(name_) + 3);
    if (name_ == NULL) {
        exit(137);
    }
    strcat(name_, "[]"); // construct names

    return;
}

Type *ArrayType::elem(void)
{
    return elem_;
}
