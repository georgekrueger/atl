
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

class Lock
{
	public:
		Lock(pthread_mutex_t* mutex) : mutex_(mutex) { pthread_mutex_lock(mutex); }
		~Lock() { pthread_mutex_unlock(mutex_); }

		Release() { pthread_mutex_unlock(mutex_); }

	private:
		pthread_mutex_t* mutex_;
};

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

		const double& GetNumber()
		{
			return numberValue_;
		}

		void SetString(std::string s)
		{
			type_ = STRING;
			stringValue_ = s;
		}

		const std::string& GetString()
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

bool Add(const Value& v1, const Value& v2, Value& out)
{
	if (v1.GetType() == NUMBER && v2.GetType() == NUMBER) {
		out.SetNumber(v1.GetNumber() + v1.GetNumber());
	}
	else if (v1.GetType() == STRING && v2.GetType() == STRING) {
		out.SetString(v1.GetString() + v2.GetString());
	}
	else {
		return false;
	}
	return true;
}


bool Subtract(const Value& v1, const Value& v2, Value& out)
{
	if (v1.GetType() == NUMBER && v2.GetType() == NUMBER) {
		out.SetNumber(v1.GetNumber() - v1.GetNumber());
	}
	else {
		return false;
	}
	return true;
}

bool Multiply(const Value& v1, const Value& v2, Value& out)
{
	if (v1.GetType() == NUMBER && v2.GetType() == NUMBER) {
		out.SetNumber(v1.GetNumber() * v1.GetNumber());
	}
	else {
		return false;
	}
	return true;
}

bool Divide(const Value& v1, const Value& v2, Value& out)
{
	if (v1.GetType() == NUMBER && v2.GetType() == NUMBER) {
		out.SetNumber(v1.GetNumber() / v1.GetNumber());
	}
	else {
		return false;
	}
	return true;
}

class Node
{
	public:
		Node(Graph* graph);

		typedef unsigned int ChildIndex;

		// Initialize Node.  Setup state, start any requests, etc...
		virtual bool Init(Value& outputValue) = 0;
		// Update output value in response one of it's child dependencies updating
		virtual bool Evaluate(ChildIndex childUpdated, Value& outputValue) = 0;
		// Create a deep copy of thde node
		virtual Node* Copy() const = 0;

	private:
		Node*     parent_;
		std::vector<Node*>  children_;
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
		NumberNode(double n);

		virtual bool Init(Value& outputValue);
		virtual bool Evaluate(ChildIndex childUpdated, Value& outputValue) {}
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

		virtual bool Init(Value& outputValue) {}
		virtual bool Evaluate(ChildIndex childUpdated, Value& outputValue);
		virtual Node* Copy() const { return new ArithmeticNode(this); }

	private:
		Type type_;
};
//
} // end TimeSeries namespace

#endif

