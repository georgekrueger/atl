#include <iostream>
#include "node.H"
extern NBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << programBlock << std::endl;
	programBlock->compile();
    return 0;
}
