#include <cstdlib>  // For exit
#include <iostream> // For std::cerr
#include "irgen.hh"

#include "llvm/Support/raw_ostream.h"

namespace irgen {

llvm::Value *IRGenerator::visit(const IntegerLiteral &literal) {
  return Builder.getInt32(literal.value);
}

llvm::Value *IRGenerator::visit(const StringLiteral &literal) {
  return Builder.CreateGlobalStringPtr(literal.value.get());
}

llvm::Value *IRGenerator::visit(const Break &b) {
  // In case there is unreachable code placed just after a break
  llvm::BasicBlock *after_break =
    llvm::BasicBlock::Create(Context, "break_deprecated", current_function);

  Builder.CreateBr(loop_exit_bbs[&b.get_loop().get()]);

  Builder.SetInsertPoint(after_break);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const BinaryOperator &op) {
  // Void values can be compared for equality only. We directly
  // return 1 or 0 depending on the equality/inequality operator.
  if (op.get_left().get_type() == t_void) {
    return Builder.getInt32(op.op == o_eq);
  }

  llvm::Value *l = op.get_left().accept(*this);
  llvm::Value *r = op.get_right().accept(*this);

  if (op.get_left().get_type() == t_string) {
    auto const strcmp = Mod->getOrInsertFunction("__strcmp", Builder.getInt32Ty(),
        Builder.getInt8PtrTy(), Builder.getInt8PtrTy()
#if LLVM_VERSION_MAJOR < 5
        , nullptr  // Older versions of LLVM uses C-vararg with a NULL end sigil
#endif // LLVM_MAJOR_VERSION < 5
        );
    l = Builder.CreateCall(strcmp, {l, r});
    r = Builder.getInt32(0);
  }

  switch(op.op) {
    case o_plus: return Builder.CreateBinOp(llvm::Instruction::Add, l, r);
    case o_minus: return Builder.CreateBinOp(llvm::Instruction::Sub, l, r);
    case o_times: return Builder.CreateBinOp(llvm::Instruction::Mul, l, r);
    case o_divide: return Builder.CreateBinOp(llvm::Instruction::SDiv, l, r);
    default: break;
  }

  // Comparisons return an i1 result which needs to be
  // casted to i32, as Tiger might use that as an integer.
  llvm::Value *cmp;

  switch(op.op) {
    case o_eq: cmp = Builder.CreateICmpEQ(l, r); break;
    case o_neq: cmp = Builder.CreateICmpNE(l, r); break;
    case o_gt: cmp = Builder.CreateICmpSGT(l, r); break;
    case o_lt: cmp = Builder.CreateICmpSLT(l, r); break;
    case o_ge: cmp = Builder.CreateICmpSGE(l, r); break;
    case o_le: cmp = Builder.CreateICmpSLE(l, r); break;
    default: assert(false); __builtin_unreachable();
  }

  return Builder.CreateIntCast(cmp, Builder.getInt32Ty(), false);
}

llvm::Value *IRGenerator::visit(const Sequence &seq) {
  llvm::Value *result = nullptr;
  for (auto expr : seq.get_exprs())
    result = expr->accept(*this);
  // An empty sequence should return () but the result
  // will never be used anyway, so nullptr is fine.

  return result;
}

llvm::Value *IRGenerator::visit(const Let &let) {
  for (auto decl : let.get_decls())
    decl->accept(*this);

  return let.get_sequence().accept(*this);
}

llvm::Value *IRGenerator::visit(const Identifier &id) {
  if (id.get_decl()->get_type() == t_void) {
    return nullptr;
  }
  llvm::Value *reg = Builder.CreateLoad(address_of(id));
  return reg;
}

llvm::Value *IRGenerator::visit(const IfThenElse &ite) {
  llvm::BasicBlock *const if_then =
      llvm::BasicBlock::Create(Context, "if_then", current_function);
  llvm::BasicBlock *const if_else =
      llvm::BasicBlock::Create(Context, "if_else", current_function);
  llvm::BasicBlock *const if_end =
      llvm::BasicBlock::Create(Context, "if_end", current_function);

  bool void_ite = ite.get_then_part().get_type() == t_void;

  llvm::Value *result;
  llvm::Value *value;

  if (!void_ite)
    result = alloca_in_entry(llvm_type(t_int), "result");

  Builder.CreateCondBr(Builder.CreateICmpNE(ite.get_condition().accept(*this), Builder.getInt32(0)),
                        if_then, if_else);

  Builder.SetInsertPoint(if_then);
  value = ite.get_then_part().accept(*this);
  if (!void_ite)
    Builder.CreateStore(value, result);
  Builder.CreateBr(if_end);

  Builder.SetInsertPoint(if_else);
  value = ite.get_else_part().accept(*this);
  if (!void_ite)
    Builder.CreateStore(value, result);
  Builder.CreateBr(if_end);

  Builder.SetInsertPoint(if_end);

  if (!void_ite)
    return Builder.CreateLoad(result);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const VarDecl &decl) {
  if (decl.get_type() == t_void) {
    decl.get_expr()->accept(*this);
    return nullptr;
  }

  llvm::Value *variable = generate_vardecl(decl);
  if (decl.get_expr()) {
    llvm::Value *value = decl.get_expr()->accept(*this);
    Builder.CreateStore(value, variable);
  }

  return variable;
}

llvm::Value *IRGenerator::visit(const FunDecl &decl) {
  std::vector<llvm::Type *> param_types;

  if (!decl.is_external) {
    param_types.push_back(frame_type[&decl.get_parent().get()]->getPointerTo());
  }
  for (auto param_decl : decl.get_params()) {
    param_types.push_back(llvm_type(param_decl->get_type()));
  }

  llvm::Type *return_type = llvm_type(decl.get_type());

  llvm::FunctionType *ft =
      llvm::FunctionType::get(return_type, param_types, false);

  llvm::Function::Create(ft,
                         decl.is_external ? llvm::Function::ExternalLinkage
                                          : llvm::Function::InternalLinkage,
                         decl.get_external_name().get(), Mod.get());

  if (decl.get_expr())
    pending_func_bodies.push_front(&decl);

  return nullptr;
}

llvm::Value *IRGenerator::visit(const FunCall &call) {
  // Look up the name in the global module table.
  const FunDecl &decl = call.get_decl().get();
  llvm::Function *callee =
      Mod->getFunction(decl.get_external_name().get());

  if (!callee) {
    // This should only happen for primitives whose Decl is out of the AST
    // and has not yet been handled
    assert(!decl.get_expr());
    decl.accept(*this);
    callee = Mod->getFunction(decl.get_external_name().get());
  }

  std::vector<llvm::Value *> args_values;
  if (!decl.is_external) {
    args_values.push_back(frame_up(call.get_depth() - decl.get_depth()).second);
  }
  for (auto expr : call.get_args()) {
    args_values.push_back(expr->accept(*this));
  }

  if (decl.get_type() == t_void) {
    Builder.CreateCall(callee, args_values);
    return nullptr;
  }
  return Builder.CreateCall(callee, args_values, "call");
}

llvm::Value *IRGenerator::visit(const WhileLoop &loop) {
  llvm::BasicBlock *const test_block =
    llvm::BasicBlock::Create(Context, "loop_test", current_function);
  llvm::BasicBlock *const body_block =
    llvm::BasicBlock::Create(Context, "loop_body", current_function);
  llvm::BasicBlock *const end_block =
    llvm::BasicBlock::Create(Context, "loop_end", current_function);

  loop_exit_bbs[&loop] = end_block;

  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(test_block);
  Builder.CreateCondBr(Builder.CreateICmpNE(loop.get_condition().accept(*this), Builder.getInt32(0)),
  body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const ForLoop &loop) {
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "loop_test", current_function);
  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "loop_body", current_function);
  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "loop_end", current_function);
  llvm::Value *const index = loop.get_variable().accept(*this);
  llvm::Value *const high = loop.get_high().accept(*this);

  loop_exit_bbs[&loop] = end_block;

  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(test_block);
  Builder.CreateCondBr(Builder.CreateICmpSLE(Builder.CreateLoad(index), high),
                       body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  Builder.CreateStore(
      Builder.CreateAdd(Builder.CreateLoad(index), Builder.getInt32(1)), index);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Assign &assign) {
  llvm::Value *value = assign.get_rhs().accept(*this);
  if (assign.get_lhs().get_decl()->get_type() != t_void) {
    Builder.CreateStore(value, address_of(assign.get_lhs()));
  }
  return nullptr;
}

} // namespace irgen
