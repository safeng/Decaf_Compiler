/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct.
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */


#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"
#include "hashtable.h"

class Decl;
class FnDecl;
class LengthFn;
class VarDecl;
class ClassDecl;
class InterfaceDecl;
class NamedType;
class Expr;

class Program : public Node
{
    private:
        Hashtable<Decl*> *sym_table_;

    protected:
        List<Decl*> *decls_;
        void DoCheck(void);

    public:
        Program(List<Decl*> *decls);

        ClassDecl *GetClass(NamedType *t);
        FnDecl *GetFn(Identifier *id);
        VarDecl *GetVar(Identifier *id);
        InterfaceDecl *GetInterface(NamedType *t);
};

class Stmt : public Node
{
	protected:
		virtual Stmt *GetContextStmt(void); // Get context of stmt of interest
    public:
        Stmt(void);
        Stmt(yyltype loc);
};

class StmtBlock : public Stmt
{
    private:
        Hashtable<Decl*> *sym_;

    protected:
        List<VarDecl*> *decls;
        List<Stmt*> *stmts;
        void DoCheck(void);

    public:
        StmtBlock(List<VarDecl*> *variableDeclarations,
                  List<Stmt*> *statements);

        VarDecl *GetVar(Identifier *id);
};

class ConditionalStmt : public Stmt
{
    protected:
        Expr *test;
        Stmt *body;
        void DoCheck(void); // test testExpr is of boolean type

    public:
        ConditionalStmt(Expr *testExpr, Stmt *body);
};

class LoopStmt : public ConditionalStmt
{
	protected:
		Stmt *GetContextStmt(void); // Return LoopStmt

    public:
        LoopStmt(Expr *testExpr, Stmt *body);
};

class ForStmt : public LoopStmt
{
    protected:
        Expr *init, *step;
        void DoCheck(void);

    public:
        ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
};

class WhileStmt : public LoopStmt
{
    public:
        WhileStmt(Expr *test, Stmt *body);
};

class IfStmt : public ConditionalStmt
{
    protected:
        void DoCheck(void);

    protected:
        Stmt *elseBody;

    public:
        IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
};

class BreakStmt : public Stmt
{
	protected:
		void DoCheck(void); // break stmt can only appear within while or for loop

    public:
        BreakStmt(yyltype loc);
};

class ReturnStmt : public Stmt
{
    protected:
        void DoCheck(void);

    protected:
        Expr *expr;

    public:
        ReturnStmt(yyltype loc, Expr *expr);
};

class PrintStmt : public Stmt
{
    protected:
        List<Expr*> *args;
        void DoCheck(void);

    public:
        PrintStmt(List<Expr*> *arguments);
};

#endif
