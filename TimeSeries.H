
#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/signal.hpp>
#include <string>
#include <vector>
#include <list>
#include <pthread.h>

using namespace boost::posix_time;

namespace TimeSeries
{

class Node;
class Graph;
class GraphScheduler;

class Value
{
	public:
		Value();
		Value Add(const Value& v);
		Value Sub(const Value& v);
		Value Mul(const Value& v);
		Value Div(const Value& v);

	private:
		ptime timestamp;
};

class IntegerValue : public Value
{
	public:
		IntegerValue(signed long value);

	private:
		signed int value;
};

class RealValue : public Value
{
	public:
		RealValue(double value);

	private:
		float value;
};

class StringValue : public Value
{
	public:
		StringValue(std::string value);

	private:
		std::string value;
};

class Value
{
	public:
		Value() : inited_(false), type_(NONE)
		{

		}

		void SetInt(signed int v)
		{
			inited_ = true;
			type_ = INT;
			intValue_ = v;
		}
		signed int GetInt() 
		{
			return intValue_;
		}

		void SetFloat(float v)
		{
			inited_ = true;
			type_ = FLOAT;
			floatValue_ = v;
		}

		float GetFloat()
		{
			return float_;
		}

		void SetString(string s)
		{
			inited_ = true;
			type_ = STRING;
			stringValue_ = s;
		}

		enum Type
		{
			NONE,
			INT,
			FLOAT,
			STRING
		};

	private:
		bool  inited_;
		Type  type_;
		union
		{
			signed int intValue;
			float floatValue;
			string stringValue;
		};
		ptime timestamp_;
};

typedef std::vector<Node*> NodeList;
typedef std::vector<Value*> ValueList;

class Node
{
	public:
		Node(Graph* graph);

		// Initialize Node.  Setup state, start any requests, etc...
		virtual bool Init(Value& outputValue) = 0;
		// Update output value in response one of it's child dependencies updating.  When
		// this method is called all dependencies will be defined.
		virtual bool Evaluate(unsigned long childUpdated, const Value& childValue, Value& outputValue) = 0;
		// Create a deep copy of thde node
		virtual Node* Copy() const = 0;

	private:
		Node*     parent_;
		NodeList  children_;
		ValueList childValues_;
		Graph*    graph_;
		Value     currOutputValue_;

	friend class Graph;
};

class Graph
{
	struct UpdatePair
	{
		Node* node;
		Value* value;
	};
	
	typedef Node* NodeHandle;
	static const NodeHandle NULL_HANDLE;

	public:
		Graph(GraphScheduler* scheduler);
		~Graph();
		// Adds a new node to graph
		NodeHandle AddNode(NodeHandle parent, const Node& node);
		// Connect nodes in graph
		void ConnectNodes(NodeHandle parent, NodeHandle child);
		// Schedule an update for a given node on the graph
		void ScheduleUpdate(NodeHandle node, Value* value);
		// Start the graph
		void Start();

	private:
		Node* root_;
		GraphScheduler* scheduler_;
		pthread_mutex_t mutex_;

	private:
		Value* Update(NodeHandle node, Value* value);

	friend class GraphScheduler;
};

class GraphScheduler
{
	public:
		GraphScheduler();
		~GraphScheduler();

		struct Task
		{
			Graph* graph;
			Graph::NodeHandle node;
			Value* value;
		};
		void AddTask(const Task& task);
		void Update();

	private:
		std::list<Task> scheduledTasks_;
		pthread_t thread_;
		pthread_mutex_t mutex_;
};

class NumberNode : public Node
{
	public:
		NumberNode(signed int n);
		NumberNode(float n);

		virtual Value* Init();
		virtual Value* Evaluate(unsigned long childUpdated, Value* value) {}
		virtual Node* Copy() const { return new NumberNode(this); }

	private:
		enum Type
		{
			INT,
			FLOAT
		};
		Type type_;
		float float_;
		signed int int_;
};

class ArithmeticNode : public Node
{
	enum Type
	{
		ADD,
		SUB,
		MUL,
		DIV
	};

	public:
		ArithmeticNode(Type type);

		virtual Value* Init();
		virtual Value* Evaluate(unsigned long childUpdated, Value* value);
		virtual Node* Copy() const { return new ArithmeticNode(this); }

	private:
		Type type_;
};

} // end TimeSeries namespace

#endif

