#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
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
		string line_content;
		std::stringstream buffer;
		buffer<<inputFile.rdbuf();
		string file_contents = buffer.str();
		auto frequencyTable = calculateFrequencies(file_contents);
		map<char,string> result = huffmanCode(file_contents,frequencyTable);	

		ofstream outputFile(outFile);
		if(outputFile.is_open())
		{
			for(auto &p:result)
			{
				outputFile<<p.second;
			}
			outputFile.close();
		}
		else{
			cout<<"Error in creating/writing the file\n";
		}

		ofstream opfile(outFile+".map");
		if(opfile.is_open())
		{
			for (auto &p : result) {
        		if (p.first == '\n') opfile<< "\\n"; // special case for newline
        		else if (p.first == ' ') opfile << "[space]";
        		else opfile << p.first;
        		opfile << " " << p.second << "\n";
			}
		}
}
