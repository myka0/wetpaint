#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "./interpreter.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
      cerr << "No input file detected. Correct usage is..." << endl;  
      cerr << "paint <input.wp>" << endl;  
      return EXIT_FAILURE;
    }
    
    // Read file into contents
    string contents;
    {
      stringstream contents_stream;
      fstream input(argv[1], ios::in);
      contents_stream << input.rdbuf();
      contents = contents_stream.str();
    }
    
    Tokenizer tokenizer(contents);
    vector<Token> tokens = tokenizer.tokenize();

    Error error(tokens);

    Parser parser(tokens, error);
    Program program = parser.createAST();    

    Interpreter interpreter(program, error);
    RuntimeVal runtimeVal = interpreter.evaluate_program();
    
    Token token;
    int index = runtimeVal.value.index();

    if (index == 1) 
      token = get<1>(runtimeVal.value).token;
    else if (index == 2) 
      token = get<2>(runtimeVal.value).token;
    else if (index == 3) 
      token = get<3>(runtimeVal.value).token;
    else if (index == 4) 
      token = get<4>(runtimeVal.value).token;

    cout << "Index: " << runtimeVal.value.index() << " | Result: " << token.rawValue.value() << "\n";

    return EXIT_SUCCESS;
}
