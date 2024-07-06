#pragma once

#include <optional>
#include <string>

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
  Fn,
  If,
  Else,
  Elif,
  Return,

  // Grouping
  OpenPar,
  ClosePar,
  OpenBrace,
  CloseBrace,
  OpenBracket,
  CloseBracket,

  // Operators
  Plus,
  Minus,
  Star,
  FwdSlash,
  Modulo,
  Equals, 
  Not,
  Greater,
  GreaterEquals,
  Less,
  LessEquals,
  And,
  Or,
  Comma,
  Colon,
  Semicol,
  Dot,
  EndOfFile
};

struct Token {
  TokenType type;
  int line;
  std::optional<std::string> raw_value;
};

