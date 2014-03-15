/*** ast_decl.h - ASTs of declarations *******************************/

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "hashtable.h"
#include "list.h"

class Identifier;
class Stmt;

class Decl : public Node
{
    protected:
        Identifier *id_;

    public:
        Decl(Identifier *i);

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
        ClassDecl *GetClass(NamedType *t);
        VarDecl *GetMemberVar(char *name);
        VarDecl *GetVar(Identifier *i);
        InterfaceDecl *GetInterface(NamedType *t);
        FnDecl *GetMemberFn(char *name);
        FnDecl *GetFn(Identifier *i);
        bool IsTypeCompatibleWith(NamedType *baseClass);
        bool IsSubsetOf(NamedType *t);
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

        Hashtable<Decl*> *sym_table(void);

        FnDecl *GetMemberFn(char *name);
        FnDecl *GetFn(Identifier *i);
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
        List<VarDecl*> *formals(void);
        void set_body(Stmt *b);

        ClassDecl *GetClass(NamedType *t);
        VarDecl *GetVar(Identifier *i);
        InterfaceDecl *GetInterface(NamedType *t);
        FnDecl *GetFn(Identifier *i);
        bool IsSigEquivalentTo(FnDecl *other);
        void CheckCallCompatibility(Identifier *caller,
                                    List<Expr*> *actuals);

};

class LengthFn : public FnDecl
{
    public:
        LengthFn(yyltype loc);
};

#endif
