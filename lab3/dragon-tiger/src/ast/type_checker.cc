#include "type_checker.hh"
#include "../utils/errors.hh"

namespace ast {
namespace type_checker {

TypeChecker::TypeChecker() {}

void TypeChecker::type_check(FunDecl *main) {
    main->accept(*this);
}

void TypeChecker::visit(IntegerLiteral &literal) {
    literal.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &literal) {
    literal.set_type(t_string);
}

void TypeChecker::visit(BinaryOperator &op) {
}

void TypeChecker::visit(Sequence &seq) {
    for (auto expr : seq.get_exprs())
        expr->accept(*this);
    if (seq.get_exprs().empty()) {
        seq.set_type(t_void);
    }
    else {
        seq.set_type(seq.get_exprs().back()->get_type());
    }
}

void TypeChecker::visit(Let &let) {
    for (auto decl : let.get_decls())
        decl->accept(*this);
    let.get_sequence().accept(*this);
    let.set_type(let.get_sequence().get_type());
}

void TypeChecker::visit(Identifier &id) {
    id.set_type(id.get_decl()->get_type());
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
    b.set_type(t_void);
}

void TypeChecker::visit(Assign &assign) {
}

}
}