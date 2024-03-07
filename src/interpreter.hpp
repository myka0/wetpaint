#pragma once

#include "environment.hpp"
#include "parser.hpp"

using namespace std;

struct NullVal { 
};

struct IntVal {
  int value;
};

struct RuntimeVal {
  variant<NullVal, IntVal> value;
};

class Interpreter {
public:
  explicit Interpreter(Program program)
    : m_program(program) 
  {
  }

  RuntimeVal evaluate_program() {
    RuntimeVal lastEval;
    NullVal nullVal;
    lastEval.value = nullVal;

    for (Stmt stmt : m_program.stmts) {
      lastEval = evaluate(stmt);
    }

    return lastEval;
  }

private:
  RuntimeVal evaluate(Stmt stmt) {
    switch (stmt.expr.index()) {
      // Expression
      case 0: 
	return eval_expr(get<Expr>(stmt.expr));

      // Variable Declaration
      case 1: { 
	env.declare_var(get<VarDeclaration>(stmt.expr));
	NullVal nullVal;
	return { nullVal };
      }
      // Variable Assignment
      case 2: {
	env.assign_var(get<VarAssignment>(stmt.expr));
	NullVal nullVal;
	return { nullVal };
      }
      default: {
	cerr << "This statement is not being interpreted yet.";
        exit(EXIT_FAILURE);
      }
    }
  }

  RuntimeVal eval_expr(Expr expr) {
    switch (expr.var.index()) {
      // Indetifier
      case 0: {
	string ident = get<Identifier>(expr.var).token.rawValue.value();
	return eval_expr(env.search_var(ident).value().expr.value());
      }
      // Int Literal
      case 1: {
	IntVal integer;
        integer.value = stoi(get<IntLiteral>(expr.var).token.rawValue.value());
	return { integer };
      }
      // Null Literal
      case 2: {
        NullVal nullVal;
        return { nullVal };
      }
      // Binary Expression
      case 3: {
        return eval_bin_expr(get<BinaryExpr*>(expr.var));
      }
      default: {
        cerr << "This Node is not being interpreted yet.";
        exit(EXIT_FAILURE);
      }
    } 
  }

  RuntimeVal eval_bin_expr(BinaryExpr* bin_expr) {
    RuntimeVal lhs = evaluate({ bin_expr->lhs });
    RuntimeVal rhs = evaluate({ bin_expr->rhs });
    
    if (lhs.value.index() == 1 && rhs.value.index() == 1) { 
      IntVal lhsInt = get<IntVal>(lhs.value);
      IntVal rhsInt = get<IntVal>(rhs.value);
      return { eval_numeric_bin_expr(lhsInt, rhsInt, bin_expr->operand) };
    }

    NullVal nullVal;
    return { nullVal };
  }

  IntVal eval_numeric_bin_expr(IntVal lhs, IntVal rhs, Token t_operand) {
    TokenType operand = t_operand.type;
    int result = -1;

    switch (operand) {
      case TokenType::Plus: 
	return { lhs.value + rhs.value };
      case TokenType::Minus:
	return { lhs.value - rhs.value };
      case TokenType::Star:
	return { lhs.value * rhs.value };
      case TokenType::FwdSlash:
	return { lhs.value / rhs.value };
      case TokenType::Modulo:
	return { lhs.value % rhs.value };
      default: 
	return { result };
    }
  }

  const Program m_program;
  vector<VarDeclaration> vars;
  Environment env;
};

