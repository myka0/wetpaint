#pragma once

#include "error.hpp"
#include "values/ast.hpp"

class Parser {
public:
  explicit Parser(std::vector<Token> tokens, Error error)
    : m_tokens(std::move(tokens)), m_error(std::move(error)), m_idx(0)
  {
  }

  Program create_ast() {
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
      case TokenType::Fn:
        return parse_fn_declaration();
      default: 
        return parse_assignment_expr();
    }
  }

  Stmt parse_fn_declaration() {
    pop();
    Identifier name = { expect(TokenType::Identifier, 
      "Expected identifier following fucntion declaration keyword.") };

    // Parse function params
    std::vector<Identifier> params;
    std::vector<Stmt> args = parse_args(); 
    for (Stmt arg : args) {
      // Ensure argument is an identifier
      auto expr = arg.get_if<Expr>();
      auto ident = expr->get_if<Identifier>();
      if (!expr || !ident) {
        m_error.report_error("Function parmaters must be of type `Identifier`.", peek(-1).value());
      }
      
      params.push_back(*ident);
    }

    expect(TokenType::OpenBrace, "Expected fucntion body following function declaration.");
    std::vector<Stmt> body;

    // Parse function body
    while (not_eof() && peek().value().type != TokenType::CloseBrace) {
      body.push_back(parse_stmt());
    }
      
    expect(TokenType::CloseBrace, "Closing brace expect to end function declaration.");
    return FunctionDeclaration { name, params, body };
  }

  // Handle variable declaration
  Stmt parse_declaration_stmt() {
    VarDeclaration variable;
    
    // Check for const keyword
    if (pop().type == TokenType::Const) 
      variable.constant = true;

    variable.identifier = { expect(TokenType::Identifier, 
      "Expected identifier following variable declaration keyword.") };

    // Variable in not assigned
    if (peek().value().type == TokenType::Semicol) {
      pop();

      if (variable.constant == true) {
        m_error.report_error("Must assign value to constant variable.", peek(-1).value());
      }

      return variable;
    }

    // Check for variable assignment
    expect(TokenType::Equals, "Expected equals `=` following identifier in variable declaration.");

    variable.expr = parse_object_expr();
    return variable;
  }

  // Handle variable reassignment
  Stmt parse_assignment_expr() {
    Expr lhs = parse_object_expr();

    // Check if lhs is an identifier and next token is an equals
    auto ident = lhs.get_if<Identifier>();
    if (ident && peek().value().type == TokenType::Equals) {
      pop();
      VarAssignment assignment{ *ident, parse_object_expr() };
      return assignment;
    }

    return lhs;
  }

  Expr parse_object_expr() {
    if (peek().value().type != TokenType::OpenBrace) {
      return parse_additive_expr();
    }

    pop();
    ObjectLiteral object;

    // Fill new object with all keys and values 
    while (not_eof() && peek(-1).value().type != TokenType::CloseBrace) {
      // Check for key and get key if it exists
      Identifier key{ expect(TokenType::Identifier, "Object key expected.") };

      // Check for shorthand key declaration
      if (peek().value().type == TokenType::CloseBrace) {
        object.properties.push_back(Property{ key });
        continue;
      } 
      else if (peek().value().type == TokenType::Comma) {
        pop();
        object.properties.push_back(Property{ key });
        continue;
      }

      // Parse key value
      expect(TokenType::Equals, "Expected equals `=` following identifier in variable declaration.");
      object.properties.push_back(Property{ key, parse_object_expr() });

      // Check for next key or object declaration close
      if (peek().value().type == TokenType::Comma) {
        pop();
      } else {
        expect(TokenType::CloseBrace, "Expected closing brace or comma following property.");
      }
    }

    return object;
  }

  // Handle Addition & Subtraction operations
  Expr parse_additive_expr() {
    Expr expr = parse_multiplicitave_expr();
    
    while (peek().value().type == TokenType::Plus || peek().value().type == TokenType::Minus) { 
      Token operand = pop();
      BinaryExpr bin_expr{ expr, parse_multiplicitave_expr(), operand };
      expr = bin_expr;
    }

    return expr; 
  }

  // Handle Multiplication, Division & Modulo operations
  Expr parse_multiplicitave_expr() {
    Expr expr = parse_call_member_expr();
    
    while (peek().value().type == TokenType::Star
        || peek().value().type == TokenType::FwdSlash
        || peek().value().type == TokenType::Modulo) {
      Token operand = pop();
      BinaryExpr bin_expr{ expr, parse_multiplicitave_expr(), operand };
      expr = bin_expr;
    }

    return expr; 
  }

  // Parses a call member expression
  Expr parse_call_member_expr() {
    Expr member = parse_member_expr();

    // If the next token is an open parenthesis, it's a function call
    if (peek().value().type == TokenType::OpenPar) {
      // Parse the call expression with the member as the caller
      return parse_call_expr(member);
    }

    return member;
  }

  // Parses a call expression
  CallExpr parse_call_expr(Expr caller) {
    CallExpr call_expr{ parse_args(), caller }; 

    // If the next token is still an open parenthesis, it's a nested call
    if (peek().value().type == TokenType::OpenPar) {
      // Parse the nested call expression with the current callExpr as the caller
      call_expr = parse_call_expr(call_expr);
    }

    return call_expr;
  }

  // Parses arguments for a function call
  std::vector<Stmt> parse_args() {
    pop();
    // If the next token is a closing parenthesis, there are no arguments
    std::vector<Stmt> args = peek().value().type == TokenType::ClosePar 
      ? std::vector<Stmt>() 
      : parse_args_list();

    // Expect a closing parenthesis to end the argument list
    expect(TokenType::ClosePar, "Expected closing parenthesis.");
    return args;
  }

  // Parses a list of arguments
  std::vector<Stmt> parse_args_list() {
    std::vector<Stmt> args;
    args.push_back(parse_assignment_expr());

    // Parse arguments separated by commas
    while (peek().value().type == TokenType::Comma) {
      pop();
      args.push_back(parse_assignment_expr());
    }

    return args;
  }

  // Recursively parses a member expression
  Expr parse_member_expr() {
    Expr object = parse_primary_expr();

    // Handle dot operator for member access
    if (peek().value().type == TokenType::Dot) {
      Token token = pop();
      Expr member = parse_member_expr();
      
      // Ensure the property is an identifier
      auto ident = object.get_if<Identifier>();
      if (!ident) {
        m_error.report_error("Unexpected token: `dot`.\nDot operator must be used on an identifier.", token);
      }

      // Update the object to be a new member expression with the paresed details
      object = MemberExpr{ *ident, member };
    }

    return object;
  }
  // Parse literal values & grouping expr
  Expr parse_primary_expr() {
    Token token = pop();

    switch (token.type) {
      // User defined values
      case TokenType::Identifier: {
        return Identifier{ token };
      } 
      // Constants and Numeric Constants
      case TokenType::Int: {
        return IntLiteral{ token };
      }
      case TokenType::Float: {
        return FloatLiteral{ token };
      }
      // String Value
      case TokenType::String: {
        return StringLiteral{ token };
      }
      // Boolean Value
      case TokenType::True: {
        token.raw_value = "1";
        return BoolLiteral{ token };
      }
      case TokenType::False: {
        token.raw_value = "0";
        return BoolLiteral{ token };
      }
      // Null Expression
      case TokenType::Null: {
        pop();
        return NullLiteral();
      } 
      // Grouping Expressions
      case TokenType::OpenPar: {
        Expr expr(parse_additive_expr()); 
        pop();
        return expr;
      }
      case TokenType::Return: {
        return ReturnExpr { parse_object_expr() };
      }
      // Unidentified Tokens and Invalid Code Reached
      default: {
        m_error.report_error("Unexpected token found during parsing: `" + 
            m_error.to_string(token.type) + "`", peek(-1).value());
      }
    }
  }

  [[nodiscard]] std::optional<Token> peek(int ahead = 0) const { 
    if (m_idx + ahead >= m_tokens.size()) {
      return {};
    }

    return m_tokens.at(m_idx + ahead);
  }

  Token pop() {
    return m_tokens.at(m_idx++);
  }

  Token expect(TokenType expected_type, std::string message) {
    Token token = pop();

    if (token.type != expected_type) {
      m_error.report_error("Unexpected token: `" + m_error.to_string(peek(-1).value().type) +
          "` \n" + message, peek(-1).value());
    }

    return token;
  }

  bool not_eof() {
    return m_tokens.at(m_idx).type != TokenType::EndOfFile;
  }

private:
  const std::vector<Token> m_tokens;
  Error m_error;
  size_t m_idx;
};
