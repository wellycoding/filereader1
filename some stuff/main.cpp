#define NOMINMAX

#include <fstream>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime> 
#include <direct.h>
#include <vector>
#include <Windows.h>
#include <chrono>
//#include <zip.h>

#include "nodelist.h"
#include "processor.h"

//A = ANSI (uses char)
//W = Wide(uses wchar_t, Unicode)

void static skipLine(void) {
	std::cout << std::endl;
}

bool static fileExists(const std::string& fileName) {
	std::ifstream f(fileName);
	return f.good();
}

// Read plainly, no ordering or patternization 
void static BruteforceReading(const std::string filePath) {
	std::ifstream file(filePath);
	if (!fileExists(filePath)) {
		std::cerr << "Unable to find file\n";
		if (!file.is_open()) {
			std::cerr << "Error opening file\n";
		}
	}

	std::string line;
	while (std::getline(file, line)) {
		std::cout << line << "\n";
	}

	if (line.empty()) {
		std::cout << "File is empty." << std::endl;
	}
}

// Read file starting from the offset until fulfilling byte count
void static ReadFileFrom(const std::string filePath, int chunkSize, int startOffset) {
	if (chunkSize <= 0) return;
	skipLine();
	std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: could not open file." << std::endl;
		return;
	}
	inFile.seekg(startOffset);
	std::vector<char> buffer(static_cast<size_t>(chunkSize) + 1);
	if (inFile.read(buffer.data(), static_cast<std::streamsize>(chunkSize))) {
		if (inFile.get() == ' ') { 
			inFile.ignore(1);
		}
		int nextCharacter = inFile.peek();
		if (nextCharacter == EOF) return;
		if (static_cast<char>(nextCharacter) == '_') {
			inFile.ignore(1);
		}
		buffer[chunkSize] = '\0'; // null terminator
		std::cout << "Chunk: " << buffer.data() << std::endl;
	}
}

// Read file from the beginning but ignores X bytes of info per Y bytes
void static ReadFileIgnoring(const std::string filePath, int ignByteCount, int numByte) {
    if (numByte <= 0) return; 
	skipLine();
    std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile) {
		std::cerr << "Error: could not open file." << std::endl;
		return;
	}
    std::vector<char> buffer(static_cast<size_t>(numByte));
	int numChunks = 0;
	while (inFile.read(buffer.data(), static_cast<std::streamsize>(numByte - 1))) {
		buffer[static_cast<std::vector<char, std::allocator<char>>::size_type>(numByte) - 1] = '\0';
		int nextCharacter = inFile.peek();
		if (nextCharacter == EOF) break;
		if (static_cast<char>(nextCharacter) == ' ') return; 		
		if (static_cast<char>(nextCharacter) == '_') inFile.ignore(1);
		numChunks++;
		std::cout << "Info Chunk "
			<< std::setw(2) << std::right << numChunks << ": "
			<< std::setw(12) << std::right << buffer.data()
			<< std::endl;
		if (!std::cout) {
			std::cout << "Unable to get chunk:" << buffer.data() << std::endl;
		}
		inFile.ignore(static_cast<std::streamsize>(ignByteCount));
	}
}

