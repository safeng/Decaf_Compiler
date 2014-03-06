/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement
 * semantic processing including detection of declaration conflicts
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_type.h"
#include "hashtable.h"
#include "list.h"

class Identifier;
class Stmt;

class Decl : public Node
{
    protected:
        Identifier *id_;

    public:
        Decl(Identifier *id);
        Identifier *get_id(void);
        friend std::ostream& operator<<(std::ostream& out, Decl *d);
};

class VarDecl : public Decl
{
    protected:
        Type *type_;
        void DoCheck(void);

    public:
        VarDecl(Identifier *name, Type *type);
        Type *get_type(void);
};

class ClassDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;
        void MergeSymbolTable(ClassDecl *base);

    protected:
        List<Decl*> *members;
        NamedType *extends;
        List<NamedType*> *implements;
        void DoCheck(void);

    public:
        ClassDecl(Identifier *name, NamedType *extends,
                  List<NamedType*> *implements, List<Decl*> *members);

        Hashtable<Decl*> *get_sym_table(void);
        ClassDecl *GetCurrentClass(void);
        VarDecl *GetMemberVar(char *name);
        FnDecl *GetMemberFn(char *name);

		VarDecl *GetVar(Identifier *id);
		FnDecl *GetFn(Identifier *id);
};

class InterfaceDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;

    protected:
        List<Decl*> *members_;
        void DoCheck(void);

    public:
        InterfaceDecl(Identifier *name, List<Decl*> *members);

        Hashtable<Decl*> *get_sym_table(void);
};

class FnDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;

    protected:
        List<VarDecl*> *formals_;
        Type *returnType_;
        Stmt *body_;
        void DoCheck(void);

    public:
        FnDecl(Identifier *name, Type *returnType,
               List<VarDecl*> *formals);
        Type *get_return_type(void);
        void SetFunctionBody(Stmt *b);
        VarDecl *GetVar(Identifier *id);
};

#endif
