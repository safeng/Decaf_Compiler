/**** ast_type.h - ASTs for types ************************************/

#ifndef _H_ast_type
#define _H_ast_type

#include <iostream>

#include "ast.h"
#include "list.h"

class Type : public Node
{
    protected:
        char *name_;

    public :
        // Static built-in types
        static Type *intType;
        static Type *doubleType;
        static Type *boolType;
        static Type *voidType;
        static Type *nullType;
        static Type *stringType;
        static Type *errorType;

        Type(const char *str);
        Type(yyltype loc);

        char *name(void);

        friend std::ostream& operator<<(std::ostream& out, Type *t);

        virtual bool IsEquivalentTo(Type *other);
};

/* Type for classes and interfacess */
class NamedType : public Type
{
    protected:
        Identifier *id_;
        void DoCheck(void);

    public:
        NamedType(Identifier *i);

        Identifier *id(void);
};

class ArrayType : public Type
{
    protected:
        Type *elem_;

    public:
        ArrayType(yyltype loc, Type *t);

        Type *elem(void);
};

#endif
