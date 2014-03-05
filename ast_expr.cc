/**** ast_expr.cc - ASTs of expressions ******************************/

#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"

/*** class Expr ******************************************************/

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

/*** class IntConstant ***********************************************/

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc)
{
    value_ = val;
    type_  = Type::intType;

    return;
}

/*** class DoubleConstant ********************************************/

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc)
{
    value_ = val;
    type_  = Type::doubleType;

    return;
}

/*** class BoolConstant **********************************************/

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc)
{
    value_ = val;
    type_  = Type::boolType;

    return;
}

/*** class StringConstant ********************************************/

StringConstant::StringConstant(yyltype loc, const char *val) :
    Expr(loc)
{
    Assert(val != NULL);
    value_ = strdup(val);
    type_  = Type::stringType;

    return;
}

/*** class NullConstant **********************************************/

NullConstant::NullConstant(yyltype loc) : Expr(loc)
{
    type_ = Type::nullType;

    return;
}

/*** class Operator **************************************************/

Operator::Operator(yyltype loc, const char *lexeme) : Node(loc)
{
    Assert(tok != NULL);
    strncpy(lexeme_, tok, sizeof(lexeme_));

    return;
}

std::ostream& operator<<(std::ostream& out, Operator *o)
{
    return out << o->lexeme_;
}

/*** class CompoundExpr **********************************************/

void CompoundExpr::OperandCheck(void)
{
    if (left_ != NULL) {
        left_->Check();
    }
    right_->Check();

    return;
}

CompoundExpr::CompoundExpr(Expr *lhs, Operator *op, Expr *rhs) :
    Expr(Join(lhs->GetLocation(), rhs->GetLocation()))
{
    Assert(lhs != NULL && op != NULL && rhs != NULL);
    (op_ = op)->SetParent(this);
    (left_ = lhs)->SetParent(this);
    (right_ = rhs)->SetParent(this);

    return;
}

CompoundExpr::CompoundExpr(Operator *op, Expr *rhs) :
    Expr(Join(o->GetLocation(), rhs->GetLocation()))
{
    Assert(op != NULL && rhs != NULL);
    left_ = NULL;
    (op_ = op)->SetParent(this);
    (right_ = rhs)->SetParent(this);

    return;
}

/*** class ArithmeticExpr ********************************************/

void ArithmeticExpr::DoCheck(void)
{
    OperandCheck();
    if (left_ == NULL) {
        if (right_->type() == Type::intType ||
            right_->type() == Type::doubleType ||
            right_->type() == Type::errorType) {
            type_ = right_->type();
        } else {
            ReportError::IncompatibleOperand(op_, right_->type());
            type_ = Type::errorType;
        }
    } else {
        if (left_->type() == Type::errorType ||
            right_->type() == Type::errorType) {
            type_ = Type:errorType;
        } else if (left_->type() == right_->type() &&
                   (left_->type() == Type::intType ||
                    left_->type() == Type::doubleType)) {
                type_ = left_->type();
        } else {
            ReportError::IncompatibleOperand(op_, left_->type(),
                                             right_->type());
            type_ = Type::errorType;
        }
    }

    return;
}

ArithmeticExpr::ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

ArithmeticExpr::ArithmeticExpr(Operator *op, Expr *rhs) :
    CompoundExpr(op, rhs)
{
    return;
}

/*** class RelationalExpr ********************************************/

void RelationalExpr::DoCheck(void)
{
    OperandCheck();
    if (left_->type() == Type::errorType ||
        right_->type() == Type::errorType) {
        type_ = Type:errorType;
    } else if (left_->type() == right_->type() &&
               (left_->type() == Type::intType ||
                left_->type() == Type::doubleType)) {
        type_ = left_->type();
    } else {
        ReportError::IncompatibleOperand(op_, left_->type(),
                                         right_->type());
        type_ = Type::errorType;
    }

    return;
}

