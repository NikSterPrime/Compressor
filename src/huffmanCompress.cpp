#include <iostream>
#include <vector>
#include "../include/huffmanCompress.h"
#include <unordered_map>
#include "../include/utils.h"
using namespace std;

void compress(const string& inputFile,const string& outputFile)
{

		auto frequencyTable = calculateFrequencies(inputFile);
		for(const auto& i:frequencyTable)
		{
			cout<<i.first<<" "<<i.second;
		}
}

