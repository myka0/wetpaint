#pragma once

#include "parser.hpp"

class Environment {
public:
  explicit Environment()
    : m_variables()
  {
  }
   
  void declare_var(VarDeclaration declaration) {
    // Check if variable was declared already 
    if (search_var(declaration.identifier).has_value()) {
      cerr << "Variable `" << declaration.identifier << "` is already declared. \n";
      exit(EXIT_FAILURE);
    }

    m_variables.push_back(declaration);
  }

  void assign_var(VarAssignment assignment) {
    // Check if variable was declared and find its index
    int idx = -1;
    for (int i = 0; i < m_variables.size(); ++i) {
      if (m_variables[i].identifier == assignment.identifier) {
	idx = i;
	break;
      }
    }

    // Variable was never declared
    if (idx == -1) {
      cerr << "Variable was never declared: `" << assignment.identifier << "`.\n";
      exit(EXIT_FAILURE);
    }

    // Declaration is a const
    if (m_variables[idx].constant) {
      cerr << "Cannot reasign constant variable: `" << assignment.identifier << "`.\n";
      exit(EXIT_FAILURE);
    }

    m_variables[idx].expr = assignment.expr;
  }

  [[nodiscard]] optional<VarDeclaration> search_var(string identifier) {
    for (VarDeclaration variable : m_variables) {
      if (variable.identifier == identifier)
	return variable;
    }
    return {};
  }

private:
  vector<VarDeclaration> m_variables; 
};
