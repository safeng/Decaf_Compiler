/**** ast.cc - AST base class and common ASTs ************************/

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include <stdio.h>

/*** class Node *******************************************************/

Node::Node(yyltype loc)
{
    location_ = new yyltype(loc);
    parent_ = NULL;
    checked_ = false;

    return;
}

Node::Node(void)
{
    location_ = NULL;
    parent_ = NULL;
    checked_ = false;

    return;
}

yyltype *Node::location(void)
{
    return location_;
}

void Node::set_parent(Node *p)
{
    parent_ = p;

    return;
}

Node *Node::parent(void)
{
    return parent_;
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

    if (parent() != NULL) {
        c = parent()->GetClass(t);
    } else {
        c = NULL;
    }

    return c;
}

ClassDecl *Node::GetCurrentClass(void)
{
    ClassDecl *c;

    if (parent() != NULL) {
        c = parent()->GetCurrentClass();
    } else {
        c = NULL;
    }

    return c;
}

FnDecl *Node::GetCurrentFn(void)
{
	FnDecl *f;
	if(parent() != NULL)
		f = parent()->GetCurrentFn();
	else
		f = NULL;

	return f;
}

InterfaceDecl *Node::GetInterface(NamedType *t)
{
    InterfaceDecl *i;

    if (parent() != NULL) {
        i = parent()->GetInterface(t);
    } else {
        i = NULL;
    }

    return i;
}

FnDecl *Node::GetFn(Identifier *id)
{
    FnDecl *f;

    if (parent() != NULL) {
        f = parent()->GetFn(id);
    } else {
        f = NULL;
    }

    return f;
}

VarDecl *Node::GetVar(Identifier *id)
{
    VarDecl *v;

    if (parent() != NULL) {
        v = parent()->GetVar(id);
    } else {
        v = NULL;
    }

    return v;
}

/*** class Identifier *************************************************/

Identifier::Identifier(yyltype loc, const char *n) : Node(loc)
{
    name_ = strdup(n);

    return;
}

char *Identifier::name(void)
{
    return name_;
}

std::ostream& operator<<(std::ostream& out, Identifier *id)
{
    return out << id->name_;
}

/*** class Error ******************************************************/

Error::Error(void) : Node()
{
    return;
}
