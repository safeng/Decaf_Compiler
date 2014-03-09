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
        Identifier *id(void);
        friend std::ostream& operator<<(std::ostream& out, Decl *d);
};

class VarDecl : public Decl
{
    protected:
        Type *type_;
        void DoCheck(void);

    public:
        VarDecl(Identifier *name, Type *type);
        Type *type(void);
};

class ClassDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;
        void MergeSymbolTable(ClassDecl *base);

    protected:
        List<Decl*> *members_;
        NamedType *extends_;
        List<NamedType*> *implements_;
        void DoCheck(void);

    public:
        ClassDecl(Identifier *n, NamedType *ext,
                  List<NamedType*> *impl, List<Decl*> *memb);

        Hashtable<Decl*> *sym_table(void);

        ClassDecl *GetCurrentClass(void);
        VarDecl *GetMemberVar(char *name);
        FnDecl *GetMemberFn(char *name);
        VarDecl *GetVar(Identifier *id);
        FnDecl *GetFn(Identifier *id);

        // test whether this class is compatible with baseClass
        bool IsTypeCompatibleWith(NamedType *t);
};

class InterfaceDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;

    protected:
        List<Decl*> *members_;
        void DoCheck(void);

    public:
        InterfaceDecl(Identifier *n, List<Decl*> *memb);

        Hashtable<Decl*> *namesym_table(void);
};

class FnDecl : public Decl
{
    private:
        Hashtable<Decl*> *sym_table_;

    protected:
        List<VarDecl*> *formals_;
        Type *return_type_;
        Stmt *body_;
        void DoCheck(void);
        FnDecl *GetCurrentFn(void);

    public:
        FnDecl(Identifier *n, Type *ret, List<VarDecl*> *form);

        Type *return_type(void);
        void set_body(Stmt *b);

        VarDecl *GetVar(Identifier *id);
};

#endif
