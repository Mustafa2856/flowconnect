#include <string>

#define FLAGS_INCLUDED
// getConfigFromFile; | -f
// getFromStdInput;   | -i
// verbose;           | -v
const int NUM_FLAGS = 3;
enum FLAGS {InputFromFile,InputFromStdin,Verbose};
const std::string FLAGS = "fiv";

struct FLAG_LIST {  
  bool is[NUM_FLAGS];

  FLAG_LIST();
};

FLAG_LIST::FLAG_LIST() {
  for(int i=0;i<NUM_FLAGS;i++) {
    is[i] = 0;
  }
}
