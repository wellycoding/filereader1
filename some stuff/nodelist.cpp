#include "nodelist.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cmath> // Add this for floor and round

nodelist::nodelist() {
    // Initialize multiDataList if needed (no explicit initialization required for std::vector)
}

nodelist::~nodelist() {
    // Destructor logic if needed
}

void nodelist::sporaticFillup()
{
	srand(static_cast<unsigned int>(time(0)));
	char delimiter = '_';
	for (int iter = 0; iter < MAX_SIZE; iter++) {
		for (int jter = 0; jter < MAX_SIZE; jter++) {
			std::string firstVal = std::to_string(randint(1, 9));
			char secondVal = randChar(0, 24, randint(0, 2));

			dataList[jter][iter] = new node(firstVal + secondVal + delimiter);
			delimiter = '_';
		}
		delimiter = ' ';
	}	
}

void nodelist::multisporaticFillup(int repX, int repY)
{
    bool wholenumber = repX == floor(repX) || repY == floor(repY);
    if (!wholenumber) std::cout << "Decimals are rounded to their floor values." << std::endl;

    srand(static_cast<unsigned int>(time(0)));
    char delimiter = '_';
    int sizeX = round(MAX_SIZE * repX);
    int sizeY = round(MAX_SIZE * repY);

    multiDataList.clear();

    for (int iter = 0; iter < round(MAX_SIZE * sizeY); iter++) {
        for (int jter = 0; jter < round(MAX_SIZE * sizeX); jter++) {
            std::string firstVal = std::to_string(randint(1, 9));
            char secondVal = randChar(0, 24, randint(0, 2));

            dataList[jter][iter] = new node(firstVal + secondVal + delimiter);
            multiDataList.push_back(dataList[jter][iter]->getData());
            delimiter = '_';
        }
        delimiter = ' ';
    }
}


int nodelist::randint(int min, int max) {
	return min + rand() % (max - min + 1);
}

char nodelist::randChar(int min, int max, bool lowercase) {
	int ASCIIconstant;
	if (!lowercase) ASCIIconstant = 97;
	else ASCIIconstant = 65;
	int value = ASCIIconstant + randint(min, max);
	char newVal = static_cast<char>(value);
	return newVal;
}

void nodelist::readData() {
	for (int iter = 0; iter < MAX_SIZE; iter++) {
		for (int jter = 0; jter < MAX_SIZE; jter++) {
			std::string ptrData = dataList[jter][iter]->getData();
			std::cout << ptrData;
		}
		std::cout << std::endl;
	}
}

std::string nodelist::getData(int row, int col)
{
	std::string tempData = dataList[row][col]->getData();
	return tempData;
}

std::string nodelist::getAllData() {
	std::string allData= " ";
	for (int iter = 0; iter < MAX_SIZE; iter++) {
		for (int jter = 0; jter < MAX_SIZE; jter++) {
			allData += dataList[jter][iter]->getData();
		}
	}
	return allData;
}
