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
    ite.get_condition().accept(*this);
    ite.get_then_part().accept(*this);
    ite.get_else_part().accept(*this);
    if (ite.get_then_part().get_type() ==  ite.get_else_part().get_type()) {
        ite.set_type(ite.get_then_part().get_type());
    }
    else {
        utils::error("different return types for 'then' and 'else'");
    }
}

void TypeChecker::visit(VarDecl &decl) {
    if (decl.get_expr())
        decl.get_expr()->accept(*this);
    if (decl.type_name) {
        Symbol type = decl.type_name.get();
        if (!decl.get_expr())
            type == Symbol("int") ? decl.set_type(t_int) : decl.set_type(t_string);
        else if ((type == Symbol("int")) && (decl.get_expr()->get_type() == t_int))
            decl.set_type(t_int);
        else if((type == Symbol("string")) && (decl.get_expr()->get_type() == t_string))
            decl.set_type(t_string);
        else
            utils::error(decl.loc, "declared type '" + decl.type_name.get().get()
                + "' doesn't match with expression type");
    }
    else {
        decl.set_type(decl.get_expr()->get_type());
    }
}

void TypeChecker::visit(FunDecl &decl) {
    for (auto param : decl.get_params())
        param->accept(*this);
    decl.get_expr()->accept(*this);
}

void TypeChecker::visit(FunCall &call) {
    FunDecl &decl = call.get_decl().get();

    if (call.get_args().size() != decl.get_params().size()) {
        utils::error(call.loc, "function call lacking parameters");
    }
    for (auto arg : call.get_args())
        arg->accept(*this);
    for (int i = 0; i < decl.get_params().size(); i++) {
        if (call.get_args().at(i)->get_type() != decl.get_params().at(i)->get_type()) {
            VarDecl *param = decl.get_params().at(i);
            Expr *arg = call.get_args().at(i);
            utils::error(arg->loc, "argument type differs from expected '" + param->name.get() + "' parameter type");
        }
    }
    call.set_type(decl.get_type());
}

void TypeChecker::visit(WhileLoop &loop) {
    loop.get_condition().accept(*this);
    loop.get_body().accept(*this);
    if (loop.get_condition().get_type() != t_int) {
        utils::error(loop.loc, "loop condition must be an 'int' type expression");
    }
    if (loop.get_body().get_type() != t_void) {
        utils::error(loop.loc, "loop body must be of type void");
    }
    loop.set_type(t_void);
}

void TypeChecker::visit(ForLoop &loop) {
    loop.get_variable().accept(*this);
    loop.get_high().accept(*this);
    loop.get_body().accept(*this);
    if ((loop.get_variable().get_type() != t_int) || (loop.get_high().get_type() != t_int)) {
        utils::error(loop.loc, "loop bounds must be of type 'int'");
    }
    if (loop.get_body().get_type() != t_void) {
        utils::error(loop.loc, "loop body must be of type void");
    }
    loop.set_type(t_void);
}


void TypeChecker::visit(Break &b) {
    b.set_type(t_void);
}

void TypeChecker::visit(Assign &assign) {
    assign.get_lhs().accept(*this);
    assign.get_rhs().accept(*this);
    if ( assign.get_lhs().get_type() != assign.get_rhs().get_type()) {
        utils::error(assign.loc, "assigned value and variable must be of the same type");
    }
    assign.set_type(t_void);
}

}
}