
#include <boost/date_time/posix_time/ptime.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace boost::posix_time;

namespace TimeSeries
{

class Node;
class Graph;

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
		// Add the given node as a parent
		//void AddParent(Node* node);
		// Child node has updated to new value
		void ChildUpdated(Node* node, Value* value);

	private:
		NodeList parents_;
		NodeList children_;
		Graph*   graph_;

	friend class Graph;
};

Node::Node(Graph* graph) : graph_(graph)
{
}

void Node::ChildUpdated(Node* node, Value* value)
{
	bool childFound = false;
	unsigned long childIndex = 0;	
	NodeList::iterator it;
	int i=0;
	for (it = children_.begin(); it != children_.end(); it++, i++) {
		if (*it == node) {
			childIndex = i;
			childFound = true;
			break;
		}
	}

	if (!childFound) {
		std::cerr << "Error in Node::ChildUpdated. Unable to find node " << node << " as child";
		return;
	}

	Evaluate(childIndex, value);
}


class Graph
{
	struct Update
	{
		Node* node;
		Value* value;
	};
	
	typedef Node* NodeHandle;
	static const NodeHandle NULL_HANDLE;

	public:
		Graph();
		// Adds a new node to 
		NodeHandle AddNode(NodeHandle parent, const Node& node);
		void ConnectNodes(NodeHandle parent, NodeHandle child);
		void ScheduleUpdate(NodeHandle node, Value* value);

	private:
		Node* root_;
		std::vector<Update> scheduledUpdates_;
};

const Graph::NodeHandle Graph::NULL_HANDLE = NULL;

Graph::NodeHandle Graph::AddNode(Graph::NodeHandle parent, const Node& node)
{
	Graph::NodeHandle n = node.Copy();
	if (parent == NULL_HANDLE) {
		root_ = n;
	}
	else {
		parent->children_.push_back(n);
		n->parents_.push_back(parent);
	}
	return n;
}

void Graph::ConnectNodes(Graph::NodeHandle parent, Graph::NodeHandle child)
{
	assert(parent != NULL_HANDLE);
	assert(child != NULL_HANDLE);
	parent->children_.push_back(child);
	child->parents_.push_back(parent);
}

void Graph::ScheduleUpdate(Node* node, Value* value)
{
	Update u;
	u.node = node;
	u.value = value;
	scheduledUpdates_.push_back(u);
}

} // end TimeSeries namespace

