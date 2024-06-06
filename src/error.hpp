#pragma once

#include <utility>

#include "tokenizer.hpp"

using namespace std;

enum class TokenType;
struct Token;

class Error {
public:
  explicit Error(vector<Token> tokens)
    : m_tokens(std::move(tokens))
  {
  }

  void error(const string& message, const Token& token) {
    string line = get_line(token.line);

    cerr << "Error on line: " << token.line << "\n" << line << "\n\n" << message << "\n";
    exit(EXIT_FAILURE);
  }

  static string to_string(const TokenType type) {
    switch (type) {
      case TokenType::Let:
        return "let";
      case TokenType::Const:
        return "const";
      case TokenType::If:
        return "if";
      case TokenType::Else:
        return "else";
      case TokenType::Elif:
        return "elif";
      case TokenType::Exit:
        return "exit";
      case TokenType::Null:
        return "null";
      case TokenType::Int:
        return "integer literal";
      case TokenType::Float:
        return "float literal";
      case TokenType::String: 
        return "string literal";
      case TokenType::True:
        return "true";
      case TokenType::False:
        return "false";
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
      case TokenType::Dot:
        return ".";
      default:
        return "";
    }
  }

private:
  string get_line(const int targetLine) {
    string line;
    line += std::to_string(targetLine) + " | ";

    // Iterate over tokens and append tokens on the target line
    for (Token token : m_tokens) {
      if (token.line == targetLine) {
        // If the token has a raw value, append it to line
        if (token.rawValue.has_value()) {
          line += token.rawValue.value() + " ";
          continue;
        }

        // If the token does not have a raw value, append its type
        line += to_string(token.type) + " ";
        continue;
      }
      
      else if (token.line > targetLine)
      break;
    }

    return line;
  }

  const vector<Token> m_tokens;
};

