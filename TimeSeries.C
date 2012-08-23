
#include "TimeSeries.H"
#include <iostream>
#include <list>

using namespace std;
using namespace TimeSeries;

const Graph::NodeHandle Graph::NULL_HANDLE = NULL;

Node::Node(Graph* graph) : graph_(graph)
{
}

Graph::Graph(GraphScheduler* scheduler) : root_(NULL), scheduler_(scheduler)
{
	pthread_mutex_init(&mutex_, NULL);
}

Graph::~Graph()
{
	pthread_mutex_destroy(&mutex_);
}

Graph::NodeHandle Graph::AddNode(Graph::NodeHandle parent, const Node& node)
{
	Graph::NodeHandle n = node.Copy();
	if (parent == NULL_HANDLE) {
		root_ = n;
	}
	else {
		parent->children_.push_back(n);
		n->parent_ = parent;
	}
	return n;
}

void Graph::ConnectNodes(Graph::NodeHandle parent, Graph::NodeHandle child)
{
	assert(parent != NULL_HANDLE);
	assert(child != NULL_HANDLE);
	parent->children_.push_back(child);
	child->parent_ = parent;
}

void Graph::ScheduleUpdate(NodeHandle node, Value* value)
{
	GraphScheduler::Task task;
	task.graph = this;
	task.node = node;
	task.value = value;
	scheduler_->AddTask(task);
}

void Graph::Start()
{
	if (root_ == NULL)
		return;
	// call init on all nodes in breadth first order
	list<Node*> nodes;
	nodes.push_back(root_);
	while (!nodes.empty()) {
		Node* curr = nodes.front();
		Value* initValue = curr->Init();
		// schedule an update for the intial value if there is one
		if (initValue) {
			ScheduleUpdate(curr, initValue);
		}
		for (NodeList::iterator it = curr->children_.begin(); it != curr->children_.end(); it++) {
			nodes.push_back(*it);
		}
		nodes.pop_front();
	}
}

Value* Graph::Update(NodeHandle node, Value* value)
{
	// lock the graph
	pthread_mutex_lock(&mutex_);

	// start at given node and update every node that depends on it
	list<UpdatePair> updates;
	UpdatePair update;
	update.node = node;
	update.value = value;
	updates.push_back(update);

	while (!updates.empty()) {
		UpdatePair u = updates.front();
		updates.pop_front();
		Node* n = u.node;
		Value* v = u.value;
		Node* parent = n->parent_;
		if (parent == NULL) {
			return v;
		}

		// iterate parent's children to find child index
		signed long index = -1;	
		NodeList::iterator it;
		int i=0;
		for (it = parent->children_.begin(); it != parent->children_.end(); it++, i++) {
			if (*it == n) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			// update parent's child value
			parent->childValues_[index] = v;
			// check if all parent's child values exist
			bool undefinedChildren = false;
			ValueList::iterator valueIter;
			for (valueIter = parent->childValues_.begin(); valueIter != parent->childValues_.end(); valueIter++) {
				if (*valueIter == NULL) {
					undefinedChildren = true;
					break;
				}
			}
			if (undefinedChildren)
				// stop updating graph.  parent node can't be updated because all of it's children have not reported values.
				break;

			Value* parentValue = parent->Evaluate(index, v);
			if (!parentValue)
				break;

			UpdatePair parentUpdate;
			parentUpdate.node = parent;
			parentUpdate.value = parentValue;
			updates.push_back(parentUpdate);
		}
		else {
			cerr << "Unable to find node in parent's child list. Graph will not be fully evaluated." << endl;
			break;
		}
	}

	// unlock the graph
	pthread_mutex_unlock(&mutex_);

	return NULL;
}

/*void* GraphSchedulerLoop(void* arg)
{
	while (1) {
		Value* value = ((GraphScheduler*)arg)->Update();
		cout << "New Value: " << value << endl;
	}
}*/

GraphScheduler::GraphScheduler()
{
	pthread_mutex_init(&mutex_, NULL);
	//pthread_create(&thread_, NULL, GraphSchedulerLoop, (void*)this);
}

GraphScheduler::~GraphScheduler()
{
	pthread_mutex_destroy(&mutex_);
}

void GraphScheduler::AddTask(const Task& task)
{
	pthread_mutex_lock(&mutex_);
	scheduledTasks_.push_back(task);
	pthread_mutex_unlock(&mutex_);
}

void GraphScheduler::Update()
{
	pthread_mutex_lock(&mutex_);
	if (scheduledTasks_.empty()) {
		pthread_mutex_unlock(&mutex_);
		return;
	}
	Task task = scheduledTasks_.front();
	scheduledTasks_.pop_front();
	pthread_mutex_unlock(&mutex_);

	task.graph->Update(task.node, task.value);
}

NumberNode::NumberNode(signed int n) : Node(), type_(INT), int_(n)
{
}

NumberNode::NumberNode(float n) : Node(), type_(FLOAT), float_(n)
{
}

bool NumberNode::Init(Value& outputValue)
{
	switch (type_)
	{
		case INT:
			return int_;
		case FLOAT:
			return float_;
	}
}

ArithmeticNode::ArithmeticNode(Type type) : Node(), type_(type)
{
}


Value* ArithmeticNode::Evaluate(unsigned long childUpdated, Value* value)
{
	switch(type_)
	{

	}
}