bool static directoryExists(const std::string& path)
{
	DWORD attrib = GetFileAttributesA(path.c_str());

	return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

void static ReadByChunk(const std::string fileName, int chunkNum) {
	std::ifstream inFile(fileName);
	if (!inFile) {
		std::cerr << "Error: could not open file." << std::endl;
		return;
	}
	skipLine();
	char ch;
	int currentChunk = -1;

	while (inFile.get(ch)) { 
		if (ch == ' ') {
			currentChunk++;
			if (currentChunk > chunkNum) {
				return;
			}
		};

		if (currentChunk == chunkNum) std::cout << ch;
	}
}

bool static copyFile(const std::string& sourcePath, const std::string& destPath) {
	std::ifstream src(sourcePath, std::ios::binary);
	if (!src) {
		std::cerr << "Error: Cannot open source file '" << sourcePath << "'\n";
		return false;
	}
	std::ofstream dst(destPath, std::ios::binary);
	if (!dst) {
		std::cerr << "Error: Cannot create destination file '" << destPath << "'\n";
		return false;
	}
	const size_t bufferSize = 8192; // 8KB transfer
	char buffer[bufferSize];
	while (src.read(buffer, bufferSize)) {
		dst.write(buffer, src.gcount());
	}

	dst.write(buffer, src.gcount());

	if (!src.eof()) {
		std::cerr << "Error: Failed while reading source file.\n";
		return false;
	}
	if (!dst) {
		std::cerr << "Error: Failed while writing destination file.\n";
		return false;
	}

	return true;
}

void static byteFormat(size_t byteSize, size_t checkByteSize, const std::string& notation, int decimals)
{
	double value = static_cast<double>(byteSize) / checkByteSize;
	if (value < 1) return;
	std::cout << std::fixed << std::setprecision(decimals) << value << notation << '\n';
}

size_t static notateLongForm(unsigned int baseNum, int numZero) {
	std::string numZeros(numZero, '0');
	if (baseNum > 9) {
		std::cout << "Base number cannot exceed 9" << std::endl;
		return 0;
	}
	std::string longNumber = std::to_string(baseNum) + numZeros;
	try {
		return static_cast<size_t>(std::stoull(longNumber));
	}
	catch (const std::invalid_argument&) {
		std::cerr << "Cannot convert letters to int type." << std::endl;
		return 0;
	}
	catch (const std::out_of_range&) {
		std::cerr << "Cannot convert data out of range." << std::endl;
		return 0;
	}
}

// The processFile() function handles the file - reading and processing operation.
// The file is read in 8 KB chunks using the buffer array, so the entire file does not need to be loaded into memory at once.
// The ptr pointer points to the beginning of the buffer.
// The expression* (ptr + i) accesses each byte through pointer arithmetic and is used to check for newline characters.
// The program maintains totalBytes and totalLines to count the amount of data processed.
// chrono::steady_clock records the start and end times, and their difference gives the elapsed time for reading and processing the file.
void static processFile(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		std::cerr << "Unable to open file." << std::endl;
		skipLine();
		return;
	}
	const size_t BUFFER_SIZE = 8192; // 8KB read
	char buffer[BUFFER_SIZE];
	size_t totalBytes = 0;
	size_t totalLines = 0;
	auto start = std::chrono::steady_clock::now();
	while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
		size_t bytesRead = (size_t)(file.gcount());
		totalBytes += bytesRead;
		char* ptr = buffer;
		for (size_t i = 0; i < bytesRead; i++) {
			if (*(ptr + i) == '\n') {
				totalLines++;
			}
		}
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	file.close();
	const int numNotations = 3;
	size_t byteSizes[numNotations] = { 0 };
	for (int i = 0; i < numNotations; ++i) {
		byteSizes[i] = notateLongForm(1, (i + 1) * 3);
	}
	const char notations[numNotations] = { 'K', 'M', 'B' };
	for (int i = 0; i < numNotations; ++i) {
	    byteFormat(totalBytes, static_cast<int>(byteSizes[i]), std::string(1, notations[i]), 2);
	}
	
	std::cout << "Total Bytes: " << totalBytes << std::endl;
	std::cout << "Total Lines: " << totalLines << std::endl;
	std::cout << "Time Taken: " << elapsed.count() << " seconds" << std::endl;
	skipLine();
}

bool static checkFileExtension(const std::string& filename, const std::string& extension) {
	size_t dotPos = filename.find('.');
	if (dotPos == std::string::npos) return false;
	return filename.substr(dotPos + 1) == extension;
}

