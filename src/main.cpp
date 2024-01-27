#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "./tokenize.hpp"
#include "./parser.hpp"

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

    return EXIT_SUCCESS;
}

