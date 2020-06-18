#include "irgen.hh"
#include "../utils/errors.hh"

#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using utils::error;

namespace irgen {

IRGenerator::IRGenerator() : Builder(Context) {
  Mod = llvm::make_unique<llvm::Module>("tiger", Context);
}

llvm::Type *IRGenerator::llvm_type(const ast::Type ast_type) {
  switch (ast_type) {
  case t_int:
    return Builder.getInt32Ty();
  case t_string:
    return Builder.getInt8PtrTy();
  case t_void:
    return Builder.getVoidTy();
  default:
    assert(false); __builtin_unreachable();
  }
}

llvm::Value *IRGenerator::alloca_in_entry(llvm::Type *Ty,
                                          const std::string &name) {
  llvm::IRBuilderBase::InsertPoint const saved = Builder.saveIP();
  Builder.SetInsertPoint(&current_function->getEntryBlock());
  llvm::Value *const value = Builder.CreateAlloca(Ty, nullptr, name);
  Builder.restoreIP(saved);
  return value;
}

void IRGenerator::print_ir(std::ostream *ostream) {
  // FIXME: This is inefficient. Should probably take a filename
  // and use directly LLVM raw stream interface
  std::string buffer;
  llvm::raw_string_ostream OS(buffer);
  OS << *Mod;
  OS.flush();
  *ostream << buffer;
}

llvm::Value *IRGenerator::address_of(const Identifier &id) {
  assert(id.get_decl());
  const VarDecl &decl = dynamic_cast<const VarDecl &>(id.get_decl().get());
  if (id.get_depth() == decl.get_depth()) {
    return allocations[&decl];
  }
  else {
    llvm::Value *frame_address = frame_up(id.get_depth() - decl.get_depth()).second;
    llvm::Value *decl_address = Builder.CreateStructGEP(frame_address, frame_position[&id.get_decl().get()]);
    return decl_address;
  }
}

void IRGenerator::generate_program(FunDecl *main) {
  main->accept(*this);

  while (!pending_func_bodies.empty()) {
    generate_function(*pending_func_bodies.back());
    pending_func_bodies.pop_back();
  }
}

void IRGenerator::generate_function(const FunDecl &decl) {
  // Reinitialize common structures.
  allocations.clear();
  loop_exit_bbs.clear();

  // Set current function
  current_function = Mod->getFunction(decl.get_external_name().get());
  current_function_decl = &decl;
  std::vector<VarDecl *> params = decl.get_params();

  // Create a new basic block to insert allocation insertion
  llvm::BasicBlock *bb1 =
      llvm::BasicBlock::Create(Context, "entry", current_function);

  // Generate a frame structure to the function
  generate_frame();

  // Create a second basic block for body insertion
  llvm::BasicBlock *bb2 =
      llvm::BasicBlock::Create(Context, "body", current_function);

  Builder.SetInsertPoint(bb2);

  // Set the name for each argument and register it in the allocations map
  // after storing it in an alloca.
  unsigned i = 0;
  for (auto &arg : current_function->args()) {
    if (!decl.is_external && &arg == current_function->args().begin()) {
      arg.setName("sl");
      Builder.CreateStore(&arg, Builder.CreateStructGEP(frame, 0));
      continue;
    }
    arg.setName(params[i]->name.get());
    llvm::Value *const shadow = generate_vardecl(*params[i]);
    Builder.CreateStore(&arg, shadow);
    i++;
  }

  // Visit the body
  llvm::Value *expr = decl.get_expr()->accept(*this);

  // Finish off the function.
  if (decl.get_type() == t_void)
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(expr);

  // Jump from entry to body
  Builder.SetInsertPoint(bb1);
  Builder.CreateBr(bb2);

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*current_function);
}

void IRGenerator::generate_frame() {
  std::vector<llvm::Type*> escaping_types;

  if (current_function_decl->get_parent()) {
    llvm::PointerType *parent_frame = frame_type[&current_function_decl->get_parent().get()]->getPointerTo();
    escaping_types.push_back(parent_frame);
  }

  for (auto esc : current_function_decl->get_escaping_decls()) {
    if (esc->get_type() != t_void)
      escaping_types.push_back(llvm_type(esc->get_type()));
  }

  llvm::StructType *frame_structure =
    llvm::StructType::create(Context, escaping_types, "ft_" + current_function_decl->get_external_name().get());

  frame_type[current_function_decl] = frame_structure;
  frame = alloca_in_entry(frame_structure, "frame");
}

std::pair<llvm::StructType *, llvm::Value *> IRGenerator::frame_up(int levels) {
  FunDecl const* fun = current_function_decl;
  llvm::Value *sl = frame;

  for (int i = 0; i < levels; i++) {
    fun = &fun->get_parent().get();
    sl = Builder.CreateLoad(Builder.CreateStructGEP(sl, 0));
  }

  std::pair<llvm::StructType *, llvm::Value *> frame_info(frame_type[fun], sl);
  return frame_info;
}

llvm::Value *IRGenerator::generate_vardecl(const VarDecl &decl) {
  if (decl.get_escapes()) {
    int pos = 0;

    if (current_function_decl->get_parent()) {
      pos++;
    }
    for (auto esc : current_function_decl->get_escaping_decls()) {
      if (esc == &decl)
        break;
      if (esc->get_type() == t_void)
        continue;
      pos++;
    }
    frame_position[&decl] = pos;

    llvm::Value *decl_address = Builder.CreateStructGEP(frame, pos);

    allocations[&decl] = decl_address;
    return decl_address;
  }
  else {
    llvm::Value *decl_address = alloca_in_entry(llvm_type(decl.get_type()), decl.name);
    allocations[&decl] = decl_address;
    return decl_address;
  }
}




} // namespace irgen
