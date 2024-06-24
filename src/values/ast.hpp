#pragma once

#include "tokens.hpp"

#include <variant>
#include <vector>
#include <functional>

// Forward declarations of expression types
struct BinaryExpr; 
struct ObjectLiteral;
struct CallExpr;
struct MemberExpr;
struct NativeFunction;

// Literal Types
struct Identifier {
  Token token;
};

struct IntLiteral {
  Token token;
};

struct FloatLiteral {
  Token token;
};

struct StringLiteral {
  Token token;
};

struct BoolLiteral {
  Token token;
};

struct NullLiteral {};

struct Expr {
  std::variant<Identifier, IntLiteral, FloatLiteral, StringLiteral, BoolLiteral, NullLiteral, 
    BinaryExpr*, ObjectLiteral*, CallExpr*, MemberExpr*, NativeFunction*> var;
};

struct VarDeclaration {
  Identifier identifier;
  std::optional<Expr> expr;
  bool constant = false;
};

struct VarAssignment {
  Identifier identifier;
  Expr expr;
};

struct Stmt {
  std::variant<Expr, VarDeclaration, VarAssignment> expr;
};

struct Program {
  std::vector<Stmt> stmts;
};

struct Property {
  Identifier key;
  std::optional<Expr> value;
};

struct ObjectLiteral {
  std::vector<Property> properties;
};

// Expression types
struct BinaryExpr {
  Expr lhs;
  Expr rhs;
  Token operand;
};

struct CallExpr {
  std::vector<Stmt> args;
  Expr caller;
};

struct MemberExpr {
  Identifier object;
  Expr member;
};

struct RuntimeVal {
  std::variant<NullLiteral, IntLiteral, FloatLiteral, StringLiteral, BoolLiteral> value;
};

struct NativeFunction {
  using Call = std::function<RuntimeVal(const std::vector<RuntimeVal>)>;
  Call call;
};
