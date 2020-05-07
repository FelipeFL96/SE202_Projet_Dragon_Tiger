#ifndef AST_EVALUATOR_HH
#define AST_EVALUATOR_HH

#include "nodes.hh"

namespace ast {

class ASTEvaluator : public ConstASTVisitor {

public:
    ASTEvaluator() {};
    
    virtual void visit(const IntegerLiteral &);
    virtual void visit(const StringLiteral &);
    virtual void visit(const BinaryOperator &);
    virtual void visit(const Sequence &);
    virtual void visit(const Let &);
    virtual void visit(const Identifier &);
    virtual void visit(const IfThenElse &);
    virtual void visit(const VarDecl &);
    virtual void visit(const FunDecl &);
    virtual void visit(const FunCall &);
    virtual void visit(const WhileLoop &);
    virtual void visit(const ForLoop &);
    virtual void visit(const Break &);
    virtual void visit(const Assign &);
};

}

#endif // _AST_EVALUATOR_HH