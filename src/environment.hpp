#pragma once

#include "parser.hpp"

class Environment {
public:
  explicit Environment(Error error)
    : m_variables(), m_error(error)
  {
  }
   
  void declare_var(VarDeclaration declaration) {
    // Check if variable was declared already 
    if (search_var(declaration.identifier.rawValue.value()).has_value()) {
      m_error.error("Variable `" + declaration.identifier.rawValue.value() + 
      "` is already declared.", declaration.identifier);
    }

    m_variables.push_back(declaration);
  }

  void assign_var(VarAssignment assignment) {
    // Check if variable was declared and find its index
    int idx = -1;
    for (int i = 0; i < m_variables.size(); ++i) {
      if (m_variables[i].identifier.rawValue.value() == assignment.identifier.rawValue.value()) {
        idx = i;
        break;
      }
    }

    // Variable was never declared
    if (idx == -1) {
      m_error.error("Variable `" + assignment.identifier.rawValue.value() + 
      "` was never declared.", assignment.identifier);
    }

    // Declaration is a const
    if (m_variables[idx].constant) {
      m_error.error("Cannot reasign constant variable `" + assignment.identifier.rawValue.value() + 
      "`.", assignment.identifier);
    }

    m_variables[idx].expr = assignment.expr;
  }

  [[nodiscard]] optional<VarDeclaration> search_var(string identifier) {
    for (VarDeclaration variable : m_variables) {
      if (variable.identifier.rawValue.value() == identifier)
      return variable;
    }

    return {};
  }

private:
  vector<VarDeclaration> m_variables; 
  Error m_error;
};
