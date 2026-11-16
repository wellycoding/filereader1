#include "node.h"
node::node(std::string val)
{
	data = val;
}

node::~node() {
	data = -1;
}

const std::string& node::getData() { return data; }
