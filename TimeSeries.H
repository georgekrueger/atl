
#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/signal.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <string>
#include <vector>
#include <list>
#include <pthread.h>
#include <iostream>

using namespace boost::posix_time;

namespace TimeSeries
{

class Node;
class Graph;
class GraphScheduler;

typedef std::vector<Node*> NodeList;
typedef Node* NodeHandle;
	
static const NodeHandle NULL_NODE_HND = NULL;

class Lock
{
	public:
		Lock(pthread_mutex_t* mutex) : mutex_(mutex) { pthread_mutex_lock(mutex); }
		~Lock() { pthread_mutex_unlock(mutex_); }

		void Release() { pthread_mutex_unlock(mutex_); }

	private:
		pthread_mutex_t* mutex_;
};

class Value
{
	public:
		enum Type
		{
			NONE,
			NUMBER,
			STRING
		};

		Value() : type_(NONE)
		{
		}

		bool IsUndefined() const 
		{
			return type_ == NONE; 
		}

		Type GetType() const
		{
			return type_;
		}

		const double& GetNumber() const
		{
			assert(!IsUndefined());
			return boost::get<double>(value_);
		}

		const std::string& GetString() const
		{
			assert(!IsUndefined());
			return boost::get<std::string>(value_);
		}

		Value& operator =(const double& v)
		{
			type_ = NUMBER;
			value_ = v;
			return *this;
		}

		Value& operator =(const std::string& v)
		{
			type_ = STRING;
			value_ = v;
			return *this;
		}

	private:
		Type  type_;
		boost::variant<double, std::string> value_;
		ptime timestamp_;
};

class Node
{
	public:
		Node();

	public:
		// Initialize Node.  Setup state, start any requests, etc...
		virtual Value Init() = 0;
		// Update output value in response one of it's child dependencies updating
		virtual Value Evaluate(unsigned int childUpdated) = 0;
		// Create a deep copy of thde node
		virtual Node* Clone() const = 0;

		const Value& GetCurrentValue() const { return currentValue_; }
		const Node* GetChild(unsigned int i) const { return children_[i]; }

	private:
		Node*     parent_;
		NodeList  children_;
		Graph*    graph_;
		Value     currentValue_;

	private:
		// Find index of node n in this node's children
		boost::optional<unsigned int> FindChildIndex(Node* n);

	friend class Graph;
};

class Graph
{
	public:
		Graph(GraphScheduler* scheduler);
		~Graph();
		// Adds a new node to graph
		NodeHandle AddNode(NodeHandle parent, const Node* node);
		// Start the graph
		void Start();

	private:
		Node* root_;
		GraphScheduler* scheduler_;
		pthread_mutex_t mutex_;

	private:
		bool Update(NodeHandle node, const Value& value);
		// Schedule an update for a given node on the graph
		void ScheduleUpdate(NodeHandle node, const Value& value);

	friend class GraphScheduler;
	friend class Node;
};

class GraphScheduler
{
	public:
		GraphScheduler();
		~GraphScheduler();

		struct Task
		{
			Graph* graph;
			NodeHandle node;
			Value value;
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

		virtual Value Init();
		virtual Value Evaluate(unsigned int childUpdated);
		virtual Node* Clone() const { return new NumberNode(*this); }

	private:
		double num_;
};

class ArithmeticNode : public Node
{
	public:
		enum Type
		{
			ADD,
			SUB,
			MUL,
			DIV
		};

		ArithmeticNode(Type type);
		virtual Value Init();
		virtual Value Evaluate(unsigned int childUpdated);
		virtual Node* Clone() const { return new ArithmeticNode(*this); }

	private:
		Type type_;
};
//
} // end TimeSeries namespace

#endif

