#pragma once

#include "environment.hpp"

#include <variant>

class Interpreter {
public:
  explicit Interpreter(Program program, Error error, Environment env)
    : m_program(std::move(program)), m_error(std::move(error)), m_env(std::move(env)) 
  {
  }

  RuntimeVal evaluate_program() {
    RuntimeVal last_eval{ NullLiteral() };

    for (Stmt stmt : m_program.stmts) {
      last_eval = evaluate(stmt);

      if (last_eval.is<ReturnExpr>()) {
        return eval_expr(last_eval.get<ReturnExpr>().expr);
      }
    }

    return last_eval;
  }

private:
  RuntimeVal evaluate(Stmt stmt) {
    if (stmt.is<Expr>()) {
      Expr expr = stmt.get<Expr>();
      if (expr.is<ReturnExpr>()) {
        return expr.get<ReturnExpr>();
      }

      return eval_expr(expr);
    }
    else if (stmt.is<VarDeclaration>()) {
      m_env.declare_var(stmt.get<VarDeclaration>());
      return NullLiteral();
    }
    else if (stmt.is<VarAssignment>()) {
      m_env.assign_var(stmt.get<VarAssignment>());
      return NullLiteral();
    }
    else if (stmt.is<FunctionDeclaration>()) {
      FunctionDeclaration function_dec = stmt.get<FunctionDeclaration>(); 
      std::shared_ptr<Environment> env = std::make_shared<Environment>(m_env);

      Function function{ function_dec, env };
      m_env.declare_var(VarDeclaration{ function_dec.name, function, true });
      return NullLiteral();
    }
    else if (stmt.is<ConditionalBlock>()) {
      return eval_conditional(stmt.get<ConditionalBlock>());
    }
    else if (stmt.is<ForLoop>()) {
      return eval_for_loop(stmt.get<ForLoop>());
    }
    else if (stmt.is<WhileLoop>()) {
      return eval_while_loop(stmt.get<WhileLoop>());
    }
    else {
      return NullLiteral();
    }
  }

  RuntimeVal eval_expr(Expr expr) {
    // Map for handling literals
    static const std::unordered_map<std::type_index, std::function<RuntimeVal(const Expr&)>> literals {
      { typeid(IntLiteral), [](const Expr& e) { return e.get<IntLiteral>(); } },
      { typeid(FloatLiteral), [](const Expr& e) { return e.get<FloatLiteral>(); } },
      { typeid(StringLiteral), [](const Expr& e) { return e.get<StringLiteral>(); } },
      { typeid(BoolLiteral), [](const Expr& e) { return e.get<BoolLiteral>(); } },
      { typeid(NullLiteral), [](const Expr&) { return NullLiteral(); } }
    };

    // Get the value of literal types
    auto it = literals.find(expr.type());
    if (it != literals.end()) {
      return it->second(expr);
    }

    // Handle Identifier
    if (expr.is<Identifier>()) {
      Identifier ident = expr.get<Identifier>();
      return eval_expr(m_env.search_var(ident).expr.value());
    }
    // Handle other expression types
    else if (expr.is<BinaryExpr>()) {
      return eval_bin_expr(expr.get<BinaryExpr>());
    }
    else if (expr.is<BoolExpr>()) {
      bool boolean = eval_bool_expr(expr.get<BoolExpr>());
      Token token = boolean ? Token{ TokenType::True, 0, "true" } 
                            : Token{ TokenType::False, 0, "false" };
      return BoolLiteral{ boolean, token };
    }
    else if (expr.is<ObjectLiteral>()) {
      return eval_object_literal(expr.get<ObjectLiteral>());
    }
    else if (expr.is<CallExpr>()) {
      return eval_call_expr(expr.get<CallExpr>());
    }
    else if (expr.is<MemberExpr>()) {
      return eval_member_expr(expr.get<MemberExpr>());
    }
    else if (expr.is<Increment>()) {
      return eval_increment(expr.get<Increment>());
    }
    else if (expr.is<RuntimeVal>()) {
      return expr.get<RuntimeVal>();
    }
    else {
      return NullLiteral();
    }
  }

  RuntimeVal eval_conditional(ConditionalBlock block) {
    for (ConditionalStmt stmt : block.stmts) {
      // Check if the statement's condition has no value or evaluates to true     
      if (!stmt.condition.has_value() || eval_bool_expr(stmt.condition.value())) {
        eval_body(stmt.body);
        return NullLiteral();
      }
    }

    return NullLiteral();
  }

