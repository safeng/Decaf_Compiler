/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h> // strdup
#include <stdio.h>  // printf

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = NULL;
    checked_ = false;
}

Node::Node() {
    location = NULL;
    parent = NULL;
    checked_ = false;
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
        c = parent->GetClass(t); // Recursively find classDecl
    } else {
        c = NULL;
    }

    return c;
}

ClassDecl *Node::GetCurrentClass()
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

FnDecl *Node::GetFn(char *name)
{
    FnDecl *f;

    if (parent != NULL) {
        f = parent->GetFn(name);
    } else {
        f = NULL;
    }

    return f;
}

VarDecl *Node::GetVar(char *name)
{
    VarDecl *v;

    if (parent != NULL) {
        v = parent->GetVar(name);
    } else {
        v = NULL;
    }

    return v;
}

Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
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
