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
        bool is_valid_;

    public :
        // Static built-in types
        static Type *intType;
        static Type *doubleType;
        static Type *boolType;
        static Type *voidType;
        static Type *nullType;
        static Type *stringType;
        static Type *errorType;

        Type(void);
        Type(const char *str);
        Type(yyltype loc);

        char *name(void);
        bool is_valid(void);

        friend std::ostream& operator<<(std::ostream& out, Type *t);

        virtual bool IsEquivalentTo(Type *other); // return A==B
        virtual bool IsCompatibleWith(Type *B); // return A<=B
};

/* Type for classes and interfaces */
class NamedType : public Type
{
    protected:
        Identifier *id_;
        void DoCheck(void);

    public:
        NamedType(Identifier *i);

        Identifier *id(void);

        bool IsCompatibleWith(Type *B);
};

class ArrayType : public Type
{
    protected:
        Type *elem_; // ArrayType has a Type associated with it

        void DoCheck(void);

    public:
        ArrayType(Type *t);
        ArrayType(yyltype loc, Type *t);

        Type *elem(void);
};

#endif
