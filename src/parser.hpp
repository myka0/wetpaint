#pragma once

#include "tokenizer.hpp"
#include <variant>

#include "error.hpp"

using namespace std; 

struct BinaryExpr; 

struct ObjectLiteral;

struct CallExpr;

struct MemberExpr;

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

struct NullLiteral {
};

struct Expr {
  variant<Identifier, IntLiteral, FloatLiteral, StringLiteral, BoolLiteral, NullLiteral, 
    BinaryExpr*, ObjectLiteral*, CallExpr*, MemberExpr*> var;
};

struct VarDeclaration {
  Token identifier;
  optional<Expr> expr;
  bool constant = false;
};

struct VarAssignment {
  Token identifier;
  Expr expr;
};

struct Stmt {
  variant<Expr, VarDeclaration, VarAssignment> expr;
};

struct Property {
  string key;
  optional<Expr> value;
};

struct ObjectLiteral {
  vector<Property> properties;
};

struct BinaryExpr {
  Expr lhs;
  Expr rhs;
  Token operand;
};

struct CallExpr {
  vector<Stmt> args;
  Expr caller;
};

struct MemberExpr {
  Expr object;
  Expr property;
  bool computed;
};

struct Program {
  vector<Stmt> stmts;
};

class Parser {
public:
  explicit Parser(vector<Token> tokens, Error error)
    : m_tokens(tokens), m_error(error)
  {
  }

  Program createAST() {
    Program program;
    
    while (not_eof()) {
      program.stmts.push_back(parse_stmt());
    }

    return program; 
  }

private:
  // Handle complex statement types
  Stmt parse_stmt() {
    Stmt stmt;
    
    switch (peek().value().type) {
      case TokenType::Let:
      case TokenType::Const:
        return parse_declaration_stmt();
      default: 
        return { parse_expr() };
    }
  }

