#pragma once

#include "parser.hpp"

class Environment {
public:
  explicit Environment()
    : m_variables()
  {
  }
   
  void declare_var(VarDeclaration declaration) {
    if (search_var(declaration.identifier).has_value()) {
      cerr << "Variable `" << declaration.identifier << "` is already declared. \n";
      exit(EXIT_FAILURE);
    }

    m_variables.push_back(declaration);
  }

  void assign_var(VarAssignment assignment) {
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
