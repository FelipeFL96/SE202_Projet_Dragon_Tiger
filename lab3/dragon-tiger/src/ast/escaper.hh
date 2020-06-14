#ifndef AST_ESCAPER_HH
#define AST_ESCAPER_HH

#include "nodes.hh"

namespace ast {
namespace escaper {

class Escaper : public ASTVisitor {

  std::vector<FunDecl *> functions;

public:
  Escaper();
  void escape_decls(FunDecl *main);
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


} // namespace escaper
} // namespace ast

#endif // AST_ESCAPER_HH