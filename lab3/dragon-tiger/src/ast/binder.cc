#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  const Symbol s_int = Symbol("int");
  const Symbol s_string = Symbol("string");
  enter_primitive("print_err", boost::none, {s_string});
  enter_primitive("print", boost::none, {s_string});
  enter_primitive("print_int", boost::none, {s_int});
  enter_primitive("flush", boost::none, {});
  enter_primitive("getchar", s_string, {});
  enter_primitive("ord", s_int, {s_string});
  enter_primitive("chr", s_string, {s_int});
  enter_primitive("size", s_int, {s_string});
  enter_primitive("substring", s_string, {s_string, s_int, s_int});
  enter_primitive("concat", s_string, {s_string, s_string});
  enter_primitive("strcmp", s_int, {s_string, s_string});
  enter_primitive("streq", s_int, {s_string, s_string});
  enter_primitive("not", s_int, {s_int});
  enter_primitive("exit", boost::none, {s_int});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const boost::optional<Symbol> &type_name,
    const std::vector<Symbol> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const Symbol &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), nullptr, tn));
  }

  boost::optional<Symbol> type_name_symbol = boost::none;
  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), std::move(args), nullptr,
                            type_name, true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Verifies if a given variable declaration corresponds to a for-loop index */
bool Binder::is_loop_index(VarDecl *var) {
  for (auto index : loop_indexes)
    if (var == index)
      return true;
  return false;
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), main_params,
                                    main_body, Symbol("int"), true);
  main->accept(*this);
  return main;
}


void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op) {
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq) {
  for (auto expr : seq.get_exprs()) {
    expr->accept(*this);
  }
}

void Binder::visit(Let &let) {
  push_scope();
  std::vector<FunDecl *> consecutive_functions;

  // Analysing declarations in Let
  for (auto decl : let.get_decls()) {
    if (dynamic_cast<FunDecl *>(decl)) { // Making consecutive function declarations visible in the scope
      enter(*decl);
      consecutive_functions.push_back(&dynamic_cast<FunDecl&>(*decl));
    }
    else {
      // Analysing consecutive function declaration blocks
      if (!consecutive_functions.empty()) {
        for (auto fun_decl : consecutive_functions)
          fun_decl->accept(*this);
        consecutive_functions.clear();
      }

      decl->accept(*this);
    }
  }
  // Analysing possible lasting consecutive function declaration block
  if (!consecutive_functions.empty()) {
    for (auto fun_decl : consecutive_functions)
      fun_decl->accept(*this);
    consecutive_functions.clear();
  }

  // Analysing expressions in Let
  let.get_sequence().accept(*this);

  pop_scope();
}

void Binder::visit(Identifier &id) {
  if (!id.get_decl()) {
    try {
      id.set_decl(&dynamic_cast<VarDecl&>(find(id.loc, id.name)));
    }
    catch (std::bad_cast) {
      utils::error(id.loc, "invalid reference to function in expression");
    }
    id.set_depth(functions.size() - 1);
    if (id.get_depth() != id.get_decl()->get_depth()) {
      id.get_decl()->set_escapes();
    }
  }
}

void Binder::visit(IfThenElse &ite) {
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);
}

void Binder::visit(VarDecl &decl) {
  if (!is_loop_index(&decl))
    variable_declaration = true;
  if (decl.get_expr())
    decl.get_expr()->accept(*this);
  variable_declaration = false;
  enter(decl);
  decl.set_depth(functions.size() - 1);
}

void Binder::visit(FunDecl &decl) {
  set_parent_and_external_name(decl);
  functions.push_back(&decl);
  /* ... put your code here ... */
  decl.set_depth(functions.size() - 1);

  push_scope();
  for (auto param : decl.get_params()) {
    param->accept(*this);
  }
  decl.get_expr()->accept(*this);
  pop_scope();

  functions.pop_back();
}

void Binder::visit(FunCall &call) {
  call.set_decl(&dynamic_cast<FunDecl&>(find(call.loc, call.func_name)));
  call.set_depth(functions.size() - 1);
  for (auto arg : call.get_args()) {
    arg->accept(*this);
  }
}

void Binder::visit(WhileLoop &loop) {
  loop.get_condition().accept(*this);
  loops.push_back(&loop);
  loop.get_body().accept(*this);
  loops.pop_back();
}

void Binder::visit(ForLoop &loop) {
  push_scope();
  loop_indexes.push_back(&loop.get_variable());
  loop.get_variable().accept(*this);
  loop.get_high().accept(*this);

  loops.push_back(&loop);
  loop.get_body().accept(*this);
  loops.pop_back();
  loop_indexes.pop_back();

  pop_scope();
}

void Binder::visit(Break &b) {
  if (variable_declaration) {
    utils::error(b.loc, "breaks are not allowed in variable declarations");
  }
  if (loops.size() >= 1) {
    b.set_loop(loops.back());
  }
  else {
    utils::error(b.loc, "break outside loop");
  }
}

void Binder::visit(Assign &assign) {
  assign.get_lhs().accept(*this);
  if(assign.get_lhs().get_decl()) {
    if (is_loop_index(&assign.get_lhs().get_decl().get()))
        error(assign.get_lhs().loc, "loop index is not assignable");
  }
  assign.get_rhs().accept(*this);
}

} // namespace binder
} // namespace ast
