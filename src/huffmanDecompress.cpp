#include "huffmanDecompress.h"
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

struct Node {
    char data;
    Node *left, *right;
    Node(char c = '\0') : data(c), left(nullptr), right(nullptr) {}
};

Node* buildHuffmanTree(const unordered_map<string, char>& codeMap) {
    Node* root = new Node();
    for (const auto& pair : codeMap) {
        const string& code = pair.first;
        char ch = pair.second;
        Node* current = root;
        for (char bit : code) {
            if (bit == '0') {
                if (!current->left) 
                current->left = new Node();
                current = current->left;
            } else { // bit == '1'
                if (!current->right)
                current->right = new Node();
                current = current->right;
            }
        }
        current->data = ch;
    }
    return root;
}

void deleteTree(Node* root) {
    if (root) {
        deleteTree(root->left);
        deleteTree(root->right);
        delete root;
    }
}

void decompress(const std::string& input,const std::string& output)
{
    ifstream inFile(input, ios::binary);
    if(!inFile.is_open())
    {
        cerr<<"Error opening the file "<<input<<endl;
        return;
    }
    ifstream mapFile(input+".map");
    if(!mapFile.is_open())
    {
        cerr<<"Error opening the map file "<<input<<".map"<<endl;
        inFile.close();
        return;
    }

    unordered_map<string, char> codeMap;
    string line;
    while(getline(mapFile,line))
    {
        if(line.length()>=3){
            string charPart = line.substr(0, line.find(' '));
            string code = line.substr(line.find(' ') + 1);
            
            char ch;
            if(charPart == "\\n") {
                ch = '\n';
            } else if(charPart == "[space]") {
                ch = ' ';
            } else {
                ch = charPart[0];
            }
            
            codeMap[code] = ch;
        }
    }
    mapFile.close();
    
    cout << "Loaded " << codeMap.size() << " codes from map file" << endl;

    Node* root = buildHuffmanTree(codeMap);

    stringstream buffer;
    buffer << inFile.rdbuf();
    string compressedData = buffer.str();
    inFile.close();
    
    cout << "Read " << compressedData.length() << " bytes of compressed data" << endl;

    //convert compressed data to bit string
    string bitString="";
    for(char byte:compressedData)
    {
        for(int i=7;i>=0;--i)
        {
            bitString += ((byte>>i)&1) ? '1' : '0';
        }
    }
    
    cout << "Converted to " << bitString.length() << " bits" << endl;

    // Decompression logic with null checks
    string decodedString = "";
    Node* current = root;
    for (char bit : bitString) {
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }
        
        // Check if current is null or if we reached a leaf node
        if (current == nullptr) {
            cerr << "Error: Invalid bit sequence encountered" << endl;
            deleteTree(root);
            return;
        }
        
        if (current->data != '\0') {
            decodedString += current->data;
            current = root;
        }
    }
    
    cout << "Decoded " << decodedString.length() << " characters" << endl;

    // Write the decompressed data to the output file
    ofstream outFile(output);
    if (!outFile.is_open()) {
        cerr << "Error opening the output file " << output << endl;
        deleteTree(root);
        return;
    }
    outFile << decodedString;
    outFile.close();
    
    cout << "Successfully decompressed to " << output << endl;

    // Clean up properly
    deleteTree(root);
}