RelationalExpr::RelationalExpr(Expr *lhs, Operator *op, Expr *rhs)
    : CompoundExpr(lhs, op, rhs)
{
    return;
}

/*** class EqualityExpr ********************************************/

void EqualityExpr::DoCheck(void)
{
    OperandCheck();
    if (left_->type() == Type::errorType ||
        right_->type() == Type::errorType) {
        type_ = Type:errorType;
    } else if (left_->type() == right_->type() &&
               (left_->type() == Type::intType ||
                left_->type() == Type::doubleType)) {
        type_ = left_->type();
    } else {
        ReportError::IncompatibleOperand(op_, left_->type(),
                                         right_->type());
        type_ = Type::errorType;
    }

    return;
}

EqualityExpr::EqualityExpr(Expr *lhs, Operator *op, Expr *rhs)
    : CompoundExpr(lhs, op, rhs)
{
    return;
}

/*** class This ******************************************************/

void This::DoCheck(void)
{
    ClassDecl *c = GetCurrentClass();
    if (c == NULL) {
        ReportError::ThisOutsideClassScope(this);
    } else {
        NamedType *t = new NamedType(c->get_id());
        type_ = t;
    }

    return;
}

/*** class ArrayAccess ***********************************************/

void ArrayAccess::DoCheck(void)
{
    base->Check();
    subscript->Check();

    return;
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc)
{
    (base=b)->SetParent(this);
    (subscript=s)->SetParent(this);

    return;
}


FieldAccess::FieldAccess(Expr *b, Identifier *f)
: LValue((b != NULL) ? Join(b->GetLocation(), f->GetLocation())
         : *f->GetLocation())
{
    Assert(f != NULL); // b can be be NULL
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);

    return;
}

void FieldAccess::DoCheck(void)
{
    if (base == NULL) {
        VarDecl *v = GetVar(field);
        if (v == NULL) {
            ReportError::IdentifierNotDeclared(field,
                                               LookingForVariable);
        } else {
            type_ = v->get_type();
        }
    } else {
        This *th = dynamic_cast<This*>(base);
        if (th == NULL) {
            base->Check();
            ReportError::InaccessibleField(field, base->type());
        } else {
            ClassDecl *c = GetCurrentClass();
            NamedType *t = new NamedType(c->get_id());
            VarDecl *v = c->GetMemberVar(field->get_name());
            if (v == NULL) {
                ReportError::FieldNotFoundInBase(field, t);
            } else {
                type_ = v->get_type();
            }
        }
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
        FnDecl *f = GetFn(field);
        if (f == NULL) {
            ReportError::IdentifierNotDeclared(field,
                                               LookingForFunction);
        } else {
            type_ = f->get_return_type();
        }
    } else {
        This *th = dynamic_cast<This*>(base);
        if (th == NULL) {
            Type *t;
            NamedType *nt;
            base->Check();
            t = base->type();
            nt = dynamic_cast<NamedType*>(t);
            if (nt == NULL) {
                if (t != NULL) {
                    ReportError::FieldNotFoundInBase(field, t);
                }
            } else {
                ClassDecl *c = GetClass(nt);
                FnDecl *f;
                if (c != NULL) {
                    c->Check();
                    f = c->GetMemberFn(field->get_name());
                    if (f == NULL) {
                        ReportError::FieldNotFoundInBase
                            (field, base->type());
                    } else {
                        type_ = f->get_return_type();
                    }
                }
            }
        } else {
            ClassDecl *c = GetCurrentClass();
            NamedType *t = new NamedType(c->get_id());
            FnDecl *f = c->GetMemberFn(field->get_name());
            if (f == NULL) {
                ReportError::FieldNotFoundInBase(field, t);
            } else {
                type_ = f->get_return_type();
            }
        }
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
    if (GetClass(cType) == NULL) {
        ReportError::IdentifierNotDeclared(cType->get_id(),
                                           LookingForClass);
    }

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
