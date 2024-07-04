#include <fstream>
#include <sstream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
      std::cerr << "No input file detected. Correct usage is...\n";  
      std::cerr << "paint <input.wp>\n";  
      return EXIT_FAILURE;
    }
    
    // Read file into contents
    std::string contents;
    std::stringstream contents_stream;
    std::fstream input(argv[1], std::ios::in);
    contents_stream << input.rdbuf();
    contents = contents_stream.str();
    
    Tokenizer tokenizer(contents);
    std::vector<Token> tokens = tokenizer.tokenize();

    Error error(tokens);

    Parser parser(tokens, error);
    Program program = parser.create_ast();    

    Environment env(error);
    Interpreter interpreter(program, error, env);
    RuntimeVal runtimeVal = interpreter.evaluate_program();

    return EXIT_SUCCESS;
}
