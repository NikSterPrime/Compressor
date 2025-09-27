#include <iostream>
#include <string>
#include "../include/huffmanCompress.h"
using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        cout<<"Usage: \n";
        cout<<" compress <input_file> <output_file>\n";
        cout<<" decompress <input_file> <output_file>\n";
        return 1;
    }

    string command = argv[1];

    if(command == "compress")
    {
        if(argc < 4)
        {
            cout<<"Missing input/output files";
            return 1;
        }
        string inputFile = argv[2];
        string outputFile = argv[3];


        compress(inputFile,outputFile);
    }

    else if(command == "decompress")
    {
        if(argc < 4)
        {
            cout<<"Missing input/output files";
            return 1;
        }
        string inputFile = argv[2];
        string outputFile = argv[3];


    }

    else
    {
        cout<<"Unknown command: "<<command<<"\n";
        cout<<"Use compress or decompress .\n";
    }

    return 0;
}