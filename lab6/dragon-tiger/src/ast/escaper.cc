#include "escaper.hh"

namespace ast {
namespace escaper {

Escaper::Escaper() {}

void Escaper::escape_decls(FunDecl *main) {
    main->accept(*this);
}

void Escaper::visit(IntegerLiteral &literal) {
}

void Escaper::visit(StringLiteral &literal) {
}

void Escaper::visit(BinaryOperator &op) {
    op.get_left().accept(*this);
    op.get_right().accept(*this);
}

void Escaper::visit(Sequence &seq) {
    for (auto expr : seq.get_exprs()) {
        expr->accept(*this);
    }
}

void Escaper::visit(Let &let) {
    for (auto decl : let.get_decls()) {
        decl->accept(*this);
    }
    let.get_sequence().accept(*this);
}

void Escaper::visit(Identifier &id) {
}

void Escaper::visit(IfThenElse &ite) {
    ite.get_condition().accept(*this);
    ite.get_then_part().accept(*this);
    ite.get_else_part().accept(*this);
}

void Escaper::visit(VarDecl &decl) {
    if (decl.get_escapes()) {
        current_function->get_escaping_decls().push_back(&decl);
    }
    if (decl.get_expr()) {
        decl.get_expr()->accept(*this);
    }
}

void Escaper::visit(FunDecl &decl) {
    current_function =  &decl;
    for (auto param : decl.get_params()) {
        param->accept(*this);
    }
    decl.get_expr()->accept(*this);
}

void Escaper::visit(FunCall &call) {
    for (auto arg : call.get_args()) {
        arg->accept(*this);
    }
}

void Escaper::visit(WhileLoop &loop) {
    loop.get_condition().accept(*this);
    loop.get_body().accept(*this);
}

void Escaper::visit(ForLoop &loop) {
    loop.get_variable().accept(*this);
    loop.get_high().accept(*this);
    loop.get_body().accept(*this);
}

void Escaper::visit(Break &b) {
}

void Escaper::visit(Assign &assign) {
    assign.get_lhs().accept(*this);
    assign.get_rhs().accept(*this);
}


} // namespace escaper
} // namespace ast