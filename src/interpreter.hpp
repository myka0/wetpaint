#pragma once

#include "environment.hpp"
#include "parser.hpp"
#include <map>

using namespace std;

struct NumVal {
  variant<int, double> value;
};

struct RuntimeVal {
  variant<NullLiteral, IntLiteral, FloatLiteral, StringLiteral, BoolLiteral> value;
};

struct ObjectVal {
  map<string, RuntimeVal> properties;  
};

class Interpreter {
public:
  explicit Interpreter(Program program, Error error)
    : m_program(program), m_error(error), env(error) 
  {
  }

  RuntimeVal evaluate_program() {
    RuntimeVal lastEval;
    lastEval.value = NullLiteral();

    for (Stmt stmt : m_program.stmts) {
      lastEval = evaluate(stmt);
    }

    return lastEval;
  }

private:
  RuntimeVal evaluate(Stmt stmt) {
    switch (stmt.expr.index()) {
      // Expression
      case 0: {
        return eval_expr(get<Expr>(stmt.expr), env);
      }
      // Variable Declaration
      case 1: { 
        env.declare_var(get<VarDeclaration>(stmt.expr));
	return { NullLiteral() }; 
      }
      // Variable Assignment
      case 2: {
	env.assign_var(get<VarAssignment>(stmt.expr));
	return { NullLiteral() };
      }
      default: {
	return { NullLiteral () };
      } 
    }
  }

  RuntimeVal eval_expr(Expr expr, Environment env) {
    switch (expr.var.index()) {
      // Indetifier
      case 0: {
        string ident = get<Identifier>(expr.var).token.rawValue.value();
        return eval_expr(env.search_var(ident).value().expr.value(), env);
      }
      // Int Literal
      case 1: {
        return { get<IntLiteral>(expr.var) };
      }
      // Float Literal
      case 2: {
        return { get<FloatLiteral>(expr.var) };
      }
      // String Literal
      case 3: {
        return { get<StringLiteral>(expr.var) };
      }
      // Bool Literal
      case 4: {
        return { get<BoolLiteral>(expr.var) };
      }
      // Null Literal
      case 5: {
        return { get<NullLiteral>(expr.var) };
      }
      // Binary Expression
      case 6: {
        return eval_bin_expr(get<BinaryExpr*>(expr.var));
      }
      // TODO Object Literal
      case 7: {
        cout << "Object Literal\n";
        return { NullLiteral() };
      }
      // TODO Call Expr
      case 8: {
        cout << "Call Expr\n";
        return { NullLiteral() };
      }
      // TODO Member Expr
      case 9: {
        cout << "Member Expr\n";
        return { NullLiteral() };
      }
      default: {
        return { NullLiteral() };
      }
    } 
  }

  RuntimeVal eval_bin_expr(BinaryExpr* bin_expr) {
    RuntimeVal lhs = evaluate({ bin_expr->lhs });
    RuntimeVal rhs = evaluate({ bin_expr->rhs });
    
    // Evaluate NullLiteral
    if (lhs.value.index() == 0) {
      return rhs;
    }
    if (rhs.value.index() == 0) {
      return lhs;
    }

    // Numeric Binary Expr
    if ((lhs.value.index() == 1 || lhs.value.index() == 2) && 
      (rhs.value.index() == 1 || rhs.value.index() == 2)) {

      NumVal lhsNum, rhsNum;

      // Extract value of lhs and rhs into variant
      lhsNum.value = lhs.value.index() == 1 
        ? variant<int, double>(stoi(get<1>(lhs.value).token.rawValue.value()))
        : variant<int, double>(stod(get<2>(lhs.value).token.rawValue.value()));

      rhsNum.value = rhs.value.index() == 1 
        ? variant<int, double>(stoi(get<1>(rhs.value).token.rawValue.value()))
        : variant<int, double>(stod(get<2>(rhs.value).token.rawValue.value()));

      Token result;
      NumVal num = eval_numeric_bin_expr(lhsNum, rhsNum, bin_expr->operand);

      // Num result is an integer
      if (num.value.index() == 0) {
        result.rawValue = to_string(get<0>(num.value));
        result.type = TokenType::Int;

        IntLiteral intResult;
        intResult.token = result;

        return { intResult };
      }

      // Num result is a double
      else {
        result.rawValue = to_string(get<1>(num.value));
        result.type = TokenType::Float;

        FloatLiteral floatResult;
        floatResult.token = result;

        return { floatResult };
      }
    }

    // Concatonate Strings
    if (lhs.value.index() == 3 && rhs.value.index() == 2 && bin_expr->operand.type == TokenType::Plus) {
      StringLiteral lhsStr = get<3>(lhs.value);
      StringLiteral rhsStr = get<3>(rhs.value);

      StringLiteral concat;
      concat.token.rawValue = lhsStr.token.rawValue.value() + lhsStr.token.rawValue.value();

      return { concat };
    }

    auto visitor = [](auto&& arg) -> Token {
      using T = std::decay_t<decltype(arg)>;
      return arg.token; // Return the token for all other cases
    };

    m_error.error("Expression:" + get_type(lhs) + Error::to_string(bin_expr->operand.type) +
        get_type(rhs) + "is invalid.", bin_expr->operand);

    return RuntimeVal();
  }

  string get_type(RuntimeVal runtimeVal) {
    switch (runtimeVal.value.index()) {
      case 0:
        return " `null` ";
      case 1: 
      case 2:
        return " `number` ";
      case 3:
        return " `string` ";
      case 4:
        return " `boolean` ";
      default: 
        return "";
    }
  }

  NumVal eval_numeric_bin_expr(NumVal lhsNum, NumVal rhsNum, Token t_operand) {
    TokenType operand = t_operand.type;

    // Perform the arithmetic operation
    auto perform_operation = [&](auto lhs, auto rhs) -> NumVal {
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
            m_error.error("Division by zero.", t_operand);
          }
        case TokenType::Modulo:
          if (rhs != 0) // Check for modulo by zero
            return { static_cast<int>(lhs) % static_cast<int>(rhs) }; // Casting to int for modulo operation
          else {
            m_error.error("Modulo by zero.", t_operand);
          }
        default:
          m_error.error("Invalid operand.", t_operand);
          exit(EXIT_FAILURE);
      }
    };

    // Visit the variants and complete the arithmetic operation
    return visit(perform_operation, lhsNum.value, rhsNum.value);
  }

  const Program m_program;
  vector<VarDeclaration> vars;
  Error m_error;
  Environment env;
};

