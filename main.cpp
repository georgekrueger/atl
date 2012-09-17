#include <iostream>
//#include "node.H"
#include "TimeSeries.H"
//extern NBlock* programBlock;
//extern int yyparse();

using namespace TimeSeries;

int main(int argc, char **argv)
{
    //yyparse();
    //std::cout << programBlock << std::endl;
	//programBlock->compile();

	NumberNode numNode1(10);
	NumberNode numNode2(5);
	ArithmeticNode addNode(ArithmeticNode::ADD);

	GraphScheduler scheduler;
	Graph graph(&scheduler);
	NodeHandle hnd = graph.AddNode(NULL_NODE_HND, &addNode);
	graph.AddNode(hnd, &numNode1);
	graph.AddNode(hnd, &numNode2);
	graph.Start();

	while(1)
	{
		scheduler.Update();
	}

    return 0;
}
