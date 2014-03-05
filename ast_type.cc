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

Type::Type(const char *n)
{
    Assert(n);
    name_ = strdup(n);

    return;
}

Type::Type(yyltype loc) : Node(loc)
{
    return;
}

char *Type::name(void)
{
    return name_;
}

std::ostream& operator<<(std::ostream& out, Type *t)
{
    out << name_;

    return out;
}

bool Type::IsEquivalentTo(Type *other)
{
    bool result;

    if (strcmp(name_, other->name()) == 0) {
        result = true;
    } else {
        result = false;
    }

    return result;
}

/*** class NamedType *************************************************/

void NamedType::DoCheck(void)
{
    if (GetClass(this) == NULL) {
        ReportError::IdentifierNotDeclared(id_, LookingForType);
    }

    return;
}

NamedType::NamedType(Identifier *i) : Type(*i->location())
{
    Assert(i != NULL);
    (id_ = i)->SetParent(this);
    name_ = id_->name();

    return;
}

Identifier *NamedType::id(void)
{
    return id_;
}

/*** class ArrayType *************************************************/

ArrayType::ArrayType(yyltype loc, Type *t) : Type(loc)
{
    Assert(t != NULL);
    (elem_ = t)->SetParent(this);
    name_ = strdup(elem_->name());
    realloc(name_, strlen(name_) + 3);
    if (name_ == NULL) {
        exit(137);
    }
    strcat(name_, "[]");

    return;
}
