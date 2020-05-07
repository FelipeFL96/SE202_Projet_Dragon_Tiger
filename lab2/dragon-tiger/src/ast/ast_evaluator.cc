#include "ast_evaluator.hh"
#include "../utils/errors.hh"

namespace ast {

void ASTEvaluator::visit(const IntegerLiteral &literal) {
    if (value == 0){
        output(literal.value);
    }
    else {
        values.push(literal.value);
    }
}

void ASTEvaluator::visit(const StringLiteral &literal) {
    printf("NODE OF TYPE StringLiteral\n");
}

void ASTEvaluator::visit(const BinaryOperator &binop) {
    int val_expr_left, val_expr_right, val;

    value++;

    binop.get_left().accept(*this);
    val_expr_left = values.top();
    values.pop();

    binop.get_right().accept(*this);
    val_expr_right = values.top();
    values.pop();

    switch (binop.op) {
        case Operator::o_plus:
            val = val_expr_left + val_expr_right;
            break;
        case Operator::o_minus:
            val = val_expr_left - val_expr_right;
            break;
        case Operator::o_times:
            val = val_expr_left * val_expr_right;
            break;
        case Operator::o_divide:
            val = val_expr_left / val_expr_right;
            break;
        case Operator::o_eq:
            val = val_expr_left == val_expr_right;
            break;
        case Operator::o_neq:
            val = val_expr_left != val_expr_right;
            break;
        case Operator::o_lt:
            val = (val_expr_left < val_expr_right) ? 1 : 0;
            break;
        case Operator::o_le:
            val = (val_expr_left <= val_expr_right) ? 1 : 0;
            break;
        case Operator::o_gt:
            val = (val_expr_left > val_expr_right) ? 1 : 0;
            break;
        case Operator::o_ge:
            val = (val_expr_left >= val_expr_right) ? 1 : 0;
            break;
    }

    value--;

    if (value == 0) {
        output(val);
    }
    else {
        values.push(val);
    }
}

void ASTEvaluator::visit(const Sequence &seqExpr) {
    int val;

    const auto exprs = seqExpr.get_exprs();

    if (exprs.empty())
        utils::error("fatal: invalid empty sequence");

    value++;

    for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
        (*expr)->accept(*this);
        if (!values.empty()) {
            val = values.top();
            values.pop();
        }
    }

    value--;

    if (value == 0) {
        output(val);
    }
    else {
        values.push(val);
    }
}

void ASTEvaluator::visit(const Let &let) {
    printf("NODE OF TYPE Let\n");
}

void ASTEvaluator::visit(const Identifier &id) {
    printf("NODE OF TYPE Identifier\n");
}

void ASTEvaluator::visit(const IfThenElse &ite) {
    int val_expr_cond, val;

    value++;

    ite.get_condition().accept(*this);
    val_expr_cond = values.top();
    values.pop();

    if (val_expr_cond) {
        ite.get_then_part().accept(*this);
        val = values.top();
        values.pop();
    }
    else {
        ite.get_else_part().accept(*this);
        val = values.top();
        values.pop();
    }

    value--;

    if (value == 0) {
        output(val);
    }
    else {
        values.push(val);
    }
}

void ASTEvaluator::visit(const VarDecl &decl) {
    printf("NODE OF TYPE VarDecl\n");
}

void ASTEvaluator::visit(const FunDecl &decl) {
    printf("NODE OF TYPE FunDecl\n");
}

void ASTEvaluator::visit(const FunCall &call) {
    printf("NODE OF TYPE FunCall\n");
}

void ASTEvaluator::visit(const WhileLoop &loop) {
    printf("NODE OF TYPE WhileLoop\n");
}

void ASTEvaluator::visit(const ForLoop &loop) {
    printf("NODE OF TYPE ForLoop\n");
}

void ASTEvaluator::visit(const Break &brk) {
    printf("NODE OF TYPE Break\n");
}

void ASTEvaluator::visit(const Assign &assign) {
    printf("NODE OF TYPE Assign\n");
}


}