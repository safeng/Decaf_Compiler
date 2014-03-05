/**** ast.cc - AST base class and common ASTs *************************
 *
 * Copyright © 2014 Shuang Feng, Hongjiu Zhang
 *
 * All rights reserved.                                              */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include <stdio.h>

/*** class Node *******************************************************/

Node::Node(yyltype loc)
{
    location = new yyltype(loc);
    parent = NULL;
    checked_ = false;

    return;
}

Node::Node(void)
{
    location = NULL;
    parent = NULL;
    checked_ = false;

    return;
}

yyltype *Node::GetLocation(void)
{
    return location;
}

void Node::SetParent(Node *p)
{
    parent = p;

    return;
}

Node *Node::GetParent(void)
{
    return parent;
}

void Node::DoCheck(void)
{
    return;
}

void Node::Check(void)
{
    if (!checked_) {
        checked_ = true;
        DoCheck();
    }

    return;
}

ClassDecl *Node::GetClass(NamedType *t)
{
    ClassDecl *c;

    if (parent != NULL) {
        c = parent->GetClass(t);
    } else {
        c = NULL;
    }

    return c;
}

ClassDecl *Node::GetCurrentClass(void)
{
    ClassDecl *c;

    if (parent != NULL) {
        c = parent->GetCurrentClass();
    } else {
        c = NULL;
    }

    return c;
}

InterfaceDecl *Node::GetInterface(NamedType *t)
{
    return NULL;
}

FnDecl *Node::GetFn(Identifier *id)
{
    FnDecl *f;

    if (parent != NULL) {
        f = parent->GetFn(id);
    } else {
        f = NULL;
    }

    return f;
}

VarDecl *Node::GetVar(Identifier *id)
{
    VarDecl *v;

    if (parent != NULL) {
        v = parent->GetVar(id);
    } else {
        v = NULL;
    }

    return v;
}

/*** class Identifier *************************************************/

Identifier::Identifier(yyltype loc, const char *n) : Node(loc)
{
    name = strdup(n);

    return;
}

char *Identifier::get_name(void)
{
    return name;
}

std::ostream& operator<<(std::ostream& out, Identifier *id)
{
    return out << id->name;
}

/*** class Error ******************************************************/

Error::Error(void) : Node()
{
    return;
}
