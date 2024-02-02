#pragma once

#include "tokenize.hpp"
#include <vector>
#include <string>

using namespace std; 

inline string to_string(const TokenType type) {
  switch (type) {
    case TokenType::Let:
      return "let";
    case TokenType::If:
      return "if";
    case TokenType::Else:
      return "else";
    case TokenType::Elif:
      return "elif";
    case TokenType::Exit:
      return "exit";
    case TokenType::Int_Lit:
      return "int";
    case TokenType::Identifier:
      return "identifier";
    case TokenType::Plus:
      return "+";
    case TokenType::Minus:
      return "-";
    case TokenType::Star:
      return "*";
    case TokenType::FwdSlash:
      return "/";
    case TokenType::Modulo:
      return "%";
    case TokenType::Equals:
      return "=";
    case TokenType::OpenPar:
      return "(";
    case TokenType::ClosePar:
      return ")";
    case TokenType::OpenBrace:
      return "{";
    case TokenType::CloseBrace:
      return "}";
    case TokenType::Semicol:
      return ";";
    case TokenType::Hashtag:
      return "#";
  }
  return "";
}

class Parser {
public:
  inline explicit Parser(vector<Token> tokens)
    : m_tokens(tokens) 
  {
  }

  void print_tokens() {
    while (peek().has_value()) {
      cout << to_string(pop().type) << " ";
    }
  }

private:
  [[nodiscard]] inline optional<Token> peek(int count = 0) const {
    if (m_idx + count >= m_tokens.size()) {
      return {};
    }

    return m_tokens.at(m_idx);
  }

  inline Token pop() {
    return m_tokens.at(m_idx++);
  }

  const vector<Token> m_tokens;
  size_t m_idx = 0;
};
