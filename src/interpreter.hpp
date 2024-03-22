#pragma once

#include "environment.hpp"
#include "parser.hpp"

using namespace std;

struct NullVal { 
};

struct NumVal {
  variant<int, double> value;
};

struct StringVal {
  string value;
};

struct BoolVal {
  bool value;
};

struct RuntimeVal {
  variant<NullVal, NumVal, StringVal, BoolVal> value;
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
	NumVal num;
        num.value = stoi(get<IntLiteral>(expr.var).token.rawValue.value());
	return { num };
      }
      // Float Literal
      case 2: {
	NumVal num;
	num.value = stod(get<FloatLiteral>(expr.var).token.rawValue.value());
	return { num };
      }
      // String Literal
      case 3: {
	StringVal str;
	str.value = get<StringLiteral>(expr.var).token.rawValue.value();
	return { str };
      }
      // Bool Literal
      case 4: {
	bool value = (get<BoolLiteral>(expr.var).token.rawValue.value() == "1") ? true : false;
	BoolVal bl;
	bl.value = value;
	return { bl };
      }
      // Null Literal
      case 5: {
        NullVal nullVal;
        return { nullVal };
      }
      // Binary Expression
      case 6: {
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
    
    // Evaluate NullVal
    if (lhs.value.index() == 0) {
      NumVal null;
      null.value = 0;
      lhs.value = null;
    }
    if (rhs.value.index() == 0) {
      NumVal null;
      null.value = 0;
      rhs.value = null;
    }

    // Numeric Binary Expr
    if (lhs.value.index() == 1 && rhs.value.index() == 1) { 
      NumVal lhsNum = get<NumVal>(lhs.value);
      NumVal rhsNum = get<NumVal>(rhs.value);
      return { eval_numeric_bin_expr(lhsNum, rhsNum, bin_expr->operand) };
    }

    // Concatonate Strings
    if (lhs.value.index() == 2 && rhs.value.index() == 2 && bin_expr->operand.type == TokenType::Plus) {
      StringVal lhsStr = get<StringVal>(lhs.value);
      StringVal rhsStr = get<StringVal>(rhs.value);
      StringVal concat;
      concat.value = lhsStr.value + rhsStr.value;
      return { concat };
    }

    cerr << "Binary expression, `" << get_runtimeVal(lhs.value.index()) << " " << 
      to_string(bin_expr->operand.type) << " " << get_runtimeVal(rhs.value.index()) << "` is invalid.";
    exit(EXIT_FAILURE);
  }

  string get_runtimeVal(int idx) {
    switch (idx) {
      case 0:
	return "Null";
      case 1:
	return "Number";
      case 2:
	return "String";
      case 3:
	return "Boolean";
      default: 
	return "Invalid";
    }
  }

  NumVal eval_numeric_bin_expr(NumVal lhsNum, NumVal rhsNum, Token t_operand) {
    TokenType operand = t_operand.type;
    int result = -1;

    auto lhs = 0.0, rhs = 0.0;
 
    if (lhsNum.value.index() == 0) 
      lhs = get<int>(lhsNum.value);
    else
      lhs = get<double>(lhsNum.value);

    if (rhsNum.value.index() == 0)
      rhs = get<int>(rhsNum.value);
    else
      rhs = get<double>(rhsNum.value);
    
    switch (operand) {
      case TokenType::Plus: 
	return { lhs + rhs };
      case TokenType::Minus:
	return { lhs - rhs };
      case TokenType::Star:
	return { lhs * rhs };
      case TokenType::FwdSlash:
        if (rhs != 0) // Check for division by zero
          return { lhs / rhs };
        else {
	  std::cerr << "Error: Division by zero" << std::endl;
          exit(EXIT_FAILURE);
        }
      case TokenType::Modulo:
        if (rhs != 0) // Check for modulo by zero
          return { static_cast<int>(lhs) % static_cast<int>(rhs) }; // Casting to int for modulo operation
        else {
          std::cerr << "Error: Modulo by zero" << std::endl;
	  exit(EXIT_FAILURE);
        }
      default: 
	std::cerr << "Error: Invalid operand" << std::endl;
        return { result };
    }
  }

  const Program m_program;
  vector<VarDeclaration> vars;
  Environment env;
};

