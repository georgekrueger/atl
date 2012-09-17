
#include "TimeSeries.H"
#include <iostream>
#include <list>

using namespace std;
using namespace boost;
using namespace TimeSeries;

Value Add(const Value& v1, const Value& v2)
{
	Value val;
	if (v1.GetType() == Value::NUMBER && v2.GetType() == Value::NUMBER) {
		val = v1.GetNumber() + v1.GetNumber();
	}
	else if (v1.GetType() == Value::STRING && v2.GetType() == Value::STRING) {
		val = v1.GetString() + v2.GetString();
	}
	return val;
}


Value Subtract(const Value& v1, const Value& v2)
{
	Value val;
	if (v1.GetType() == Value::NUMBER && v2.GetType() == Value::NUMBER) {
		val = v1.GetNumber() - v1.GetNumber();
	}
	return val;
}

Value Multiply(const Value& v1, const Value& v2)
{
	Value val;
	if (v1.GetType() == Value::NUMBER && v2.GetType() == Value::NUMBER) {
		val = v1.GetNumber() * v1.GetNumber();
	}
	return val;
}

Value Divide(const Value& v1, const Value& v2)
{
	Value val;
	if (v1.GetType() == Value::NUMBER && v2.GetType() == Value::NUMBER) {
		val = v1.GetNumber() / v1.GetNumber();
	}
	return val;
}

std::ostream& operator<< (std::ostream& stream, const Value& value)
{
	switch(value.GetType())
	{
		case Value::NUMBER:
			stream << value.GetNumber();
			break;
		case Value::STRING:
			stream << value.GetString();
			break;
	}
	return stream;
}

Node::Node()
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

NodeHandle Graph::AddNode(NodeHandle parent, const Node* node)
{
	NodeHandle n = node->Clone();
	n->graph_ = this;

	if (parent == NULL_NODE_HND) {
		root_ = n;
	}
	else {
		parent->children_.push_back(n);
		n->parent_ = parent;
	}
	return n;
}

void Graph::ScheduleUpdate(NodeHandle node, const Value& value)
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
		Value initValue = curr->Init();
		// schedule an update for the intial value if there is one
		if (!initValue.IsUndefined()) {
			ScheduleUpdate(curr, initValue);
		}
		for (NodeList::iterator it = curr->children_.begin(); it != curr->children_.end(); it++) {
			nodes.push_back(*it);
		}
		nodes.pop_front();
	}
}

optional<unsigned int> Node::FindChildIndex(Node* n)
{
	optional<unsigned int> index;
	NodeList::iterator it;
	int i=0;
	for (it = children_.begin(); it != children_.end(); it++, i++) {
		if (*it == n) {
			index = i;
			break;
		}
	}
	return index;
}

bool Graph::Update(NodeHandle node, const Value& value)
{
	// lock the graph
	Lock lock(&mutex_);

	node->currentValue_ = value;

	if (node->parent_ == NULL) {
		return true;
	}

	Node* childNode = node;
	Node* updateNode = node->parent_;
	while (updateNode != NULL)
	{
		// only update node if all children are defined
		NodeList::iterator nodeIter;
		for (nodeIter = updateNode->children_.begin(); nodeIter != updateNode->children_.end(); nodeIter++) {
			if ((*nodeIter)->GetCurrentValue().IsUndefined()) {
				return true;
			}
		}

		optional<unsigned int> childIndex = updateNode->FindChildIndex(childNode);
		if (childIndex) 
		{
			Value v;
			v = updateNode->Evaluate(*childIndex);
			if (v.IsUndefined()) {
				break;
			}
			updateNode->currentValue_ = v;
			childNode = updateNode;
			updateNode = updateNode->parent_;
		}
	}

	cout << "graph output: " << childNode->GetCurrentValue() << endl;

	return true;
}

/*void* GraphSchedulerLoop(void* arg)
{
	while (1) {
		Value* value = ((GraphScheduler*)arg)->Update();
		cout << "New Value: " << value << endl;
	}
}*/
//

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
	Lock lock(&mutex_);
	scheduledTasks_.push_back(task);
}

void GraphScheduler::Update()
{
	Lock lock(&mutex_);
	if (scheduledTasks_.empty()) {
		return;
	}
	Task task = scheduledTasks_.front();
	scheduledTasks_.pop_front();

	lock.Release();

	task.graph->Update(task.node, task.value);
}

NumberNode::NumberNode(double n) : Node(), num_(n)
{
}

Value NumberNode::Init()
{
	Value val;
	val = num_;
	return val;
}

Value NumberNode::Evaluate(unsigned int childUpdated)
{
	return Value();
}

ArithmeticNode::ArithmeticNode(Type type) : Node(), type_(type)
{
}

Value ArithmeticNode::Init()
{
	return Value();
}

Value ArithmeticNode::Evaluate(unsigned int childUpdated)
{
	Value val;
	const Value& v1 = GetChild(0)->GetCurrentValue();
	const Value& v2 = GetChild(1)->GetCurrentValue();
	switch(type_)
	{
	case ADD:
		val = Add(v1, v2);
		break;
	case SUB:
		val = Subtract(v1, v2);
		break;
	case MUL:
		val = Multiply(v1, v2);
		break;
	case DIV:
		val = Divide(v1, v2);
		break;
	}
	return val;
}