  RuntimeVal eval_for_loop(ForLoop loop) {
    VarAssignment variable = loop.variable;
    bool variable_exists = m_env.has_var(variable.identifier).has_value();

    // Declare the variable if it doesn't already exist
    if (!variable_exists) {
      VarDeclaration declaration{ variable.identifier, variable.expr, false };
      m_env.declare_var(declaration);
    }
    
    m_env.assign_var(variable);

    // Evaluate the loop condition and body
    while (eval_bool_expr(loop.condition)) {
      eval_body(loop.body);
      eval_expr(loop.counter);
    }

    // Restore the environment to its original state
    if (!variable_exists) {
      m_env.restore_scope(m_env.size() - 1);
    } else {
      m_env.assign_var(variable);
    }

    return NullLiteral();    
  }

  RuntimeVal eval_while_loop(WhileLoop loop) {
    // Evaluate the loop condition and body   
    while (eval_bool_expr(loop.condition)) {
      eval_body(loop.body);
    }

    return NullLiteral();
  }

  void eval_body(std::vector<Stmt> body) {
    // Save the current size of the environment stack
    size_t size = m_env.size();    

    for (Stmt stmt : body) {
      evaluate(stmt);
    }

    // Restore the environment to its original state   
    m_env.restore_scope(size);
  }

  RuntimeVal eval_object_literal(ObjectLiteral object) {
    for (Property property : object.properties) {
      // If property has a value declare the variable in the environment
      if (property.value.has_value()) {
        VarDeclaration declaration = { property.key, property.value.value() };
        m_env.declare_var(declaration);
      } 
      else { 
        // If the property does not have a value make sure it has already been declared
        m_env.search_var(property.key);
      }
    }
    
    return NullLiteral();
  }

  RuntimeVal eval_call_expr(CallExpr call_expr) {
    std::vector<RuntimeVal> args;
    for (Stmt arg : call_expr.args) {
      args.emplace_back(evaluate(arg));
    }

    // Retrieve the function identifier from the caller expression
    Identifier caller = call_expr.caller.get<Identifier>();
    Expr expr = m_env.search_var(caller).expr.value();

    // Call the fucntion with the arguments and return the result
    auto native_fn = expr.get_if<NativeFunction>();
    if (native_fn) {
      return native_fn->call(args);
    }

    auto function = expr.get_if<Function>();
    if (!function) {
      m_error.report_error("Function `" + caller.token.raw_value.value() + 
          "` not declared in scope.", caller.token);
    }

    std::shared_ptr<Environment> fn_env = (*function).env;
    FunctionDeclaration function_dec = (*function).declaration;

    if (args.size() != function_dec.params.size()) {
      m_error.report_error("Number of arguments does not match function declaration.\n" 
          "Expected " + std::to_string(function_dec.params.size()) + " arguments for function: " +
          caller.token.raw_value.value(), caller.token);
    }

    // Create variables for the param list
    for (int idx = 0; idx < args.size(); ++idx) {
      if (fn_env->has_var(function_dec.params[idx]).has_value()) {
        fn_env->assign_var(VarAssignment{ function_dec.params[idx], args[idx] });
      } else {
        fn_env->declare_var(VarDeclaration{ function_dec.params[idx], args[idx], false });
      }
    }

    Interpreter interpreter(Program{ function_dec.body } , m_error, *fn_env);
    RuntimeVal value = interpreter.evaluate_program();
    return value;
  }

  RuntimeVal eval_member_expr(MemberExpr member_expr) {
    Identifier object = member_expr.object;
    Expr member = member_expr.member;

    // Get the string representation of the identifier and search for it in the environment
    Expr expr = m_env.search_var(object).expr.value();

    // Loop while the expression is an ObjectLiteral
    while (expr.is<ObjectLiteral>()) {
      const std::vector<Property> properties = expr.get<ObjectLiteral>().properties;

      // If the member is a nested MemberExpr, update the object and member
      if (auto parent = member.get_if<MemberExpr>()) {
        object = (*parent).object;
        member = (*parent).member;
      } 
      // Otherwise, the member is an Identifier
      else {
        object = member.get<Identifier>();
      }

      // Find the property in the object literal with the matching key
      std::string ident = object.token.raw_value.value();
      auto it = std::find_if(properties.begin(), properties.end(), [&ident](const Property& property) {
        return property.key.token.raw_value.value() == ident;
      });

      // Check if the property is not found or has no value
      if (it == properties.end()) {
        m_error.report_error("Member: `" + ident + "` was not found in Object.", object.token);
      }

      if (it->value.has_value()) {
        expr = it->value.value();
      } else {
        expr = m_env.search_var(it->key).expr.value();
      }
    }

    return eval_expr(expr);
  }

