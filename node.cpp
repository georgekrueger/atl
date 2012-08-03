#include <iostream>
#include <vector>
#include "node.H"

using namespace std;

TimeSeries* NInteger::compile()
{
	cout << "integer value: " << value << endl;
	return NULL;
}

TimeSeries* NDouble::compile()
{
	cout << "double value: " << value << endl;
	return NULL;
}

TimeSeries* NIdentifier::compile()
{
	cout << "identifier name" << name << endl;
	return NULL;
}

TimeSeries* NMethodCall::compile()
{
	cout << "method call" << endl;
	return NULL;
}

TimeSeries* NBinaryExpression::compile()
{
	cout << "binary expression op: " << op << endl;
	lhs.compile();
	rhs.compile();
	return NULL;
}

TimeSeries* NAssignment::compile()
{
	return NULL;
}

TimeSeries* NBlock::compile()
{
	StatementList::iterator iter;
	for (iter = statements.begin(); iter != statements.end(); iter++)
	{
		(*iter)->compile();
	}
	return NULL;
}

TimeSeries* NExpressionStatement::compile()
{
	expression.compile();
	return NULL;
}
