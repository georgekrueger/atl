bison -d -o parser.cpp parser.y
flex -otokens.cpp tokens.l
g++ -o parser -g -O0 -fno-inline -I/opt/TWWfsw/libboost147/include parser.cpp tokens.cpp node.cpp TimeSeries.C main.cpp
