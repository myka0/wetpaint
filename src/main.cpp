#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
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

    Parser parser(tokens);
    Program program = parser.createAST();    

    Interpreter interpreter(program);
    RuntimeVal value = interpreter.evaluate_program();

    //string result = get<StringVal>(value.value).value;

    NumVal val = get<NumVal>(value.value);

    auto result = 0.0;
    if (val.value.index() == 0)
      result = get<int>(val.value);
    else
      result = get<double>(val.value); 

    cout << "Result: " << result << "\n";

    return EXIT_SUCCESS;
}
