#pragma once

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <optional>
#include <vector>

using namespace std;


enum class TokenType { Number, Num_Lit, Equals, Semicol, Exit };

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

    while (peek().has_value()) {
      if (isspace(peek().value())) {
        pop();
        continue;
      }

      else if (isalpha(peek().value())) {
        while (isalnum(peek().value())) {
          buffer.push_back(pop());
        }
        buffer.clear();
      }

      while (isdigit(peek().value())) {
        buffer.push_back(pop());
        buffer.clear();
      }

      if (peek().value() == ';') {
        tokens.push_back({.type = TokenType::Semicol});
        pop();
        continue;
      }
    }

    m_idx = 0;
    return tokens;
  }

private:
  [[nodiscard]] inline optional<char> peek(int count = 1) const {
    if (m_idx + count > m_src.length()) {
      return {};
    }

    return m_src.at(m_idx);
  }

  inline char pop() {
    return m_src.at(m_idx++);
  }



  const string m_src;
  int m_idx;
};
