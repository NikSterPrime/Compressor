#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../include/huffmanCompress.h"
#include "../include/utils.h"
using namespace std;

void compress(const string& inFile,const string& outFile)
{
		ifstream inputFile(inFile);
		string line_content;
		string file_contents;
		if(!inputFile.is_open())
		{
			cerr<<"Error Opening the file !"<<endl;
			exit(0);
		}
		while(getline(inputFile,line_content))
		{
			file_contents+=line_content;
		}
		auto frequencyTable = calculateFrequencies(file_contents);
		for(const auto& i:frequencyTable)
		{
			cout<<i.first<<" "<<i.second;
		}
}

