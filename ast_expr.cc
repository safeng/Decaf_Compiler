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
EmptyExpr::EmptyExpr(void) : Stmt()
{
	type_ = Type::voidType;

	return;
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
            type_ = Type::errorType;
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
        type_ = Type::errorType;
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

RelationalExpr::RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

/*** class EqualityExpr **********************************************/

void EqualityExpr::DoCheck(void)
{
    OperandCheck();
    if (left_->type() == Type::errorType ||
        right_->type() == Type::errorType) {
        type_ = Type::errorType;
    } else if (left_->type() == right_->type()) {
        type_ = Type::boolType;
    } else {
        ReportError::IncompatibleOperand(op_, left_->type(),
                                         right_->type());
        type_ = Type::errorType;
    }

    return;
}

EqualityExpr::EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

char *EqualityExpr::GetPrintNameForNode(void)
{
    return "EqualityExpr";
}

/*** class LogicalExpr ***********************************************/

void LogicalExpr::DoCheck(void)
{
    OperandCheck();
    if (left_ == NULL) {
        if (right_->type() == Type::boolType ||
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
                   left_->type() == Type::boolType) {
            type_ = Type::boolType;
        } else {
            ReportError::IncompatibleOperand(op_, left_->type(),
                                             right_->type());
            type_ = Type::errorType;
        }
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

char *LogicalExpr::GetPrintNameForNode(void)
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
    } else if (left_->type() == right_->type()) {
        type_ = left_->type();
    } else {
        ReportError::IncompatibleOperand(op_, left_->type(),
                                         right_->type());
        type_ = Type::errorType;
    }

    return;
}

AssignExpr::AssignExpr(Expr *lhs, Operator *op, Expr *rhs) :
    CompoundExpr(lhs, op, rhs)
{
    return;
}

char *AssignExpr::GetPrintNameForNode(void)
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
        type_ = new NamedType(c->get_id());
    }

    return;
}

/*** class ArrayAccess ***********************************************/

void ArrayAccess::DoCheck(void)
{
    base_->Check();
    subscript_->Check();
    if (base_->type() == Type::errorType ||
        subscript_->type() == Type::errorType) {
        type_ = Type::errorType;
    } else {
        if (dynamic_cast<ArrayType*>(base_->type()) == NULL) {
            ReportError::BracketsOnNonArray(base_);
            type_ = Type::errorType;
        }
        if (subscript_->type() != Type::intType) {
            ReportError::SubscriptNotInteger(subscript_);
            type_ = Type::errorType;
        }
    }
	// Assign the type of base to whole expression
    if (type_ == NULL) {
        type_ = base_->type()->elem();
    }

    return;
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *base, Expr *subscript) :
    LValue(loc)
{
    (base_ = base)->SetParent(this);
    (subscript_ = subscript)->SetParent(this);

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
    type_ = Type::errorType; // default: set as error type
    if (base == NULL) {
        VarDecl *v = GetVar(field);
        if (v == NULL) {
            ReportError::IdentifierNotDeclared(field,
                                               LookingForVariable);
        } else {
            type_ = v->get_type();
        }
    } else { // must be this.field
        This *th = dynamic_cast<This*>(base);
        if (th == NULL) {
            base->Check(); // check base
            ReportError::InaccessibleField(field, base->type());
        } else {
            // Even with this.field, we should check whether we are in a class scope
            th->Check();
            ClassDecl *c = GetCurrentClass();
            if (c != NULL)
            {
                //NamedType *t = new NamedType(c->get_id());
                VarDecl *v = c->GetMemberVar(field->get_name());
                if (v == NULL) {
                    ReportError::FieldNotFoundInBase(field,
                                                     base->type());
                } else {
                    type_ = v->get_type();
                }
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
    type_ = Type::errorType; // default: set as error type
	FnDecl * calledFn = NULL; // definition of function to be called
    if (base == NULL) {
		// Omit this or call a global function
        FnDecl *f = GetFn(field); // can be global or member function
        if (f == NULL) {
            ReportError::IdentifierNotDeclared(field,
                                               LookingForFunction);
        } else {
            type_ = f->get_return_type();
			calledFn = f;
        }
    } else {
        base->Check();
        if (base->type() != Type::errorType) {
            if (dynamic_cast<This*>(base) == NULL) // var.func()
			{
                NamedType *nt = dynamic_cast<NamedType*>(base->type());
                if (nt == NULL) {
                    ReportError::FieldNotFoundInBase(field, base->type());
                } else {
                    ClassDecl *c = GetClass(nt);
                    FnDecl *f = c->GetMemberFn(field->name());
                    if (f == NULL) {
                        ReportError::FieldNotFoundInBase
                            (field, base->type());
                    } else {
                        type_ = f->get_return_type();
						calledFn = f;
                    }
                }
            } else // this.func()
			{
                ClassDecl *c = GetCurrentClass(); // must exist
                FnDecl *f = c->GetMemberFn(field->name());
                if (f == NULL) {
                    ReportError::FieldNotFoundInBase(field,
                                                     base->type());
                } else {
                    type_ = f->get_return_type();
					calledFn = f;
                }
            }
        }
    }

    for (int i = 0; i < actuals->NumElements(); i++) {
        actuals->Nth(i)->Check();
    }
	
	// check type agreement between caller and callee. Formals vs. Actuals
	if(calledFn != NULL)
	{
		calledFn->CheckCallCompatibility(actuals);
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
