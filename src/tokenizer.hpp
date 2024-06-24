#pragma once

#include "values/tokens.hpp"

#include <vector>
#include <iostream>
#include <unordered_map>

class Tokenizer {
public:
  explicit Tokenizer(std::string src)
    : m_src(std::move(src)), m_idx(0)
  {
  }

  std::vector<Token> tokenize() {
    std::vector<Token> tokens;
    std::string buffer;
    int line_count = 1;

    while(peek().has_value()) {
      // Get keyword
      if (isalpha(peek().value())) {
        while (isalnum(peek().value()) || peek().value() == '_') {
          buffer.push_back(pop());
        }

        TokenType token = get_keyword(buffer);

        if (token == TokenType::Identifier) {
          tokens.push_back({ token, line_count, buffer });
        } else {
          tokens.push_back({ token, line_count });
        }

        buffer.clear();
      }
      
      // Get number literal
      else if (isdigit(peek().value())) {
        while (isdigit(peek().value())) {
          buffer.push_back(pop());
        }

        // Tokenize floating point value if it exists
        if (peek().value() == '.') {
          buffer.push_back(pop());

          while (isdigit(peek().value()))
            buffer.push_back(pop());

          tokens.push_back({ TokenType::Float, line_count, buffer });
        }

        // Tokenize integer
        else {
          tokens.push_back({ TokenType::Int, line_count, buffer });
        }

        buffer.clear();
      }

      // Get string
      else if (peek().value() == *"\"") {
        pop();
        while (peek().value() != *"\"") {
          buffer.push_back(pop());
        }

        pop();
        tokens.push_back({ TokenType::String, line_count, buffer });
        buffer.clear();
      }

      // Skip tokenizing comment
      else if (peek().value() == '#') {
        while (peek().has_value() && peek().value() != '\n') {
          pop();
        }
      }

     // Increment line counter for every new line
      else if (peek().value() == '\n') {
        pop();
        line_count++;
      } 

      // Skip whitespace
      else if (isspace(peek().value())) {
        pop();
      }

      // Character must be symbol or invalid
      else {
        tokens.push_back({ get_symbol(peek().value()), line_count });
        pop();
      }
    }

      tokens.push_back({ TokenType::EndOfFile, line_count });
    m_idx = 0;
    return tokens;
  }

private:
  [[nodiscard]] std::optional<char> peek(int ahead = 0) const {
    if (m_idx + ahead >= m_src.length()) {
      return {};
    }

    return m_src[m_idx + ahead];
  }

  char pop() {
    return m_src[m_idx++];
  }

  TokenType get_keyword(const std::string& token) const {
    static const std::unordered_map<std::string, TokenType> keywords = {
      {"let", TokenType::Let},
      {"const", TokenType::Const},
      {"if", TokenType::If},
      {"else", TokenType::Else},
      {"elif", TokenType::Elif},
      {"exit", TokenType::Exit},
      {"null", TokenType::Null},
      {"true", TokenType::True},
      {"false", TokenType::False}
    };

    auto it = keywords.find(token);
    return it != keywords.end() ? it->second : TokenType::Identifier;
  }

  TokenType get_symbol(char token) const {
    static const std::unordered_map<char, TokenType> symbols = {
      {'+', TokenType::Plus},
      {'-', TokenType::Minus},
      {'*', TokenType::Star},
      {'/', TokenType::FwdSlash},
      {'%', TokenType::Modulo},
      {'=', TokenType::Equals},
      {'(', TokenType::OpenPar},
      {')', TokenType::ClosePar},
      {'{', TokenType::OpenBrace},
      {'}', TokenType::CloseBrace},
      {'[', TokenType::OpenBracket},
      {']', TokenType::CloseBracket},
      {',', TokenType::Comma},
      {':', TokenType::Colon},
      {';', TokenType::Semicol},
      {'.', TokenType::Dot}
    };
        
    auto it = symbols.find(token);
    if (it != symbols.end()) {
      return it->second;
    }
    std::cerr << "Invalid character: " << token << "\n"; 
    std::exit(EXIT_FAILURE);
  }

  const std::string m_src;
  size_t m_idx;
};
