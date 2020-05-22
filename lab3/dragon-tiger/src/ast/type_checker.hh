#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include "nodes.hh"
#define debug(X) std::cout<<X<<std::endl

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor {

public:
  TypeChecker();
  void type_check(FunDecl *main);
  FunDecl *analyze_program(Expr &);
  virtual void visit(IntegerLiteral &);
  virtual void visit(StringLiteral &);
  virtual void visit(BinaryOperator &);
  virtual void visit(Sequence &);
  virtual void visit(Let &);
  virtual void visit(Identifier &);
  virtual void visit(IfThenElse &);
  virtual void visit(VarDecl &);
  virtual void visit(FunDecl &);
  virtual void visit(FunCall &);
  virtual void visit(WhileLoop &);
  virtual void visit(ForLoop &);
  virtual void visit(Break &);
  virtual void visit(Assign &);
};

} // namespace ast
} // namespace type_checker

#endif //TYPE_CHECKER_HH