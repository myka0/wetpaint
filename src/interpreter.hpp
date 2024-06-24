#pragma once

#include "environment.hpp"

#include <algorithm>
#include <functional>

struct NumVal {
  std::variant<int, double> value;
};

class Interpreter {
public:
  explicit Interpreter(Program program, Error error)
    : m_program(program), m_error(error), env(error) 
  {
  }

  RuntimeVal evaluate_program() {
    RuntimeVal lastEval = { NullLiteral() };

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
        return eval_expr(std::get<Expr>(stmt.expr), env);
      }
      // Variable Declaration
      case 1: { 
        env.declare_var(std::get<VarDeclaration>(stmt.expr));
        return { NullLiteral() };
      }
      // Variable Assignment
      case 2: {
        env.assign_var(std::get<VarAssignment>(stmt.expr));
        return { NullLiteral() };
      }
      default: {
        return { NullLiteral() };
      } 
    }
  }

  RuntimeVal eval_expr(Expr expr, Environment env) {
    switch (expr.var.index()) {
      // Indetifier
      case 0: {
        Identifier ident = std::get<Identifier>(expr.var);
        return eval_expr(env.has_var(ident).expr.value(), env);
      }
      // Literal Types
      case 1:   // IntLiteral
      case 2:   // FloatLiteral
      case 3:   // StringLiteral
      case 4:   // BoolLiteral
      case 5: { // NullLiteral
        RuntimeVal runtimeVal = std::visit([](const auto& value) -> RuntimeVal {
          // Deduce the type of the Expr and return it as a RuntimeVal
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, IntLiteral> || std::is_same_v<T, FloatLiteral> || 
                        std::is_same_v<T, StringLiteral> || std::is_same_v<T, BoolLiteral> ||
                        std::is_same_v<T, NullLiteral>) {
            return { value };
          }
          return { NullLiteral() };
        }, expr.var);

        return runtimeVal;
      }
      // Binary Expression
      case 6: {
        return eval_bin_expr(std::get<BinaryExpr*>(expr.var));
      }
      // Object Literal
      case 7: {
        eval_object_literal(std::get<ObjectLiteral*>(expr.var), env);
        return { NullLiteral() };
      }
      // Call Expr
      case 8: {
        return eval_call_expr(std::get<CallExpr*>(expr.var), env);
      }
      // Member Expr
      case 9: {
        return eval_member_expr(std::get<MemberExpr*>(expr.var), env);
      }
      default: {
        return { NullLiteral() };
      }
    } 
  }

  void eval_object_literal(ObjectLiteral* object, Environment env) {
    for (Property property : object->properties) {
      // If property has a value declare the variable in the environment
      if (property.value.has_value()) {
        VarDeclaration declaration = { property.key, property.value.value() };
        env.declare_var(declaration);
      } 
      else { 
        // If the property does not have a value make sure it has already been declared
        env.has_var(property.key);
      }
    }
  }

  RuntimeVal eval_call_expr(CallExpr* callExpr, Environment env) {
    std::vector<RuntimeVal> args;
    for (Stmt arg : callExpr->args) {
      args.push_back(evaluate(arg));
    }

    // Retrieve the function identifier from the caller expression
    Identifier caller = std::get<Identifier>(callExpr->caller.var);
    Expr expr = env.has_var(caller).expr.value();

    // Call the fucntion with the arguments and return the result
    NativeFunction* fn = std::get<NativeFunction*>(expr.var);
    return fn->call(args);
  }

  RuntimeVal eval_member_expr(MemberExpr* memberExpr, Environment env) {
    Identifier object = memberExpr->object;
    Expr member = memberExpr->member;

    // Get the string representation of the identifier and search for it in the environment
    Expr expr = env.has_var(object).expr.value();

    // Loop while the expression is an ObjectLiteral
    while (std::holds_alternative<ObjectLiteral*>(expr.var)) {
      const std::vector<Property> properties = std::get<7>(expr.var)->properties;

      // If the member is a nested MemberExpr, update the object and member
      if (std::holds_alternative<MemberExpr*>(member.var)) {
        object = std::get<MemberExpr*>(member.var)->object;
        member = std::get<MemberExpr*>(member.var)->member;
      } 
      // Otherwise, the member is an Identifier
      else {
        object = std::get<Identifier>(member.var);
      }

      // Find the property in the object literal with the matching key
      std::string ident = object.token.rawValue.value();
      auto it = std::find_if(properties.begin(), properties.end(), [&ident](const Property& property) {
        return property.key.token.rawValue.value() == ident;
      });

      // Check if the property is not found or has no value
      if (it == properties.end()) {
        m_error.report_error("Member: `" + ident + "` was not found in Object.", object.token);
      }

      expr = it->value.value();
    }

    return eval_expr(expr, env);
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
        ? std::variant<int, double>(stoi(std::get<1>(lhs.value).token.rawValue.value()))
        : std::variant<int, double>(stod(std::get<2>(lhs.value).token.rawValue.value()));

      rhsNum.value = rhs.value.index() == 1 
        ? std::variant<int, double>(stoi(std::get<1>(rhs.value).token.rawValue.value()))
        : std::variant<int, double>(stod(std::get<2>(rhs.value).token.rawValue.value()));

      Token result;
      NumVal num = eval_numeric_bin_expr(lhsNum, rhsNum, bin_expr->operand);

      // Num result is an integer
      if (num.value.index() == 0) {
        result.rawValue = std::to_string(std::get<0>(num.value));
        result.type = TokenType::Int;

        IntLiteral intResult;
        intResult.token = result;

        return { intResult };
      }

      // Num result is a double
      else {
        result.rawValue = std::to_string(std::get<1>(num.value));
        result.type = TokenType::Float;

        FloatLiteral floatResult;
        floatResult.token = result;

        return { floatResult };
      }
    }

    // Concatonate strings
    if (lhs.value.index() == 3 && rhs.value.index() == 2 && bin_expr->operand.type == TokenType::Plus) {
      StringLiteral lhsStr = std::get<3>(lhs.value);
      StringLiteral rhsStr = std::get<3>(rhs.value);

      StringLiteral concat;
      concat.token.rawValue = lhsStr.token.rawValue.value() + lhsStr.token.rawValue.value();

      return { concat };
    }

    // Else Binary Expr is invalid
    m_error.report_error("Expression:" + get_type(lhs) + Error::to_string(bin_expr->operand.type) +
        get_type(rhs) + "is invalid.", bin_expr->operand);

    return RuntimeVal();
  }

  std::string get_type(RuntimeVal runtimeVal) {
    switch (runtimeVal.value.index()) {
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
            m_error.report_error("Division by zero.", t_operand);
          }
        case TokenType::Modulo:
          if (rhs != 0) // Check for modulo by zero
            return { static_cast<int>(lhs) % static_cast<int>(rhs) }; // Casting to int for modulo operation
          else {
            m_error.report_error("Modulo by zero.", t_operand);
          }
        default:
          m_error.report_error("Invalid operand.", t_operand);
          exit(EXIT_FAILURE);
      }
    };

    // Visit the variants and complete the arithmetic operation
    return visit(perform_operation, lhsNum.value, rhsNum.value);
  }

private:
  const Program m_program;
  std::vector<VarDeclaration> vars;
  Error m_error;
  Environment env;
};

