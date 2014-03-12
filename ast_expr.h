/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct.
 *
 * pp3: You will need to extend the Expr classes to implement
 * semantic analysis for rules pertaining to expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"

class NamedType; // for new
class Type; // for NewArray


class Expr : public Stmt
{
    protected:
        Type *type_;

    public:
        Expr(yyltype loc);
        Expr(void);
        Type *type(void);
};

/* This node type is used for those places where an expression is op-
 * tional. We could use a NULL pointer, but then it adds a lot of
 * checking for NULL. By using a valid, but no-op, node, we save that
 * trouble.
 */
class EmptyExpr : public Expr
{
    public:
        EmptyExpr(void);
};

/* Assign type to constant */
class IntConstant : public Expr
{
    protected:
        int value_;

    public:
        IntConstant(yyltype loc, int val);
};

class DoubleConstant : public Expr
{
    protected:
        double value_;

    public:
        DoubleConstant(yyltype loc, double val);
};

class BoolConstant : public Expr
{
    protected:
        bool value_;

    public:
        BoolConstant(yyltype loc, bool val);
};

class StringConstant : public Expr
{
    protected:
        char *value_;

    public:
        StringConstant(yyltype loc, const char *val);
};

class NullConstant: public Expr
{
    public:
        NullConstant(yyltype loc);
};

class Operator : public Node
{
    protected:
        char lexeme_[4];

    public:
        Operator(yyltype loc, const char *lexeme);
        friend std::ostream& operator<<(std::ostream& out,
                                        Operator *o);
};

class CompoundExpr : public Expr
{
    protected:
        Operator *op_;
        Expr *left_, *right_;
        void OperandCheck(void);

    public:
        CompoundExpr(Expr *lhs, Operator *op, Expr *rhs);
        CompoundExpr(Operator *op, Expr *rhs);
};

class ArithmeticExpr : public CompoundExpr
{
    private:
        void UnaryCheck(void);
        void BinaryCheck(void);

    protected:
        void DoCheck(void);

    public:
        ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs);
        ArithmeticExpr(Operator *op, Expr *rhs);
};

class RelationalExpr : public CompoundExpr
{
    protected:
        void DoCheck(void);

    public:
        RelationalExpr(Expr *lhs, Operator *op, Expr *rhs);
};

class EqualityExpr : public CompoundExpr
{
    protected:
        void DoCheck(void);

    public:
        EqualityExpr(Expr *lhs, Operator *op, Expr *rhs);

        const char *GetPrintNameForNode(void);
};

class LogicalExpr : public CompoundExpr
{
    private:
        void UnaryCheck(void);
        void BinaryCheck(void);

    protected:
        void DoCheck(void);

    public:
        LogicalExpr(Expr *lhs, Operator *op, Expr *rhs);
        LogicalExpr(Operator *op, Expr *rhs);

        const char *GetPrintNameForNode(void);
};

class AssignExpr : public CompoundExpr
{
    protected:
        void DoCheck(void);

    public:
        AssignExpr(Expr *lhs, Operator *op, Expr *rhs);
        const char *GetPrintNameForNode();
};

class LValue : public Expr
{
    public:
        LValue(yyltype loc) : Expr(loc) {}
};

class This : public Expr
{
    protected:
        void DoCheck(void);

    public:
        This(yyltype loc) : Expr(loc) {}
};

class ArrayAccess : public LValue
{
    protected:
        Expr *base_, *subscript_;
        void DoCheck(void);

    public:
        ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
};

/* Note that field access is used both for qualified names
 * base.field and just field without qualification. We don't
 * know for sure whether there is an implicit "this." in
 * front until later on, so we use one node type for either
 * and sort it out later. */
class FieldAccess : public LValue
{
    private:
        void UnaryCheck(void);
        void NativeAccessCheck(void);
        void ForeignAccessCheck(void);
        void BinaryCheck(void);

    protected:
        Expr *base; // will be NULL if no explicit base
        Identifier *field;
        void DoCheck(void);

    public:
        FieldAccess(Expr *base, Identifier *field); //NULL base is OK
};

/* Like field access, call is used both for qualified base.field()
 * and unqualified field().  We won't figure out until later
 * whether we need implicit "this." so we use one node type for either
 * and sort it out later. */
class Call : public Expr
{
    private:
        void UnaryCheck(void);
        void BinaryCheck(void);
        void CallCheck(FnDecl *f);

    protected:
        Expr *base; // will be NULL if no explicit base
        Identifier *field;
        List<Expr*> *actuals;
        void DoCheck(void);

    public:
        Call(yyltype loc, Expr *base, Identifier *field,
             List<Expr*> *args);
};

class NewExpr : public Expr
{
    protected:
        NamedType *cType;
        void DoCheck(void);

    public:
        NewExpr(yyltype loc, NamedType *clsType);
};

class NewArrayExpr : public Expr
{
    protected:
        Expr *size;
        Type *elemType;
        void DoCheck(void);

    public:
        NewArrayExpr(yyltype loc, Expr *sizeExpr, Type *elemType);
};

class ReadIntegerExpr : public Expr
{
    public:
        ReadIntegerExpr(yyltype loc);
};

class ReadLineExpr : public Expr
{
    public:
        ReadLineExpr(yyltype loc);
};

#endif