int main() {
	srand(static_cast<unsigned int>(time(0)));
	const char* FileName = "wellyData.txt";
	const char* DirName = "Stuff";
	bool exit = true;
	
	std::string newFile;
	std::string extension;
	int offset, size;
	int ignore, spliceSize;
	int chunkNo;
	int numchoice;
	int deletionChoice; 
	char userInput;
	int editChoice;
	std::pair<int, int> dataCoord;
	nodelist DATALIST;
	processor processor;
	const char* currFile;
	std::vector<std::string> filelist;
	WIN32_FIND_DATAA fileData;
	int numFiles = 0;
	std::string editInput;
	std::string line; // each line
	std::vector<std::string> lines; // all line
	std::string backupLoc = std::string(DirName) + "/BackUp";
	int backupIndex = 1;
	const std::string filepathheader = std::string(DirName) + "/";
	std::string filepath = filepathheader + std::string(FileName);
	std::string replacedData, newData;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int addIndex = -1;
	std::string allLines = ""; // every line combined from the lines vector

	while (true) {
		numFiles = 0;
		filelist.clear();
		std::string findCommand = std::string(DirName) + "\\*.*";
		HANDLE hFind = FindFirstFileA(findCommand.c_str(), &fileData);

		if (exit) {
			// WIN32_FIND_DATAA Windows structure that stores information of a file:
			// filename, size, creation date, attributes
			if (hFind == INVALID_HANDLE_VALUE) {
				std::cout << "Folder not found" << std::endl;
			}
			else {
				std::cout << "Files under " + std::string(DirName) + ": " << std::endl;
				do {
					if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // Checks if the result is a folder
						numFiles++;
						std::cout << numFiles << ". " << fileData.cFileName << std::endl;
						std::string tempFileName = fileData.cFileName;
						filelist.push_back(tempFileName);
					}
				} while (FindNextFileA(hFind, &fileData));
			}
			int select;
			std::cin >> select;
			if (select < 1) select = 1;
			if (!filelist.empty() && select <= static_cast<int>(filelist.size())) {
				currFile = filelist[static_cast<std::vector<std::string>::size_type>(select) - 1].c_str();
			}
			else {
				currFile = filelist[0].c_str();
			}

			std::cout << currFile << " selected." << std::endl;
			FileName = currFile;
			filepath = filepathheader + std::string(FileName);
			lines.clear();
		}
		// From this point onwards FileName uses the current file chosen from above
		if (checkFileExtension(FileName, "zip")) {
			std::cout << "Zip file detected" << std::endl;
			
		}
		exit = false;
		if (exit) continue;
		std::ifstream inFile(FileName);
		std::ofstream outFile(FileName);
		skipLine();
		std::cout << "---------------------------------------------------------------" << std::endl;
		std::cout << "Press A to add file" << std::endl;
		std::cout << "Press C to clear contents" << std::endl;
		std::cout << "Press D to delete files" << std::endl;
		std::cout << "Press E to exit" << std::endl;
		std::cout << "Press P to process the file" << std::endl;
		std::cout << "Press R to randomize file contents with X by X sets of data: " << std::endl;
		std::cout << "Press Z to edit file contents" << std::endl;
		std::cout << "Press 0 to read file contents" << std::endl;
		std::cout << "---------------------------------------------------------------" << std::endl;
		skipLine();
		skipLine();
		skipLine();
		userInput = 0;

		DATALIST.sporaticFillup();

		// _mkdir (make directory) creates directory (folders)
		if (!directoryExists(DirName)) {
			if (_mkdir(DirName) != 0) {
				std::perror("Unable to create Directory");
			}
			std::string moveCommand = std::string("move *.txt ") + DirName;
			system(moveCommand.c_str());
		}

		std::cin >> userInput;
		try {
			std::ifstream inFile(FileName);
		}
		catch (...) {
			std::cerr << "Unable to get file at Line " << __LINE__ << std::endl;
		}


		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid input!\n";
			continue;
		}

		int status;
		std::string newFileName;
		skipLine();
		switch (std::toupper(userInput)) {
		case 'C':
			status = remove(FileName);
			if (status != 0) { perror("Unable to delete file! Error no." + __LINE__); }
			break;

		case '0': 
			std::cout << "Choose file reading method: " << std::endl;
			std::cout << "1. Bruteforce reading." << std::endl;
			std::cout << "2. Offset reading." << std::endl;
			std::cout << "3. Splice reading." << std::endl;
			std::cout << "4. Chunk reading." << std::endl;
			std::cout << "5. Selection reading." << std::endl;
			std::cin >> numchoice;
			skipLine();
			switch (numchoice) {
				case 1: BruteforceReading(filepath); break;
				case 2: 
					std::cout << "How many bytes do you want to skip?"; std::cin >> offset; skipLine();
					std::cout << "How many bytes do you want to read?"; std::cin >> size; skipLine();
					if (offset < 0) offset = 0;
					if (size < 0) size = 0;
					ReadFileFrom(filepath, size, offset);
					break;
				case 3: 
					std::cout << "How many bytes do you want to ignore?"; std::cin >> ignore; skipLine();
					std::cout << "How many bytes do you want to read?"; std::cin >> spliceSize; skipLine();
					if (ignore < 0) ignore = 0;
					if (spliceSize < 0) spliceSize = 0;
					ReadFileIgnoring(filepath, ignore, spliceSize);
					break;
				case 4: 
					std::cout << "Which Chunk do you want to reead?"; std::cin >> chunkNo; skipLine();
					if (chunkNo < 0) chunkNo = 1;
					ReadByChunk(filepath, chunkNo);
					break;
				case 5:
					std::cout << "Data Coord X: ";
					std::cin >> dataCoord.first;
					std::cout << "Coord Y: ";
					std::cin >> dataCoord.second;
					if (dataCoord.first > 10) dataCoord.first = 10;
					if (dataCoord.second > 10) dataCoord.second = 10;
					if (dataCoord.first < 0) dataCoord.first = 0;
					if (dataCoord.second < 0) dataCoord.second = 0;
 					std::cout << DATALIST.getData(dataCoord.first, dataCoord.second) << std::endl;
					break;
				default:
					std::cout << "Invalid option." << std::endl;
					break;
					
			}
			break;
		case 'P':
			processFile(filepath);
			break;
		case 'R':
			//{
			//	int x, y;
			//	std::cout << "X amounr";
			//	std::cin >> x;
			//	std::cout << "Y amount";
			//	std::cin >> y;
			//	skipLine();
			//	DATALIST.multisporaticFillup(x, y);
			//	std::ofstream outFile(FileName);
			//}
			for (int iter = 0; iter < DATALIST.MAX_SIZE; iter++) {
				for (int jter = 0; jter < DATALIST.MAX_SIZE; jter++) {
					std::string tempdata = DATALIST.getData(jter, iter);
					outFile << tempdata;
				}
				outFile << std::endl;
			}
			outFile.close();
			
	
			break;
		case 'A':
			std::cout << "Name of file: ";
			std::cin >> newFile;
			//std::cout << "Name of extension: ";
			//std::cin >> extension;
			newFileName = newFile + ".txt";
			//newFileName = newFile + "." + extension;
			if (!fileExists(newFileName)) {
				std::ofstream outFile(filepathheader + newFileName);
				std::cout << "Success." << std::endl;
			}
			else {
				std::cout << "Failed to create file: " + newFileName + " already exists!" << std::endl;
			}
			skipLine();
			break;
		case 'E':
			exit = true;
			break;
		case 'D':
			std::cout << "Choose a file to delete: " << std::endl;
			do {
				if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // Checks if the result is a folder
					numFiles++;
					std::cout << numFiles << ". " << fileData.cFileName << std::endl;
					std::string tempFileName = fileData.cFileName;
					filelist.push_back(tempFileName);
				}
			} while (FindNextFileA(hFind, &fileData));
			skipLine();
			std::cin >> deletionChoice;
			if (deletionChoice > 0 && deletionChoice < numFiles) {
				std::string currFilePath = filelist[static_cast<std::vector<std::string>::size_type>(deletionChoice) - 1].c_str();
				copyFile(filepathheader + currFilePath, backupLoc);
				if (std::remove((filepathheader + currFilePath).c_str()) != 0) {
					perror("Error deleting file");
				}
				else {
					std::cout << "File successfully deleted\n";
				}
			}
			std::cout << "Invalid file." << std::endl;
			break;
		case 'Z':
			if (inFile.is_open()) {
				std::cout << "Successfully opened " << FileName << std::endl;
				while (std::getline(inFile, line)) {
					lines.push_back(line);
				}
				inFile.close();
				skipLine();
				std::cout << "1. Truncate all data " << std::endl;
				std::cout << "2. Replace x with y  " << std::endl;
				std::cout << "3. Add data at character " << std::endl;
				std::cout << "4. Restore backup " << std::endl;
				skipLine();
				std::cout << "Original file contents:\n";
				for (const auto& l : lines) {
					std::cout << l << "\n";
				}
				skipLine();
				std::cin >> editChoice;
				switch (editChoice) {
				case 1:
					if (!directoryExists(DirName)) {
						std::cerr << "Unable to create backup file due to missing parent directory: " << std::string(DirName) << std::endl;
						skipLine();
						break;
					}

					if (!directoryExists(backupLoc)) {
					    if (_mkdir(backupLoc.c_str()) != 0) {
					        std::perror(("Unable to create backup directory: " + backupLoc).c_str());
					    }
					}
					while (fileExists(backupLoc + "/backup" + std::to_string(backupIndex))) { backupIndex++; }
					{
						std::ifstream srcFile(filepath, std::ios::binary);
						if (srcFile) {
						    std::string content((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());
						    std::ofstream backupFile(backupLoc + "/backup" + std::to_string(backupIndex), std::ios::binary);
						    backupFile << content;
						}
					}

					outFile.close();
					outFile.open(filepath, std::ios::trunc);
					if (!outFile.is_open()) {
						std::cerr << "Unable to open " << FileName << std::endl;
					}
					outFile.close();
					std::cout << "\nFile truncated." << std::endl;
					break;
				case 2:
					std::cout << "Write in replaced data: ";
					std::cin >> replacedData;
					skipLine();
					std::cout << "Write in new data: ";
					std::cin >> newData;
					skipLine();
					while (std::getline(inFile, line)) {
						if (line.find(replacedData) != std::string::npos) {
							line.replace(line.find(replacedData), line.find(replacedData), newData);
						}
						else {
							std::cout << "Unable to find data." << std::endl;
							skipLine();
							outFile << newData;
						}
					}
					line.clear();
					outFile.close();
					inFile.close();
					break;
				case 3:
					std::cout << "Char index: ";
					std::cin >> addIndex;
					skipLine();
					while (std::getline(inFile, line)) {
						std::cout << line << std::endl;
						if (!line.empty()) {
							lines.push_back(line);
						}
					}
					for (int v = 0; v < lines.size(); v++) { allLines += lines[v]; }
					for (int p = 0; p < allLines.size(); p++) {
						std::cout << allLines[p];
						if (allLines[p] == addIndex) {
							SetConsoleTextAttribute(hConsole, 10);
						}
						else {
							SetConsoleTextAttribute(hConsole, 7);
						}
					}
					break;
				case 4:
					break;
				default:
					std::cout << "Exiting..." << std::endl;
					break;
				}
			}
			else {
				std::cout << "Unable to open file." << std::endl;
				skipLine();
			}

			break;
		default:
			std::cout << "Invalid input." << std::endl;
			break;
		}
	}

	return 0;
}

std::string static replaceAll(std::string str, const std::string& from, const std::string& to) {
	if (from.empty()) return str;
	size_t startPos = 0;
	while ((startPos = str.find(from, startPos)) != std::string::npos) {
		str.replace(startPos, from.length(), to);
		startPos += to.length();
	}
	return str;
}