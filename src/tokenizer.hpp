#pragma once

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>
#include <utility>
#include <vector>

using namespace std;

enum class TokenType { 
  // Literal Types
  Int,
  Float,
  Identifier,
  Null,
  True,
  False,
  String,

  //Keywords
  Let,
  Const,
  If,
  Else,
  Elif,
  Exit,

  // Operators + Grouping
  Plus,
  Minus,
  Star,
  FwdSlash,
  Modulo,
  Equals, 
  OpenPar,
  ClosePar,
  OpenBrace,
  CloseBrace,
  Semicol,
  Hashtag,
  EndOfFile
};

struct Token {
  TokenType type;
  int line;
  optional<string> rawValue;
};

class Tokenizer {
public:
  explicit Tokenizer(string src)
    : m_src(std::move(src))
  {
  }

  vector<Token> tokenize() {
    vector<Token> tokens;
    string buffer;
    int line_count = 1;

    while(peek().has_value()) {
      // Get keyword
      if (isalpha(peek().value())) {
        while (isalnum(peek().value())) {
          buffer.push_back(pop());
        }

        TokenType token = get_keyword(buffer);

        if (token == TokenType::Identifier)
          tokens.push_back({ token, line_count, buffer });
        else
          tokens.push_back({ token, line_count });

        buffer.clear();
      }
      
      // Get number literal
      else if (isdigit(peek().value())) {
        while (isdigit(peek().value())) 
          buffer.push_back(pop());

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
  [[nodiscard]] optional<char> peek(int ahead = 0) const {
    if (m_idx + ahead >= m_src.length()) {
      return {};
    }

    return m_src.at(m_idx);
  }

  char pop() {
    return m_src.at(m_idx++);
  }

  static TokenType get_keyword(const string& token) {
    if (token == "let") 
      return TokenType::Let;
    else if (token == "const")
      return TokenType::Const;
    else if (token == "if") 
      return TokenType::If;
    else if (token == "else") 
      return TokenType::Else;
    else if (token == "elif")
      return TokenType::Elif;
    else if (token == "exit") 
      return TokenType::Exit;
    else if (token == "null")
      return TokenType::Null;
    else if (token == "true")
      return TokenType::True;
    else if (token == "false")
      return TokenType::False;

    return TokenType::Identifier;
  }

  static TokenType get_symbol(const char token) {
    if (token == '+') 
      return TokenType::Plus;
    else if (token == '-')
      return TokenType::Minus;
    else if (token == '*')
      return TokenType::Star;
    else if (token == '/')
      return TokenType::FwdSlash;
    else if (token == '%')
      return TokenType::Modulo;
    else if (token == '=')
      return TokenType::Equals;
    else if (token == '(')
      return TokenType::OpenPar;
    else if (token == ')')
      return TokenType::ClosePar;
    else if (token == '{')
      return TokenType::OpenBrace;
    else if (token == '}')
      return TokenType::CloseBrace;
    else if (token == ';')
      return TokenType::Semicol;
    else {
      cerr << "Invalid character: " << token << endl; 
      exit(EXIT_FAILURE);
    }
  }

  const string m_src;
  size_t m_idx = 0;
};
