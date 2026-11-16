#pragma once
#include  <iostream> 

class node
{
public:
	node(std::string val);
	node();
	~node();
	friend class nodeList;
	const std::string& getData();
private:
	std::string data;
};






