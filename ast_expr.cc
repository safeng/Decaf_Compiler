/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"


Expr::Expr(yyltype loc) : Stmt(loc)
{
    type_ = NULL;

    return;
}

Expr::Expr(void) : Stmt()
{
    type_ = NULL;

    return;
}

Type *Expr::type(void)
{
    return type_;
}

void Expr::set_type(Type *t)
{
    type_ = t;

    return;
}


IntConstant::IntConstant(yyltype loc, int val) : Expr(loc)
{
    value = val;
    type_ = Type::intType;

    return;
}


DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc)
{
    value = val;
    type_ = Type::doubleType;

    return;
}


BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc)
{
    value = val;
    type_ = Type::boolType;

    return;
}


StringConstant::StringConstant(yyltype loc, const char *val) :
    Expr(loc)
{
    Assert(val != NULL);
    value = strdup(val);
    type_ = Type::stringType;

    return;
}


NullConstant::NullConstant(yyltype loc) : Expr(loc)
{
    type_ = Type::nullType;

    return;
}


Operator::Operator(yyltype loc, const char *tok) : Node(loc)
{
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));

    return;
}

std::ostream& operator<<(std::ostream& out, Operator *o)
{
    return out << o->tokenString;
}


CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) :
    Expr(Join(l->GetLocation(), r->GetLocation()))
{
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this);
    (right=r)->SetParent(this);

    return;
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) :
    Expr(Join(o->GetLocation(), r->GetLocation()))
{
    Assert(o != NULL && r != NULL);
    left = NULL;
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);

    return;
}


ArithmeticExpr::ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs,op,rhs)
{
    return;
}

ArithmeticExpr::ArithmeticExpr(Operator *op, Expr *rhs) :
    CompoundExpr(op,rhs)
{
    return;
}


ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc)
{
    (base=b)->SetParent(this);
    (subscript=s)->SetParent(this);
}


FieldAccess::FieldAccess(Expr *b, Identifier *f)
    : LValue((b != NULL) ? Join(b->GetLocation(), f->GetLocation())
             : *f->GetLocation())
{
    Assert(f != NULL); // b can be be NULL
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
}

void FieldAccess::DoCheck(void)
{
    if (base == NULL) {
        field->Check();
    } else {
        base->Check();
        field->Check(); // TODO: Need to check class sym_
    }

    return;
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) :
    Expr(loc)
{
    Assert(f != NULL && a != NULL);
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);

    return;
}

void Call::DoCheck(void)
{
    if (base == NULL) {
        field->Check();
    } else {
        base->Check();
        field->Check(); // TODO: Need to check class sym_
    }
    for (int i = 0; i < actuals->NumElements(); i++) {
        actuals->Nth(i)->Check();
    }

    return;
}


NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc)
{
    Assert(c != NULL);
    (cType=c)->SetParent(this);

    return;
}

void NewExpr::DoCheck(void)
{
    cType->Check();

    return;
}


void NewArrayExpr::DoCheck(void)
{
    size->Check();
    // TODO: Check the type of size
    elemType->Check();

    return;
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc)
{
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this);
    (elemType=et)->SetParent(this);

    return;
}


ReadIntegerExpr::ReadIntegerExpr(yyltype loc) : Expr(loc)
{
    type_ = Type::intType;

    return;
}


ReadLineExpr::ReadLineExpr(yyltype loc) : Expr (loc)
{
    type_ = Type::stringType;

    return;
}
