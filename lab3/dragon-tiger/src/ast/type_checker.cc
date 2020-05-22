#include "type_checker.hh"
#include "../utils/errors.hh"

namespace ast {
namespace type_checker {

TypeChecker::TypeChecker() {}

void TypeChecker::type_check(FunDecl *main) {
    main->accept(*this);
}

void TypeChecker::visit(IntegerLiteral &literal) {
}

void TypeChecker::visit(StringLiteral &literal) {
}

void TypeChecker::visit(BinaryOperator &op) {
}

void TypeChecker::visit(Sequence &seq) {
}

void TypeChecker::visit(Let &let) {
}

void TypeChecker::visit(Identifier &id) {;
}

void TypeChecker::visit(IfThenElse &ite) {
}

void TypeChecker::visit(VarDecl &decl) {
}

void TypeChecker::visit(FunDecl &decl) {
}

void TypeChecker::visit(FunCall &call) {
}

void TypeChecker::visit(WhileLoop &loop) {
}

void TypeChecker::visit(ForLoop &loop) {
}


void TypeChecker::visit(Break &b) {
}

void TypeChecker::visit(Assign &assign) {
}

}
}