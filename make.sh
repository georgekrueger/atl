../bison2.5/install/bin/bison -d -o parser.cpp parser.y
../flex-2.5.36/install/bin/flex -o tokens.cpp tokens.l
g++ -o parser -I/opt/app/g++lib6/boost-1.47/include/boost-1_47 parser.cpp tokens.cpp node.cpp TimeSeries.C main.cpp
