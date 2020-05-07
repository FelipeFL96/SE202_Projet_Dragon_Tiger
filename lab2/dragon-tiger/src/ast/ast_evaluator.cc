#include "ast_evaluator.hh"
#include "../utils/errors.hh"

namespace ast {

void ASTEvaluator::visit(const IntegerLiteral &node) {
    printf("NODE OF TYPE IntegerLiteral\n");
}

void ASTEvaluator::visit(const StringLiteral &node) {
    printf("NODE OF TYPE StringLiteral\n");
}

void ASTEvaluator::visit(const BinaryOperator &node) {
    printf("NODE OF TYPE BinaryOperator\n");
}

void ASTEvaluator::visit(const Sequence &node) {
    printf("NODE OF TYPE Sequence\n");
}

void ASTEvaluator::visit(const Let &node) {
    printf("NODE OF TYPE Let\n");
}

void ASTEvaluator::visit(const Identifier &node) {
    printf("NODE OF TYPE Identifier\n");
}

void ASTEvaluator::visit(const IfThenElse &node) {
    printf("NODE OF TYPE IfThenElse\n");
}

void ASTEvaluator::visit(const VarDecl &node) {
    printf("NODE OF TYPE VarDecl\n");
}

void ASTEvaluator::visit(const FunDecl &node) {
    printf("NODE OF TYPE FunDecl\n");
}

void ASTEvaluator::visit(const FunCall &node) {
    printf("NODE OF TYPE FunCall\n");
}

void visit(const FunCall &) {
    printf("NODE OF TYPE FunCall\n");
}

void ASTEvaluator::visit(const WhileLoop &node) {
    printf("NODE OF TYPE WhileLoop\n");
}

void ASTEvaluator::visit(const ForLoop &node) {
    printf("NODE OF TYPE ForLoop\n");
}

void ASTEvaluator::visit(const Break &node) {
    printf("NODE OF TYPE Break\n");
}

void ASTEvaluator::visit(const Assign &node) {
    printf("NODE OF TYPE Assign\n");
}


}