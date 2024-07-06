#pragma once

#include "error.hpp"
#include "values/ast.hpp"

class Environment {
public:
  explicit Environment(Error error)
    : m_variables(), m_error(error)
  {
    define_print_function();
  }
   
  void declare_var(VarDeclaration declaration) {
    // Check if variable was declared already 
    if (has_var(declaration.identifier).has_value()) {
      m_error.report_error("Variable `" + declaration.identifier.token.raw_value.value() +
          "` is already declared.", declaration.identifier.token);
    }

    m_variables.emplace_back(declaration);
  }

  void assign_var(const VarAssignment& assignment) {
    // Locate the variable
    auto it = std::find_if(m_variables.begin(), m_variables.end(),
      [&assignment](const VarDeclaration& var) {
        return var.identifier.token.raw_value == assignment.identifier.token.raw_value;
      });

    // If variable was not found, report an error
    if (it == m_variables.end()) {
      m_error.report_error("Variable `" + assignment.identifier.token.raw_value.value() +
          "` was never declared.", assignment.identifier.token);
    }

    // If the variable is a constant, report an error
    if (it->constant) {
      m_error.report_error("Cannot reassign constant variable `" + 
          assignment.identifier.token.raw_value.value() + "`.", assignment.identifier.token);
    }

    it->expr = assignment.expr;
  }

  std::optional<VarDeclaration> has_var(Identifier identifier) {
    // Locate the variable
    auto it = std::find_if(m_variables.begin(), m_variables.end(),
      [&identifier](const VarDeclaration& variable) {
        return variable.identifier.token.raw_value == identifier.token.raw_value;
      });

    // If variable was found, return it
    if (it != m_variables.end()) {
      return *it;
    }

    return {};
  }

  VarDeclaration search_var(Identifier identifier) {
    if (auto declaration = has_var(identifier); declaration.has_value()) {
      return declaration.value();
    } else {
      m_error.report_error("Variable `" + identifier.token.raw_value.value() + "` was never declared in scope.", 
          identifier.token);
    }
  }

  constexpr size_t size() {
    return m_variables.size();
  }

  void delete_scope(const size_t idx) {
    m_variables.erase(m_variables.begin() + idx, m_variables.end());
  }

private:
  void declare_native_function(std::string name, NativeFunction::Call function) {
    NativeFunction native_fn{ function };
    VarDeclaration declaration;
    declaration.identifier.token.raw_value = name;
    declaration.expr = Expr{ native_fn };
    declare_var(declaration);
  }

  void define_print_function() {
    declare_native_function("print", [](const std::vector<RuntimeVal>& args) -> RuntimeVal {
      for (const RuntimeVal arg : args) {
        if (arg.is<NullLiteral>()) {
          continue;
        }

        std::cout << arg.get_token().raw_value.value();
      }

      std::cout << std::endl;
      return NullLiteral();
    });
  }

private:
  std::vector<VarDeclaration> m_variables; 
  Error m_error;
};
