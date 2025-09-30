#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../include/huffmanCompress.h"
#include "../include/utils.h"
using namespace std;

class Node {
	public:
		int data;
		Node *left, *right;
		Node(int x)
		{
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

void preorder(Node* root, vector<string> &ans,string curr)
{
	if(root == nullptr) return;
	
	if(root->left == nullptr && root->right== nullptr)
	{
		ans.push_back(curr);
		return;
	}
	preorder(root->left,ans,curr+"0");
	preorder(root->right,ans,curr+"1");
}

vector<string> huffmanCode(string contents,unordered_map<char, int> freq)
{
	int n = contents.length();
	priority_queue<Node*, vector<Node*>,Compare> pq;

	for(auto &i:freq)
	{
		Node* tmp = new Node(i.second);
		pq.push(tmp);
	}

	while(pq.size()>=2)
	{
		Node* l = pq.top();
		pq.pop();

		Node* r = pq.top();
		pq.pop();

		Node* newNode = new Node(l->data+r->data);
		newNode->left = l;
		newNode->right = r;
		pq.push(newNode);
	}
	Node* root = pq.top();
	vector<string> ans;
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
		vector<string> result = huffmanCode(file_contents,frequencyTable);	

		ofstream outputFile(outFile);
		if(outputFile.is_open())
		{
			for(int i=0;i<result.size();i++)
			{
				outputFile<<result[i];
			}
			outputFile.close();
		}
		else{
			cout<<"Error in creating/writing the file.\n";
		}
}

