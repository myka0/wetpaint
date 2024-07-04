#pragma once

#include "tokens.hpp"

#include <vector>
#include <functional>
#include <memory>
#include <any>
#include <typeindex>

// Class representing a node in an abstract syntax tree
class ASTNode {
protected:
  std::any var;

public:
  ASTNode() = default;

  template<typename T>
  ASTNode(T value) : var(value) {}

  // Get the stored value as a mutable reference or constant reference
  template<typename T>
  T& get() {
    return std::any_cast<T&>(var);
  }

  template<typename T>
  const T& get() const {
    return std::any_cast<const T&>(var);
  }

  // Get a pointer to the stored value if it matches the requested type
  template<typename T>
  T* get_if() {
    if (is<T>()) {
      return std::any_cast<T>(&var);
    }
    return nullptr;
  }

  template<typename T>
  const T* get_if() const {
    if (is<T>()) {
      return std::any_cast<const T>(&var);
    }
    return nullptr;
  }

  // Get the type information
  const std::type_info& type() const {
    return var.type();
  }

  // Check if the stored value is of the requested type
  template<typename T>
  bool is() const {
    return var.type() == typeid(T);
  }
};

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

// Program structure
struct Expr : public ASTNode {
  using ASTNode::ASTNode;
};

struct Stmt : public ASTNode {
  using ASTNode::ASTNode;
};

struct Program {
  std::vector<Stmt> stmts;
};

// Declarations
struct VarDeclaration {
  Identifier identifier;
  std::optional<Expr> expr;
  bool constant = false;
};

struct VarAssignment {
  Identifier identifier;
  Expr expr;
};

struct FunctionDeclaration {
  Identifier name;
  std::vector<Identifier> params;
  std::vector<Stmt> body;
};

class Environment;
struct Function {
  FunctionDeclaration declaration;
  std::shared_ptr<Environment> env;
};

// Object Literal
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

struct ReturnExpr {
  Expr expr;
};

// Runtime
struct RuntimeVal : public ASTNode {
  using ASTNode::ASTNode;

  // Function to get the token held within the value
  Token get_token() const {
    static const std::unordered_map<std::type_index, std::function<Token(const ASTNode&)>> tokens {
      { typeid(IntLiteral), [](const ASTNode& node) { return node.get<IntLiteral>().token; } },
      { typeid(FloatLiteral), [](const ASTNode& node) { return node.get<FloatLiteral>().token; } },
      { typeid(StringLiteral), [](const ASTNode& node) { return node.get<StringLiteral>().token; } },
      { typeid(BoolLiteral), [](const ASTNode& node) { return node.get<BoolLiteral>().token; } },
      { typeid(Identifier), [](const ASTNode& node) { return node.get<Identifier>().token; } }
    };

   auto it = tokens.find(var.type());
    return it->second(var);
  }
};

struct NativeFunction {
  using Call = std::function<RuntimeVal(const std::vector<RuntimeVal>)>;
  Call call;
};
