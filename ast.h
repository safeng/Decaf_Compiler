/**** ast.h - AST base class and common ASTs *************************/

#ifndef _H_ast
#define _H_ast

#include <stdlib.h>
#include <iostream>

#include "location.h"

class FnDecl;
class VarDecl;
class ClassDecl;
class InterfaceDecl;
class NamedType;

class Identifier;

class Node
{
    protected:
        bool checked_;
        yyltype *location_;
        Node *parent_;

        virtual void DoCheck(void);

    public:
        Node(yyltype loc);
        Node(void);

        yyltype *location(void);

        void set_parent(Node *p);
        Node *parent(void);

        void Check(void);

        virtual ClassDecl *GetClass(NamedType *t);
        virtual ClassDecl *GetCurrentClass(void);
        virtual InterfaceDecl *GetInterface(NamedType *t);
        virtual FnDecl *GetFn(Identifier *id);
        virtual VarDecl *GetVar(Identifier *id);
};

class Identifier : public Node
{
    protected:
        char *name_;

    public:
        Identifier(yyltype loc, const char *n);
        char *name(void);
        friend std::ostream& operator<<(std::ostream& out,
                                        Identifier *id);
};

// This node class is designed to represent a portion of the tree that
// encountered syntax errors during parsing. The partial completed tree
// is discarded along with the states being popped, and an instance of
// the Error class can stand in as the placeholder in the parse tree
// when your parser can continue after an error.
class Error : public Node
{
    public:
        Error(void);
};

#endif
