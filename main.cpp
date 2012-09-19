#include <iostream>
//#include "node.H"
#include "TimeSeries.H"
//extern NBlock* programBlock;
//extern int yyparse();

using namespace TimeSeries;
using namespace std;

int main(int argc, char **argv)
{
    //yyparse();
    //std::cout << programBlock << std::endl;
	//programBlock->compile();

	cout << "sizeof(Value): " << sizeof(Value) << endl;

	boost::optional<Value> v;

	cout << sizeof(v) << endl;

	NumberNode numNode1(10);
	TimedNode numNode2;
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
