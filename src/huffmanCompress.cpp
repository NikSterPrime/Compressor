#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include "../include/huffmanCompress.h"
#include "../include/utils.h"
using namespace std;

class Node {
	public:
		int data;
		char c;
		Node *left, *right;
		Node(char ch,int x)
		{
			c = ch;
			data = x;
			left = nullptr;
			right = nullptr;
		}
};

class Compare{
	public:
		bool operator()(Node* a,Node* b){
			return a->data > b->data;
		}
};

void preorder(Node* root, map<char,string> &ans,string curr)
{
	if(root == nullptr) return;
	
	if(root->left == nullptr && root->right== nullptr)
	{
		ans[root->c] = curr;
		return;
	}
	preorder(root->left,ans,curr+"0");
	preorder(root->right,ans,curr+"1");
}

map<char,string> huffmanCode(string contents,map<char, int> freq)
{
	int n = contents.length();
	priority_queue<Node*, vector<Node*>,Compare> pq;

	for(auto &i:freq)
	{
		Node* tmp = new Node(i.first,i.second);
		pq.push(tmp);
	}

	while(pq.size()>=2)
	{
		Node* l = pq.top();
		pq.pop();

		Node* r = pq.top();
		pq.pop();

		Node* newNode = new Node('\0',l->data+r->data);
		newNode->left = l;
		newNode->right = r;
		pq.push(newNode);
	}
	Node* root = pq.top();
	map<char,string> ans;
	preorder(root,ans,"");
	return ans;
}

void compress(const string& inFile,const string& outFile)
{
		ifstream inputFile(inFile);
		if(!inputFile.is_open())
		{
			cerr<<"Error opening the file:"<<inFile<<endl;
			return;
		}
		std::stringstream buffer;
		buffer<<inputFile.rdbuf();
		string file_contents = buffer.str();
		inputFile.close();
		
		auto frequencyTable = calculateFrequencies(file_contents);
		map<char,string> result = huffmanCode(file_contents,frequencyTable);	

		// Build the compressed bit string
		string compressedBits = "";
		for(char c : file_contents) {
			compressedBits += result[c];
		}
		
		cout << "Original size: " << file_contents.length() << " bytes" << endl;
		cout << "Compressed bits: " << compressedBits.length() << " bits" << endl;
		
		// Pad the bit string to make it divisible by 8
		while(compressedBits.length() % 8 != 0) {
			compressedBits += "0";
		}
		
		// Convert bit string to bytes and write to file
		ofstream outputFile(outFile, ios::binary);
		if(outputFile.is_open())
		{
			for(size_t i = 0; i < compressedBits.length(); i += 8) {
				string byte = compressedBits.substr(i, 8);
				char byteValue = 0;
				for(int j = 0; j < 8; j++) {
					if(byte[j] == '1') {
						byteValue |= (1 << (7 - j));
					}
				}
				outputFile.write(&byteValue, 1);
			}
			outputFile.close();
			cout << "Compressed to " << compressedBits.length() / 8 << " bytes" << endl;
		}
		else{
			cout<<"Error in creating/writing the file\n";
			return;
		}

		// Write map file with consistent format
		ofstream opfile(outFile+".map");
		if(opfile.is_open())
		{
			for (auto &p : result) {
        		if (p.first == '\n') opfile<< "\\n"; // special case for newline
        		else if (p.first == ' ') opfile << "[space]";
        		else opfile << p.first;
        		opfile << " " << p.second << "\n";
			}
			opfile.close();
			cout << "Map file created with " << result.size() << " entries" << endl;
		}
		else {
			cout << "Error creating map file\n";
		}
}
