#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <iostream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <fstream>
#include "MyRecord.h"

using namespace std;

class GPSRecord
{
public:
    int Oid;   //object id
    int yy,mm,dd,h,m,s;    //date and time
    double lo, la;   //longitude and latitude
};

class RecordManager
{
private:
    bool                   isDef;    //if the recordmanager is av
    bool                   isDir;   //if the input is directory, it is true, or false;
    char                   *name;     // the name of input;
    char                   *extension;
    MyRecord               temprecord;
    list<char *>           *files;   //if input is a directory, and store all the files in this directory;
    list<char *>::iterator fileit;
    ifstream               curfile;

public:
    RecordManager()
    {
        isDef = false;
        isDir = false;
        extension = NULL;
        name = NULL;
        files = NULL;
    }

    ~RecordManager()
    {
        if(name){
            delete name;
        }
        if(extension){
            delete extension;
        }
        if(curfile.is_open()){
            curfile.close();
        }
        freeFileList(files);
    }

    bool setName(const char *str)
    {
        if(name){
            delete name;
        }
        name =  new char[strlen(str)+1];
        if(name == NULL){
            return false;
        }
        strcpy(name, str);
        return true;
    }

    bool setExtension(char *str)
    {
        if(extension){
            delete extension;
        }
        extension =  new char[strlen(str)+1];
        if(extension == NULL){
            return false;
        }
        strcpy(extension, str);
        return true;
    }

    char *getName(){ return name;}

    bool isDirectory(){ return isDir;}

    bool isDefined(){ return isDef;}

    void resetDefined(){ isDef = false;}

    bool Init()
    {
        files = getAllFileList(name, extension);
        if(NULL == files){
            cout<<"ERROR: There is not files in "<<name<<"!"<<endl;
            return false;
        }
        if(files->size() <= 0){
            cout<<"ERROR: There is not record in "<<name<<"!"<<endl;
            return false;
        }
        fileit = files->begin();
        if(! openNextFile()){
            // ERROR: can not open the files!
            return false;
        }
        return true;
    }

    bool openNextFile()
    {
        if(fileit == files->end()){
            cout<<"Warning: There is no file to open!"<<endl;
            return false;
        }
        if(curfile.is_open()){
            curfile.close();
        }
        cout<<"Warning: Trying to open file "<<*fileit<<endl;
        curfile.open(*fileit);
        fileit++;
        while((!curfile.is_open()) && fileit != files->end()){
            cout<<"Warning: Trying to open file "<<*fileit<<endl;
            curfile.open(*fileit);
            fileit++;
        }
        if((!curfile.is_open()) && (fileit == files->end())){
            cout<<"Warning: There is no file to open!"<<endl;
            return false;
        }
        return true;
    }

    bool getNextRecord(GPSRecord &gpsr)
    {
        char buf[1024];
        if(! curfile.is_open() || curfile.eof()){
            if(! openNextFile()){
                return false;
            }
        }
        curfile.getline(buf, 1024);
        temprecord.SetRstr(buf);
        while(-1 == temprecord.FreshData()){
            //check whether the file is to end~
            if(curfile.eof()){
                if(! openNextFile()){
                    return false;
                }
            }
            curfile.getline(buf, 1024);
            temprecord.SetRstr(buf);
        }
        gpsr.Oid = temprecord.id;
        gpsr.yy = temprecord.yy;
        gpsr.mm = temprecord.mm;
        gpsr.dd = temprecord.dd;
        gpsr.h = temprecord.h;
        gpsr.m = temprecord.m;
        gpsr.s = temprecord.s;
        gpsr.lo = temprecord.X;
        gpsr.la = temprecord.Y;

        return true;
    }
};




#endif
