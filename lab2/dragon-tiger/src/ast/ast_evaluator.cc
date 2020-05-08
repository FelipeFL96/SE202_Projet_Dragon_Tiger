#include "ast_evaluator.hh"
#include "../utils/errors.hh"

namespace ast {

void ASTEvaluator::visit(const IntegerLiteral &literal) {
    if (expr_level == 0){
        *ostream << literal.value;
    }
    else {
        expr_values.push(literal.value);
    }
}

void ASTEvaluator::visit(const StringLiteral &literal) {
    printf("NODE OF TYPE StringLiteral\n");
}

void ASTEvaluator::visit(const BinaryOperator &binop) {
    int val_expr_left, val_expr_right, val;

    expr_level++;

    binop.get_left().accept(*this);
    val_expr_left = expr_values.top();
    expr_values.pop();

    binop.get_right().accept(*this);
    val_expr_right = expr_values.top();
    expr_values.pop();

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
            if (val_expr_right == 0)
                utils::error("evaluator: attempt to divide by zero");
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

    expr_level--;

    if (expr_level == 0) {
        *ostream << val;
    }
    else {
        expr_values.push(val);
    }
}

void ASTEvaluator::visit(const Sequence &seqExpr) {
    int val;

    const auto exprs = seqExpr.get_exprs();

    if (exprs.empty())
        utils::error("ast_evaluator: invalid empty sequence");

    expr_level++;

    for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
        (*expr)->accept(*this);
        if (!expr_values.empty()) {
            val = expr_values.top();
            expr_values.pop();
        }
    }

    expr_level--;

    if (expr_level == 0) {
        *ostream << val;
    }
    else {
        expr_values.push(val);
    }
}

void ASTEvaluator::visit(const Let &let) {
    utils::error("ast_evaluator: invalid node type for evaluation (let)");
}

void ASTEvaluator::visit(const Identifier &id) {
    utils::error("ast_evaluator: invalid node type for evaluation (identifier)");
}

void ASTEvaluator::visit(const IfThenElse &ite) {
    int val_expr_cond, val;

    expr_level++;

    ite.get_condition().accept(*this);
    val_expr_cond = expr_values.top();
    expr_values.pop();

    if (val_expr_cond) {
        ite.get_then_part().accept(*this);
        val = expr_values.top();
        expr_values.pop();
    }
    else {
        ite.get_else_part().accept(*this);
        val = expr_values.top();
        expr_values.pop();
    }

    expr_level--;

    if (expr_level == 0) {
        *ostream << val;
    }
    else {
        expr_values.push(val);
    }
}

void ASTEvaluator::visit(const VarDecl &decl) {
    utils::error("ast_evaluator: invalid node type for evaluation (variable declaration)");
}

void ASTEvaluator::visit(const FunDecl &decl) {
    utils::error("ast_evaluator: invalid node type for evaluation (function declaration)");
}

void ASTEvaluator::visit(const FunCall &call) {
    utils::error("ast_evaluator: invalid node type for evaluation (function call)");
}

void ASTEvaluator::visit(const WhileLoop &loop) {
    utils::error("ast_evaluator: invalid node type for evaluation (while loop)");
}

void ASTEvaluator::visit(const ForLoop &loop) {
    utils::error("ast_evaluator: invalid node type for evaluation (for loop)");
}

void ASTEvaluator::visit(const Break &brk) {
    utils::error("ast_evaluator: invalid node type for evaluation (break)");
}

void ASTEvaluator::visit(const Assign &assign) {
    utils::error("ast_evaluator: invalid node type for evaluation (assignment)");
}


}