  RuntimeVal eval_increment(Increment variable) {
    IntLiteral one_literal{ Token{ TokenType::Int, 0, "1" } };
    BinaryExpr increment{ variable.identifier, one_literal, variable.operand };
    RuntimeVal incremented_val = eval_bin_expr(increment);

    VarAssignment assignment{ variable.identifier, incremented_val };
    m_env.assign_var(assignment);

    return incremented_val;
  }

  bool eval_bool_expr(BoolExpr expr) {
    TokenType operand = expr.operand.type;
    auto lhs = eval_expr(expr.lhs).get_token().raw_value.value();
    auto rhs = eval_expr(expr.rhs).get_token().raw_value.value();

    switch (operand) {
      case TokenType::Equals:
        return lhs == rhs;
      case TokenType::Not:
        return lhs != rhs;
      case TokenType::Greater:
        return std::stoi(lhs) > std::stoi(rhs);
      case TokenType::Less:
        return std::stoi(lhs) < std::stoi(rhs);
      case TokenType::GreaterEquals:
        return std::stoi(lhs) >= std::stoi(rhs);
      case TokenType::LessEquals:
        return std::stoi(lhs) <= std::stoi(rhs);
      case TokenType::And:
        return eval_expr(expr.lhs).get<BoolLiteral>().value 
            && eval_expr(expr.rhs).get<BoolLiteral>().value;
      case TokenType::Or: 
        return eval_expr(expr.lhs).get<BoolLiteral>().value 
            || eval_expr(expr.rhs).get<BoolLiteral>().value;
      default:
        m_error.report_error("Unsupported operand in boolean expression.", expr.operand);
    }
  }

  RuntimeVal eval_bin_expr(BinaryExpr bin_expr) {
    RuntimeVal lhs = evaluate(Stmt{ bin_expr.lhs });
    RuntimeVal rhs = evaluate(Stmt{ bin_expr.rhs });
    
    // Evaluate NullLiteral
    if (lhs.is<NullLiteral>()) {
      return rhs;
    }
    if (rhs.is<NullLiteral>()) {
      return lhs;
    }

    // Numeric Binary Expr
    if ((lhs.is<IntLiteral>() || lhs.is<FloatLiteral>()) && 
        (rhs.is<IntLiteral>() || rhs.is<FloatLiteral>())) {

      auto lhs_num = get_numeric_value(lhs);
      auto rhs_num = get_numeric_value(rhs);
      auto num = eval_numeric_bin_expr(lhs_num, rhs_num, bin_expr.operand);

      // Num result is an integer
      if (num.index() == 0) {
        Token result{ TokenType::Int, 0, std::to_string(std::get<int>(num)) };
        return IntLiteral{ result };
      }
      // Num result is a double
      else {
        Token result{ TokenType::Float, 0, std::to_string(std::get<double>(num)) };
        return FloatLiteral{ result };
      }
    }

    // Concatonate strings
    auto lhs_str = lhs.get_if<StringLiteral>();
    auto rhs_str = rhs.get_if<StringLiteral>(); 

    if ( lhs_str && rhs_str && bin_expr.operand.type == TokenType::Plus) {
      StringLiteral concat;
      concat.token.raw_value = lhs_str->token.raw_value.value() + rhs_str->token.raw_value.value();
      return concat;
    }

    // Else Binary Expr is invalid
    m_error.report_error("Expression:" + 
        Error::to_string(lhs.get_token().type) +
        Error::to_string(bin_expr.operand.type) +
        Error::to_string(rhs.get_token().type) +
        "is invalid.", bin_expr.operand);

    return RuntimeVal();
  }

  std::variant<int, double> get_numeric_value(const RuntimeVal val) {
    return val.is<IntLiteral>() ? std::variant<int, double>(std::stoi(val.get_token().raw_value.value()))
                                : std::variant<int, double>(std::stod(val.get_token().raw_value.value()));
  }

  std::variant<int, double> eval_numeric_bin_expr(std::variant<int, double> lhs_num, 
      std::variant<int, double> rhs_num, Token t_operand) {
    TokenType operand = t_operand.type;

    // Perform the arithmetic operation
    auto perform_operation = [&](auto lhs, auto rhs) -> std::variant<int, double> {
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
      }
    };

    // Visit the variants and complete the arithmetic operation
    return visit(perform_operation, lhs_num, rhs_num);
  }

private:
  const Program m_program;
  std::vector<VarDeclaration> vars;
  Error m_error;
  Environment m_env;
};

