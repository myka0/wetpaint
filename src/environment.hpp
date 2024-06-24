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
    if (search_var(declaration.identifier).has_value()) {
      m_error.report_error("Variable `" + declaration.identifier.token.rawValue.value() + 
        "` is already declared.", declaration.identifier.token);
    }

    m_variables.push_back(declaration);
  }

  void assign_var(const VarAssignment& assignment) {
    // Locate the variable
    auto it = std::find_if(m_variables.begin(), m_variables.end(),
      [&assignment](const VarDeclaration& var) {
        return var.identifier.token.rawValue == assignment.identifier.token.rawValue;
      });

    // If variable was not found, report an error
    if (it == m_variables.end()) {
      m_error.report_error("Variable `" + assignment.identifier.token.rawValue.value() +
        "` was never declared.", assignment.identifier.token);
    }

    // If the variable is a constant, report an error
    if (it->constant) {
      m_error.report_error("Cannot reassign constant variable `" + 
        assignment.identifier.token.rawValue.value() + "`.", assignment.identifier.token);
    }

    it->expr = assignment.expr;
  }

  std::optional<VarDeclaration> search_var(Identifier identifier) {
    // Locate the variable
    auto it = std::find_if(m_variables.begin(), m_variables.end(),
      [&identifier](const VarDeclaration& variable) {
        return variable.identifier.token.rawValue == identifier.token.rawValue;
      });

    // If variable was found, return it
    if (it != m_variables.end()) {
      return *it;
    }

    return {};
  }

  VarDeclaration has_var(Identifier identifier) {
    if (auto declaration = search_var(identifier); declaration.has_value()) {
      return declaration.value();
    } else {
      m_error.report_error("Variable `" + identifier.token.rawValue.value() + "` was never declared.", 
        identifier.token);
    }
  }

private:
  void declare_native_function(std::string name, NativeFunction::Call function) {
    NativeFunction* nativeFn = new NativeFunction({ function });
    VarDeclaration declaration;
    declaration.identifier.token.rawValue = name;
    declaration.expr = Expr{nativeFn};
    declare_var(declaration);
  }

  void define_print_function() {
    declare_native_function("print", [](const std::vector<RuntimeVal>& args) -> RuntimeVal {
      for (const RuntimeVal arg : args) {
        // Deduce the type of arg and print it
        std::visit([](const auto& value) {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, IntLiteral> || std::is_same_v<T, FloatLiteral> || 
                        std::is_same_v<T, StringLiteral> || std::is_same_v<T, BoolLiteral>) {
            std::cout << value.token.rawValue.value();
          }
        }, arg.value);
      }

      std::cout << std::endl;
      return { NullLiteral() };
    });
  }

private:
  std::vector<VarDeclaration> m_variables; 
  Error m_error;
};
