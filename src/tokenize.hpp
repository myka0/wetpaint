#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>
#include <vector>

using namespace std;


enum class TokenType { 
  Let,
  If,
  Else,
  Elif,
  Exit,
  Int_Lit,
  Identifier,
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
  Hashtag
};

inline TokenType get_word(const string token) {
  if (token == "let") 
    return TokenType::Let;
  else if (token == "if") 
    return TokenType::If;
  else if (token == "else") 
    return TokenType::Else;
  else if (token == "elif")
    return TokenType::Elif;
  else if (token == "exit") 
    return TokenType::Exit;

  return TokenType::Identifier;
}

inline TokenType get_symbol(const char token) {
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
    cerr << "Invalid character" << endl;
    exit(EXIT_FAILURE);
  }

  
}

struct Token {
  TokenType type;
  optional<string> value;
};

class Tokenizer {
public:
  inline explicit Tokenizer(string src)
    : m_src(src)
  {
  }

  inline vector<Token> tokenize() {
    vector<Token> tokens;
    string buffer;
    int line_count = 1;

    while(peek().has_value()) {

      if (isalpha(peek().value())) {
        while (isalnum(peek().value())) {
          buffer.push_back(pop());
        }

        TokenType token = get_word(buffer);
        if (token == TokenType::Identifier)
          tokens.push_back({ token, buffer });
        else
          tokens.push_back({ token });
        buffer.clear();
      }
      
      else if (isdigit(peek().value())) {
        while (isdigit(peek().value())) {
          buffer.push_back(pop());
        }

        tokens.push_back({ TokenType::Int_Lit, buffer });
        buffer.clear();
      }

      // Skip tokenizing sinlge line comment
      else if (peek().value() == '#') {
        while (peek().has_value() && peek().value() != '\n') {
          pop();
        }
      }

      // Skip whitespace
      else if (isspace(peek().value())) {
        pop();
      }

      // Token must be symbol or invalid
      else {
        tokens.push_back({ get_symbol(peek().value()) });
        pop();
      }
    }

    m_idx = 0;
    return tokens;
  }

private:
  [[nodiscard]] inline optional<char> peek(int ahead = 0) const {
    if (m_idx + ahead >= m_src.length()) {
      return {};
    }

    return m_src.at(m_idx);
  }

  inline char pop() {
    return m_src.at(m_idx++);
  }

  const string m_src;
  size_t m_idx = 0;
};
