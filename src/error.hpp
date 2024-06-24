#pragma once

#include "values/tokens.hpp"

#include <vector>
#include <iostream>
#include <unordered_map>

enum class TokenType;
struct Token;

class Error {
public:
  explicit Error(std::vector<Token> tokens)
    : m_tokens(std::move(tokens))
  {
  }

  [[noreturn]] void report_error(const std::string& message, const Token& token) {
    std::string line = extract_line(token.line);
    std::cerr << "Error on line: " << token.line << "\n" << line << "\n\n" << message << "\n";
    exit(EXIT_FAILURE);
  }

  static std::string to_string(TokenType type) {
    static const std::unordered_map<TokenType, std::string> token_type_map = {
      {TokenType::Let, "let"},
      {TokenType::Const, "const"},
      {TokenType::If, "if"},
      {TokenType::Else, "else"},
      {TokenType::Elif, "elif"},
      {TokenType::Exit, "exit"},
      {TokenType::Null, "null"},
      {TokenType::Int, "integer literal"},
      {TokenType::Float, "float literal"},
      {TokenType::String, "string literal"},
      {TokenType::True, "true"},
      {TokenType::False, "false"},
      {TokenType::Identifier, "identifier"},
      {TokenType::Plus, "+"},
      {TokenType::Minus, "-"},
      {TokenType::Star, "*"},
      {TokenType::FwdSlash, "/"},
      {TokenType::Modulo, "%"},
      {TokenType::Equals, "="},
      {TokenType::OpenPar, "("},
      {TokenType::ClosePar, ")"},
      {TokenType::OpenBrace, "{"},
      {TokenType::CloseBrace, "}"},
      {TokenType::Semicol, ";"},
      {TokenType::Dot, "."}
    };

    auto it = token_type_map.find(type);
    if (it != token_type_map.end()) {
      return it->second;
    } else {
      return "";
    }
  }

private:
  std::string extract_line(const int targetLine) {
    std::string line = std::to_string(targetLine) + " | ";

    // Iterate over tokens and append tokens on the target line
    for (const Token token : m_tokens) {
      if (token.line == targetLine) {
        // If the token has a raw value, append it to line
        if (token.rawValue.has_value()) {
          line += token.rawValue.value() + " ";
          continue;
        }

        // If the token does not have a raw value, append its type
        line += to_string(token.type) + " ";
      }
      
      else if (token.line > targetLine)
      break;
    }

    return line;
  }

private:
  const std::vector<Token> m_tokens;
};