  // Handle variable declaration
  Stmt parse_declaration_stmt() {
    VarDeclaration variable;
    
    // Check for const keyword
    if (peek().value().type == TokenType::Const) 
      variable.constant = true;

    pop();
    variable.identifier = pop();

    // Variable in not assigned
    if (peek().value().type == TokenType::Semicol) {
      pop();

      if (variable.constant == true) {
        m_error.error("Must assign value to constant variable.", peek(-1).value());
      }

      return { variable };
    }

    // Check for variable assignment
    if (peek().value().type != TokenType::Equals) {
      m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) + 
        "` \nExpected equals `=` following identifier in variable declaration.\n", peek(-1).value());
    }

    pop();
    variable.expr = parse_object_expr();
    
    // Check for statement close
    if (peek().value().type != TokenType::Semicol) {
      m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) + 
        "` \nExpected semicolon `;` following statement declaration.\n", peek(-1).value());
    }

    pop();
    return { variable };
  }

  // Handle expressions  
  Stmt parse_expr() {
    return parse_assignment_expr();
  }

  // Handle variable reassignment
  Stmt parse_assignment_expr() {
    Expr lhs = parse_object_expr();

    // Check if lhs is an identifier and next token is an equals
    if (lhs.var.index() == 0 && peek().value().type == TokenType::Equals) {
      pop();
      VarAssignment assignment;
      assignment.identifier = get<Identifier>(lhs.var).token; 
      assignment.expr = parse_object_expr();

      return { assignment };
    }

    return { lhs };
  }

  Expr parse_object_expr() {
    if (peek().value().type != TokenType::OpenBrace) {
      return parse_additive_expr();
    }

    pop();
    ObjectLiteral* object = new ObjectLiteral;

    // Fill new object with all keys and values 
    while (peek().has_value() && peek().value().type != TokenType::CloseBrace) {
      // Check for key and get key if it exists
      if (peek().value().type != TokenType::Identifier) {
        m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) +
        "` \nObject key expected.", peek().value());
      }

      string key = pop().rawValue.value();

      // Check for shorthand key declaration
      if (peek().value().type == TokenType::CloseBrace) {
        object->properties.push_back({ key });
        continue;
      } 
      else if (peek().value().type == TokenType::Comma) {
        pop();
        object->properties.push_back({ key });
        continue;
      }

      // Check for key value
      if (peek().value().type != TokenType::Equals) {
        m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) +
        "` \nExpected equals `=` following identifier in variable declaration.\n", peek(-1).value());
      }

      pop();
      object->properties.push_back({ key, parse_object_expr() });

      // Check for next key or object declaration close
      if (peek().value().type == TokenType::Comma) {
        pop();
        continue;
      }
      else if (peek().value().type != TokenType::CloseBrace) {
        m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) +
        "` \nExpected closing bracket or comma following property.\n", peek(-1).value());
      }
    }

    pop();
    return { object };
  }

  // Handle Addition & Subtraction operations
  Expr parse_additive_expr() {
    BinaryExpr* head_bin_expr = new BinaryExpr;
    Expr expr = parse_multiplicitave_expr();
    
    while (peek().value().type == TokenType::Plus || peek().value().type == TokenType::Minus) { 
      head_bin_expr = get_bin_expr(expr);
      expr = Expr({ head_bin_expr });
    }

    return expr; 
  }

  // Handle Multiplication, Division & Modulo operations
  Expr parse_multiplicitave_expr() {
    BinaryExpr* head_bin_expr = new BinaryExpr;
    Expr expr = parse_call_member_expr();
    
    while (peek().value().type == TokenType::Star
      || peek().value().type == TokenType::FwdSlash
      || peek().value().type == TokenType::Modulo) {

      head_bin_expr = get_bin_expr(expr);
      expr = Expr({ head_bin_expr });
    }

    return expr; 
  }

  // Populate binary expr
  BinaryExpr* get_bin_expr(Expr expr) {
    BinaryExpr* bin_expr = new BinaryExpr;

    bin_expr->lhs = expr;
    bin_expr->operand = pop();
    bin_expr->rhs = parse_multiplicitave_expr();
    
    return bin_expr;
  }
 
  // Parses a call member expression
  Expr parse_call_member_expr() {
    Expr member = parse_member_expr();

    // If the next token is an open parenthesis, it's a function call
    if (peek().value().type == TokenType::OpenPar) {
      // Parse the call expression with the member as the caller
      return { parse_call_expr(member) };
    }

    return member;
  }

  // Parses a call expression
  CallExpr* parse_call_expr(Expr caller) {
    CallExpr* callExpr = new CallExpr({ parse_args(), caller }); 

    // If the next token is still an open parenthesis, it's a nested call
    if (peek().value().type == TokenType::OpenPar) {
      // Parse the nested call expression with the current callExpr as the caller
      callExpr = parse_call_expr({ callExpr });
    }

    return callExpr;
  }

  // Parses arguments for a function call
  vector<Stmt> parse_args() {
    pop();
    // If the next token is a closing parenthesis, there are no arguments
    vector<Stmt> args = peek().value().type == TokenType::ClosePar ? vector<Stmt>() : parse_args_list();

    // Expect a closing parenthesis to end the argument list
    if (peek().value().type != TokenType::ClosePar) {
      m_error.error("Expected closing parenthesis.", peek(-1).value());
    }
    pop();

    return args;
  }

  // Parses a list of arguments
  vector<Stmt> parse_args_list() {
    vector<Stmt> args;
    args.push_back(parse_assignment_expr());

    // Parse arguments separated by commas
    while (peek().value().type == TokenType::Comma) {
      pop();
      args.push_back(parse_assignment_expr());
    }

    return args;
  }

  // Parses a member expression
  Expr parse_member_expr() {
    Expr object = parse_primary_expr();

    while (peek().value().type == TokenType::Dot || peek().value().type == TokenType::OpenBracket) {
      Token token = pop();
      Expr property;
      bool computed;

      // Handle dot operator for member access
      if (token.type == TokenType::Dot) {
        computed = false;
        property = parse_primary_expr();

        // Ensure the property is an identifier
        if (property.var.index() != 0) {
          m_error.error("Unexpected token: `dot`.\nDot operator must be used on an identifier.", token);
        }
      }
      // Handle bracket operator for computed member access
      else {
        computed = true;
        property = parse_object_expr();

        // Ensure the next token is a closing bracket
        if (peek().value().type != TokenType::CloseBracket) {
          m_error.error("Unexpected token: `" + m_error.to_string(peek().value().type) +
          "` \nExpected closing bracket.\n", peek(-1).value());
        }
      }

      // Create a MemberExpr with the parsed details and update the object to be the new member expression
      MemberExpr* member = new MemberExpr({ object, property, computed });
      object.var = member;
    }

    return object;
  }

  // Parse literal values & grouping expr
  Expr parse_primary_expr() {
    Token token = pop();

    switch (token.type) {
      // User defined values
      case TokenType::Identifier: {
        return { Identifier({ token }) };
      } 
      // Constants and Numeric Constants
      case TokenType::Int: {
        return { IntLiteral({ token }) };
      }
      case TokenType::Float: {
        return { FloatLiteral({ token }) };
      }
      // String Value
      case TokenType::String: {
        return { StringLiteral({ token }) };
      }
      // Boolean Value
      case TokenType::True: {
        token.rawValue = "1";
        return { BoolLiteral({ token }) };
      }
      case TokenType::False: {
        token.rawValue = "0";
        return { BoolLiteral({ token }) };
      }
      // Null Expression
      case TokenType::Null: {
        pop();
        return { NullLiteral() };
      } 
      // Grouping Expressions
      case TokenType::OpenPar: {
        Expr expr = Expr(parse_additive_expr()); 
        pop();
        return expr;
      }
      // Unidentified Tokens and Invalid Code Reached
      default: {
        string rawValue = "";
        if (token.rawValue.has_value())
          rawValue = token.rawValue.value();

        m_error.error("Unexpected token found during parsing: `" + m_error.to_string(token.type) +
          "`\n" + rawValue, peek(-1).value());
        exit(EXIT_FAILURE);
      }
    }
  }

  [[nodiscard]] optional<Token> peek(int ahead = 0) const { 
    if (m_idx + ahead >= m_tokens.size()) {
      return {};
    }

    return m_tokens.at(m_idx + ahead);
  }

  Token pop() {
    return m_tokens.at(m_idx++);
  }

  bool not_eof() {
    return m_tokens.at(m_idx).type != TokenType::EndOfFile;
  }

  const vector<Token> m_tokens;
  Error m_error;
  size_t m_idx = 0;
};
