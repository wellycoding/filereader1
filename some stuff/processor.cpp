#include "processor.h"
#include "nodelist.h"

processor::processor()
{
}
processor::~processor()
{
}

std::string processor::retrieveDataAt(nodelist* datalist, int row, int col) {
    node* n = datalist->dataList[row][col];
    if (n != nullptr) { return n->getData(); }
    return "";
}

std::string processor::mergeData(nodelist* datalist, bool showConnector) {
    std::string tempList = " ";
    for (int c = 0; c < datalist->MAX_SIZE; c++) {
        if (datalist->getAllData()[c] == '_') {
            datalist->getAllData()[c] = 0;
            if (showConnector) { tempList += datalist->getAllData(); }
        }
        else {
            tempList += datalist->getAllData()[c];
            continue;
        }
    }
    return tempList;
}