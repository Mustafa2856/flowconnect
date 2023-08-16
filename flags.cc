#include <string>

#define FLAGS_INCLUDED

enum FLAGS {f=0,g,i,v};
// getConfigFromFile; | -f
// singleNodeGroup;   | -g
// getFromStdInput;   | -i
// verbose;           | -v
const std::string FLAGS = "fgiv";

