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

/*** class EmptyExpr ***********************************************/

EmptyExpr::EmptyExpr(void) : Expr()
{
    type_ = Type::voidType;

    return;
}

/*** class IntConstant ***********************************************/

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc)
{
    value_ = val;
    type_ = Type::intType;

    return;
}

/*** class DoubleConstant ********************************************/

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc)
{
    value_ = val;
    type_ = Type::doubleType;

    return;
}

/*** class BoolConstant **********************************************/

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc)
{
    value_ = val;
    type_ = Type::boolType;

    return;
}

/*** class StringConstant ********************************************/

StringConstant::StringConstant(yyltype loc, const char *val) :
    Expr(loc)
{
    Assert(val != NULL);
    value_ = strdup(val);
    type_ = Type::stringType;

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
    Assert(lexeme != NULL);
    strncpy(lexeme_, lexeme, sizeof(lexeme_));

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
    Expr(Join(lhs->location(), rhs->location()))
{
    Assert(lhs != NULL && op != NULL && rhs != NULL);
    (op_ = op)->set_parent(this);
    (left_ = lhs)->set_parent(this);
    (right_ = rhs)->set_parent(this);

    return;
}

CompoundExpr::CompoundExpr(Operator *op, Expr *rhs) :
    Expr(Join(op->location(), rhs->location()))
{
    Assert(op != NULL && rhs != NULL);
    left_ = NULL;
    (op_ = op)->set_parent(this);
    (right_ = rhs)->set_parent(this);

    return;
}

/*** class ArithmeticExpr ********************************************/

void ArithmeticExpr::UnaryCheck(void)
{
    if (right_->type() == Type::intType ||
        right_->type() == Type::doubleType ||
        right_->type() == Type::errorType) {
        type_ = right_->type();
    } else {
        ReportError::IncompatibleOperand(op_, right_->type());
        type_ = Type::errorType;
    }

    return;
}

void ArithmeticExpr::BinaryCheck(void)
{
    // Left is not valid type.
    bool report = (left_->type() != Type::intType &&
                   left_->type() != Type::doubleType &&
                   left_->type() != Type::errorType);
    // Right is not valid type.
    report = report || (right_->type() != Type::intType &&
                        right_->type() != Type::doubleType &&
                        right_->type() != Type::errorType);
    // Both operand types are not the same, error excepted.
    report = report || (left_->type() != Type::errorType &&
                        right_->type() != Type::errorType &&
                        left_->type() != right_->type());
    if (report) {
        ReportError::IncompatibleOperands(op_, left_->type(),
                                          right_->type());
        type_ = Type::errorType;
    } else if (left_->type() == Type::errorType ||
               right_->type() == Type::errorType) {
        type_ = Type::errorType;
    } else {
        type_ = left_->type();
    }

    return;
}

void ArithmeticExpr::DoCheck(void)
{
    OperandCheck();
    if (left_ == NULL) {
        UnaryCheck();
    } else {
        BinaryCheck();
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
    bool report = false;

    OperandCheck();
    // Left is not valid type.
    report = (left_->type() != Type::intType &&
              left_->type() != Type::doubleType &&
              left_->type() != Type::errorType);
    // Right is not valid type.
    report = report || (right_->type() != Type::intType &&
                        right_->type() != Type::doubleType &&
                        right_->type() != Type::errorType);
    // Both operand types are not the same, error excepted.
    report = report || (left_->type() != Type::errorType &&
                        right_->type() != Type::errorType &&
                        left_->type() != right_->type());
    if (report) {
        ReportError::IncompatibleOperands(op_, left_->type(),
                                          right_->type());
    }
    type_ = Type::boolType;

    return;
}

RelationalExpr::RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

/*** class EqualityExpr **********************************************/

void EqualityExpr::DoCheck(void)
{
    OperandCheck();
    if (!left_->type()->IsCompatibleWith(right_->type()) &&
        !right_->type()->IsCompatibleWith(left_->type())) {
        ReportError::IncompatibleOperands(op_, left_->type(),
                                          right_->type());
    }
    type_ = Type::boolType;

    return;
}

EqualityExpr::EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

const char *EqualityExpr::GetPrintNameForNode(void)
{
    return "EqualityExpr";
}

/*** class LogicalExpr ***********************************************/

void LogicalExpr::UnaryCheck(void)
{
    if (right_->type() == Type::boolType ||
        right_->type() == Type::errorType) {
        type_ = right_->type();
    } else {
        ReportError::IncompatibleOperand(op_, right_->type());
        type_ = Type::errorType;
    }

    return;
}

void LogicalExpr::BinaryCheck(void)
{
    // Left is not valid type.
    bool report = (left_->type() != Type::boolType &&
                   left_->type() != Type::errorType);
    // Right is not valid type.
    report = report || (right_->type() != Type::boolType &&
                        right_->type() != Type::errorType);
    if (report) {
        ReportError::IncompatibleOperands(op_, left_->type(),
                                          right_->type());
    }
    type_ = Type::boolType;

    return;
}

void LogicalExpr::DoCheck(void)
{
    OperandCheck();
    if (left_ == NULL) {
        UnaryCheck();
    } else {
        BinaryCheck();
    }

    return;
}

LogicalExpr::LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

LogicalExpr::LogicalExpr(Operator *op, Expr *rhs) :
    CompoundExpr(op, rhs)
{
    return;
}

const char *LogicalExpr::GetPrintNameForNode(void)
{
    return "LogicalExpr";
}

/*** class AssignExpr ************************************************/

void AssignExpr::DoCheck(void)
{
    OperandCheck();
    if (left_->type() == Type::errorType ||
        right_->type() == Type::errorType) {
        type_ = Type::errorType;
    } else {
        type_ = left_->type();
        if (!right_->type()->IsCompatibleWith(left_->type())) {
            ReportError::IncompatibleOperands(op_, left_->type(),
                                              right_->type());
        }
    }

    return;
}

AssignExpr::AssignExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

const char *AssignExpr::GetPrintNameForNode(void)
{
    return "AssignExpr";
}

/*** class This ******************************************************/

void This::DoCheck(void)
{
    ClassDecl *c = GetCurrentClass();
    if (c == NULL) {
        ReportError::ThisOutsideClassScope(this);
        type_ = Type::errorType;
    } else {
        type_ = new NamedType(c->id());
    }

    return;
}

/*** class ArrayAccess ***********************************************/

void ArrayAccess::DoCheck(void)
{
    base_->Check();
    subscript_->Check();
    if (base_->type() == Type::errorType) {
        type_ = Type::errorType;
    } else if (dynamic_cast<ArrayType*>(base_->type()) == NULL) {
        ReportError::BracketsOnNonArray(base_);
        type_ = Type::errorType;
    }
    if (subscript_->type() != Type::intType &&
        subscript_->type() != Type::errorType) {
        ReportError::SubscriptNotInteger(subscript_);
    }
    // Assign the type of base to whole expression
    if (type_ == NULL) {
        type_ = dynamic_cast<ArrayType*>(base_->type())->elem();
    }

    return;
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *base, Expr *subscript) :
    LValue(loc)
{
    (base_ = base)->set_parent(this);
    (subscript_ = subscript)->set_parent(this);

    return;
}


FieldAccess::FieldAccess(Expr *b, Identifier *f) :
    LValue((b != NULL) ? Join(b->location(), f->location())
           : *f->location())
{
    Assert(f != NULL); // b can be be NULL
    base = b;
    if (base) base->set_parent(this);
    (field=f)->set_parent(this);

    return;
}

void FieldAccess::UnaryCheck(void)
{
    VarDecl *v = GetVar(field);
    if (v == NULL) {
        ReportError::IdentifierNotDeclared(field, LookingForVariable);
        type_ = Type::errorType;
    } else {
        v->Check();
        type_ = v->type();
    }

    return;
}

void FieldAccess::NativeAccessCheck(void)
{
    ClassDecl *c = GetCurrentClass();
    VarDecl *v = c == NULL ? NULL : c->GetMemberVar(field->name());
    if (v == NULL) {
        ReportError::FieldNotFoundInBase(field, base->type());
        type_ = Type::errorType;
    } else {
        v->Check();
        type_ = v->type();
    }

    return;
}

void FieldAccess::ForeignAccessCheck(void)
{
    Type *bt = base->type();
    NamedType *bnt = dynamic_cast<NamedType*>(bt);
    ClassDecl *c = bnt == NULL ? NULL : GetClass(bnt);
    if (c == NULL) {
        ReportError::FieldNotFoundInBase(field, bt);
        type_ = Type::errorType;
    } else {
        VarDecl *v;
        c->Check();
        v = c->GetMemberVar(field->name());
        if (v == NULL) {
            ReportError::FieldNotFoundInBase(field, bt);
            type_ = Type::errorType;
        } else {
            v->Check();
            if (GetCurrentClass() == NULL ||
                !GetCurrentClass()->IsSubsetOf(bnt)) {
                ReportError::InaccessibleField(field, bt);
                type_ = Type::errorType;
            } else {
                type_ = v->type();
            }
        }
    }

    return;
}

void FieldAccess::BinaryCheck(void)
{
    if (dynamic_cast<This*>(base) == NULL) {
        ForeignAccessCheck();
    } else {
        NativeAccessCheck();
    }

    return;
}

void FieldAccess::DoCheck(void)
{
    if (base == NULL) {
        UnaryCheck();
    } else {
        base->Check();
        if (base->type() != Type::errorType) {
            BinaryCheck();
        } else {
            type_ = Type::errorType;
        }
    }

    return;
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) :
    Expr(loc)
{
    Assert(f != NULL && a != NULL);
    base = b;
    if (base != NULL) {
        base->set_parent(this);
    }
    (field=f)->set_parent(this);
    (actuals=a)->set_parent_all(this);

    return;
}

void Call::UnaryCheck(void)
{
    FnDecl *f = GetFn(field);
    if (f != NULL) {
        f->Check();
        f->CheckCallCompatibility(field, actuals);
        type_ = f->return_type();
        if (!type_->is_valid()) {
            type_ = Type::errorType;
        }
    } else {
        ReportError::IdentifierNotDeclared(field, LookingForFunction);
        type_ = Type::errorType;
    }

    return;
}

void Call::BinaryCheck(void)
{
    base->Check();
    if (base->type() == Type::errorType) {
        type_ = Type::errorType;
    } else if (dynamic_cast<ArrayType*>(base->type()) != NULL) {
        // Check array.length()
        if (strcmp(field->name(), "length") == 0) {
            CallCheck(new LengthFn(*field->location()));
        } else {
            ReportError::FieldNotFoundInBase(field, base->type());
            type_ = Type::errorType;
        }
    } else if (dynamic_cast<This*>(base) != NULL) {
        // this.func()
        CallCheck(GetCurrentClass()->GetMemberFn(field->name()));
    } else {
        // var.func()
        NamedType *nt = dynamic_cast<NamedType*>(base->type());
        ClassDecl *c = nt == NULL ? NULL : GetClass(nt);
        InterfaceDecl *itf = nt == NULL ? NULL : GetInterface(nt);
        FnDecl *f = (c != NULL   ? (c->Check(), c->GetMemberFn(field->name())) :
                     itf != NULL ? (itf->Check(), itf->GetMemberFn(field->name())) :
                     /* Else */    NULL);
        CallCheck(f);
    }

    return;
}

void Call::CallCheck(FnDecl *f)
{
    if (f != NULL) {
        f->Check();
        f->CheckCallCompatibility(field, actuals);
        type_ = f->return_type();
        if (!type_->is_valid()) {
            type_ = Type::errorType;
        }
    } else {
        ReportError::FieldNotFoundInBase(field, base->type());
        type_ = Type::errorType;
    }

    return;
}

void Call::DoCheck(void)
{
    for (int i = 0; i < actuals->NumElements(); i++) {
        actuals->Nth(i)->Check();
    }
    if (base == NULL) {
        UnaryCheck();
    } else {
        BinaryCheck();
    }

    return;
}


NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc)
{
    Assert(c != NULL);
    (cType=c)->set_parent(this);

    return;
}


void NewExpr::DoCheck(void)
{
    if (GetClass(cType) == NULL) {
        ReportError::IdentifierNotDeclared(cType->id(),
                                           LookingForClass);
        type_ = Type::errorType;
    } else {
        type_ = cType;
    }

    return;
}


void NewArrayExpr::DoCheck(void)
{
    size->Check();
    if (size->type() != Type::errorType &&
        size->type() != Type::intType) {
        ReportError::NewArraySizeNotInteger(size);
    }
    elemType->Check();
    if (size->type() == Type::errorType) {
        type_ = Type::errorType;
    } else {
        type_ = new ArrayType(elemType);
    }

    return;
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc)
{
    Assert(sz != NULL && et != NULL);
    (size=sz)->set_parent(this);
    (elemType=et)->set_parent(this);

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
