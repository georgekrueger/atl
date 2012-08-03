../bison2.5/install/bin/bison -d -o parser.cpp parser.y
../flex-2.5.36/install/bin/flex -o tokens.cpp tokens.l
g++ -o parser parser.cpp tokens.cpp node.cpp main.cpp
