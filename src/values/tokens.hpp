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
  OpenBracket,
  CloseBracket,
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

