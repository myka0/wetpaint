#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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
    
    cout << contents << endl;

    return EXIT_SUCCESS;
}
