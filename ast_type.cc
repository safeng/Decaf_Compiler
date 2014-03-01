/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */

#include <string.h>

#include "ast_type.h"
#include "ast_decl.h"

/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double,
 * etc.) They can be accessed with the syntax Type::intType. This
 * allows you to directly access them and share the built-in types
 * where needed rather that creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error");

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);

    return;
}

Type::Type(yyltype loc) : Node(loc)
{
    return;
}

void Type::PrintToStream(std::ostream& out)
{
    out << typeName;

    return;
}

std::ostream& operator<<(std::ostream& out, Type *t)
{
    t->PrintToStream(out);

    return out;
}

bool Type::IsEquivalentTo(Type *other)
{
    return this == other;
}


void NamedType::DoCheck(void)
{
    GetClass(this->typeName);

    return;
}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
}

void NamedType::PrintToStream(std::ostream& out)
{
    out << id;

    return;
}


ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}

void ArrayType::PrintToStream(std::ostream& out)
{
    out << elemType << "[]";

    return;
}
