#pragma once

#include "tokenize.hpp"
#include <variant>

using namespace std; 

inline string to_string(const TokenType type) {
  switch (type) {
    case TokenType::Let:
      return "let";
    case TokenType::Const:
      return "const";
    case TokenType::If:
      return "if";
    case TokenType::Else:
      return "else";
    case TokenType::Elif:
      return "elif";
    case TokenType::Exit:
      return "exit";
    case TokenType::Null:
      return "null";
    case TokenType::Int_Lit:
      return "int";
    case TokenType::Identifier:
      return "identifier";
    case TokenType::Plus:
      return "+";
    case TokenType::Minus:
      return "-";
    case TokenType::Star:
      return "*";
    case TokenType::FwdSlash:
      return "/";
    case TokenType::Modulo:
      return "%";
    case TokenType::Equals:
      return "=";
    case TokenType::OpenPar:
      return "(";
    case TokenType::ClosePar:
      return ")";
    case TokenType::OpenBrace:
      return "{";
    case TokenType::CloseBrace:
      return "}";
    case TokenType::Semicol:
      return ";";
    case TokenType::Hashtag:
      return "#";
    default:
     return "";
  }
}

struct BinaryExpr; 

struct Identifier {
  Token token;
};

struct IntLiteral {
  Token token;
};

struct NullLiteral {
};

struct Expr {
  variant<Identifier, IntLiteral, NullLiteral, BinaryExpr*> var;
};

struct BinaryExpr {
  Expr lhs;
  Expr rhs;
  Token operand;
};

struct VarDeclaration {
  string identifier;
  optional<Expr> expr;
  bool constant = false;
};

struct VarAssignment {
  string identifier;
  Expr expr;
};

struct Stmt {
  variant<Expr, VarDeclaration, VarAssignment> expr;
};

struct Program {
  vector<Stmt> stmts;
};

class Parser {
public:
  explicit Parser(vector<Token> tokens)
    : m_tokens(tokens) 
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
    stmt.expr = parse_expr();
    return stmt;
  }

  // Handle variable declaration
  Stmt parse_declaration_stmt() {
    VarDeclaration variable;
    
    // Check for const keyword
    if (peek().value().type == TokenType::Const) 
      variable.constant = true;
    pop();

    // Check for identifier
    if (peek().value().type != TokenType::Identifier) {
      cerr << "Unexpected token: " << to_string(peek().value().type) << "\n" <<
	"Expected identifier name following keyword `let` \n";
      exit(EXIT_FAILURE);
    }
    variable.identifier = pop().rawValue.value();

    // Variable in not assigned
    if (peek().value().type == TokenType::Semicol) {
      pop();

      if (variable.constant == true) {
	cerr << "Must assign value to constant variable.";
	exit(EXIT_FAILURE);
      }

      return { variable };
    }

    // Check for variable assignment
    if (peek().value().type != TokenType::Equals) {
      cerr << "Unexpected token: " << to_string(peek().value().type) << "\n" <<
	"Expected equals `=` following identifier in variable declaration.";
      exit(EXIT_FAILURE);
    }
    pop();

    variable.expr = parse_expr();
    
    // Check for statement close
    if (peek().value().type != TokenType::Semicol) {
      cerr << "Unexpected token: " << to_string(peek().value().type) << "\n" <<
	"Expected semicolon `;` following statement declaration.";
      exit(EXIT_FAILURE);
    }
    pop();

    return { variable };
  }

  // Handle expressions  
  Expr parse_expr() {
    return parse_additive_expr();
  }

  // Handle Addition & Subtraction operations
  Expr parse_additive_expr() {
    BinaryExpr** head_bin_expr = (BinaryExpr**)malloc(sizeof(BinaryExpr));
    BinaryExpr* bin_expr = (BinaryExpr*)malloc(sizeof(BinaryExpr));
    
    *head_bin_expr = bin_expr;
    Expr expr = parse_multiplicitave_expr();
    
    while (peek().value().type == TokenType::Plus || peek().value().type == TokenType::Minus) { 
      *head_bin_expr = get_bin_expr(expr);
      expr = get_expr(*head_bin_expr);
    }

    return expr; 
  }

  // Handle Multiplication, Division & Modulo operations
  Expr parse_multiplicitave_expr() {
    BinaryExpr** head_bin_expr = (BinaryExpr**)malloc(sizeof(BinaryExpr));
    BinaryExpr* bin_expr = (BinaryExpr*)malloc(sizeof(BinaryExpr));
    
    *head_bin_expr = bin_expr;
    Expr expr = parse_primary_expr();
    
    while (peek().value().type == TokenType::Star || 
	  peek().value().type == TokenType::FwdSlash ||
          peek().value().type == TokenType::Modulo) { 
      *head_bin_expr = get_bin_expr(expr);
      expr = get_expr(*head_bin_expr);
    }

    return expr; 
  }

  // Populate binary expr
  BinaryExpr* get_bin_expr(Expr expr) {
    BinaryExpr* bin_expr = (BinaryExpr*)malloc(sizeof(BinaryExpr));

    bin_expr->lhs = expr;
    bin_expr->operand = pop();
    bin_expr->rhs = parse_multiplicitave_expr();
    
    return bin_expr;
  }
 
  // Create expr with binary expr
  Expr get_expr(BinaryExpr* bin_expr) {
    Expr expr;
    expr.var = bin_expr;
    return expr;
  }

  // Parse literal values & grouping expr
  Expr parse_primary_expr() {
    Token token = pop();
    Expr expr;

    switch (token.type) {
      // User defined values
      case TokenType::Identifier: {
        Identifier ident;
        ident.token = token;
        expr.var = ident;
        return expr;
      }
      // Constants and Numeric Constants
      case TokenType::Int_Lit: {
        IntLiteral intlit;
        intlit.token = token;
        expr.var = intlit;
        return expr;
      }
      // Null Expression
      case TokenType::Null: {
        pop();
        NullLiteral nulllit;
        expr.var = nulllit;
        return expr;
      }
      // Grouping Expressions
      case TokenType::OpenPar: {
        expr = parse_expr();
        pop();
        return expr;
      }
      // Unidentified Tokens and Invalid Code Reached
      default: {
        cerr << "Unexpected token found during parsing: " << to_string(token.type) 
	  << " " << pop().rawValue.value() << "\n";
        exit(EXIT_FAILURE);
      }
    }
  }

  [[nodiscard]] optional<Token> peek(int ahead = 0) const {
    if (m_idx + ahead >= m_tokens.size()) {
      return {};
    }

    return m_tokens.at(m_idx);
  }

  Token pop() {
    return m_tokens.at(m_idx++);
  }

  bool not_eof() {
    return m_tokens.at(m_idx).type != TokenType::EndOfFile;
  }

  const vector<Token> m_tokens;
  size_t m_idx = 0;
};
