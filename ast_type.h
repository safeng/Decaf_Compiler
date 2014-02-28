/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */

#ifndef _H_ast_type
#define _H_ast_type

#include <iostream>

#include "ast.h"
#include "list.h"

class Type : public Node
{
    protected:
        char *typeName;

    public :
        static Type *intType;
        static Type *doubleType;
        static Type *boolType;
        static Type *voidType;
        static Type *nullType;
        static Type *stringType;
        static Type *errorType;

        Type(const char *str);
        Type(yyltype loc);

        virtual void PrintToStream(std::ostream& out);
        friend std::ostream& operator<<(std::ostream& out, Type *t);
        virtual bool IsEquivalentTo(Type *other);
};

class NamedType : public Type
{
    protected:
        Identifier *id;
        void DoCheck(void);

    public:
        NamedType(Identifier *i);

        void PrintToStream(std::ostream& out);
};

class ArrayType : public Type
{
    protected:
        Type *elemType;

    public:
        ArrayType(yyltype loc, Type *elemType);

        void PrintToStream(std::ostream& out);
};

#endif
