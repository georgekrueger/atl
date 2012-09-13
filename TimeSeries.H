
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
		Value() : type_(NONE)
		{
		}

		bool IsValid()
		{
			return _type != NONE; 
		}

		Type GetType()
		{
			return type_;
		}

		void SetNumber(double n)
		{
			type_ = NUMBER;
			numberValue_ = n;
		}

		double GetNumber()
		{
			return numberValue_;
		}

		void SetString(std::string s)
		{
			type_ = STRING;
			stringValue_ = s;
		}

		std::string GetString()
		{
			return stringValue_;
		}

		enum Type
		{
			NONE,
			NUMBER,
			STRING
		};

	private:
		Type  type_;
		union
		{
			double numberValue_;
			std::string stringValue_;
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
		double num_;
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
//
} // end TimeSeries namespace

#endif

