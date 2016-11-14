#include <iostream>
#include <fstream>
using namespace std;

const int FILENAMESIZE = 64;
const int BUFFERSIZE = 1024;

class FileRecord
{
    public:
        char     *filename;
        char     *buf;
        ifstream input;
        bool     isdefined;

        FileRecord()
        {
            filename = new char[FILENAMESIZE];
            buf = new char[BUFFERSIZE];
        }

        ~FileRecord()
        {
            delete filename;
            delete buf;
        }

        bool open(char *filename)
        {
            input.open(filename, ifstream::in);
            if(input.is_open())
            {
                isdefined = true;
                return true;
            }
            cout<<"file open failed!"<<endl;
            isdefined = false;
            return false;
        }

        bool eof()
        {
            return input.eof();
        }
        
};
