#include <iostream>
#include <vector>

//class CodeGenContext;
class NStatement;
class NExpression;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;

using std::cout;
using std::endl;

// Timeseries -- Move to new file
class TimeSeries
{
};

///////////////////////////////


enum ExpressionType
{
	None,
	Integer,
	Double,
	Identifier,
	MethodCall,
	BinaryExpression,
	Assignment,
	Block
};

class Node {
public:
    virtual ~Node() {}
    virtual TimeSeries* compile()  = 0;
};

class NExpression : public Node {
//private:
//	NExpression() {}
public:
	ExpressionType type;
//public:
//	virtual TimeSeries* compile() {}
};

class NStatement : public Node {
//private:
//	NStatement() {}
//public:
//	virtual TimeSeries* compile() {}
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { type = Integer; }
    virtual TimeSeries* compile();
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { type = Double; }
    virtual TimeSeries* compile();
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { type = Identifier; }
    virtual TimeSeries* compile();
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { type = MethodCall; }
    NMethodCall(const NIdentifier& id) : id(id) { type = MethodCall; }
    virtual TimeSeries* compile();
};

class NBinaryExpression : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryExpression(NExpression& lhs, int op, NExpression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { type = BinaryExpression; }
    virtual TimeSeries* compile();
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) :
        lhs(lhs), rhs(rhs) { type = Assignment; }
    virtual TimeSeries* compile();
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { type = Block; }
    virtual TimeSeries* compile();
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
        expression(expression) { }
    virtual TimeSeries* compile();
};

/*class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id,
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};*/



