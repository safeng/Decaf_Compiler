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
}

Node::Node() {
    location = NULL;
    parent = NULL;
}

ClassDecl *Node::GetClass(char *name)
{
    ClassDecl *c;

    if (parent != NULL) {
        c = this->parent->GetClass(name);
    } else {
        c = NULL;
    }

    return c;
}

FnDecl *Node::GetFn(char *name)
{
    FnDecl *f;

    if (parent != NULL) {
        f = this->parent->GetFn(name);
    } else {
        f = NULL;
    }

    return f;
}

VarDecl *Node::GetVar(char *name)
{
    VarDecl *v;

    if (parent != NULL) {
        v = this->parent->GetVar(name);
    } else {
        v = NULL;
    }

    return v;
}

Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
}
