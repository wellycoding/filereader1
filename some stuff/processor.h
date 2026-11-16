#pragma once
#include "nodelist.h"

class processor
{
public:
	processor();
	~processor();

	std::string retrieveDataAt(nodelist* datalist, int row, int col);
	std::string mergeData(nodelist* datalist, bool showConnector);
};

