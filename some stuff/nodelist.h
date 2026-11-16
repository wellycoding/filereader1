#pragma once
#include "node.h"
#include <vector>
#include <string>

class nodelist
{
public:
	nodelist();
	~nodelist();

	const static int MAX_SIZE = 10;
	node* dataList[MAX_SIZE][MAX_SIZE];
	void sporaticFillup();
	void multisporaticFillup(int repX, int repY);
	int randint(int min, int max);
	char randChar(int min, int max, bool lowercase);
	void readData();
	std::string getData(int row, int col);
	std::string getAllData();
	std::vector<std::string> multiDataList;
};

