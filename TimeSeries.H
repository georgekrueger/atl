
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

	private:
		ptime timestamp;
};

class IntegerValue : public Value
{
	public:
		IntegerValue(signed long value);
};

class DoubleValue : public Value
{
	public:
		DoubleValue(double value);
};

class StringValue : public Value
{
	public:
		StringValue(std::string value);
};

typedef std::vector<Node*> NodeList;
typedef std::vector<Value*> ValueList;

class Node
{
	public:
		Node(Graph* graph);

		// Initialize Node.  Setup state, start any requests, etc...
		virtual Value* Init() = 0;
		// Update value in response to a dependency update
		virtual Value* Evaluate(unsigned long childUpdated, Value* value) = 0;
		// Create a deep copy of node
		virtual Node* Copy() const = 0;

	private:
		Node* parent_;
		NodeList children_;
		ValueList childValues_;
		Graph*   graph_;

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

} // end TimeSeries namespace

#endif

