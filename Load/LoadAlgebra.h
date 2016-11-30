#ifndef LOADALGEBRA_H
#define LOADALGEBRA_H

#include <iostream>
#include <fstream>
#include <DateTime.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <list>
#include <queue>
#include <vector>
#include <math.h>
#include "SpatialAlgebra.h"
using namespace std;

////////////////////////////////////////////
// file operator
//
const int PATH_MAX_SIZE = 128;

bool strContain(const char *source, const char *dest)
{
    int sptr=0, dptr=0;
    if(source == NULL){
        return false;
    }
    if(dest == NULL){
        return true;
    }
    while('\0' != source[sptr] && '\r' != source[sptr]){
        if(source[sptr++] == dest[0]){
            dptr = 1;
            while('\0' != dest[dptr] && '\r' != dest[dptr] && source[sptr] == dest[dptr]){
                sptr++;
                dptr++;
            }
            if('\0' == dest[dptr] || '\r' == dest[dptr]){
                //cout<<"sptr: "<<sptr<<endl;
                //cout<<"dptr: "<<dptr<<endl;
                return true;
            }
        }
    }
    return false;
}
//
// Check the directory, if the directory is exist, return 1, else create
// directory and return 1; if create failed, return -1;
//
int checkDir(char *dirpath)
{
    DIR *dir;
    if(dirpath == NULL)
    {
        cout<<"dir path is null!"<<endl;
        return -1;
    }
    if((dir=opendir(dirpath)) == NULL)
    {
        if(0 == mkdir(dirpath, S_IRWXU | S_IRWXG))
        {
            cout<<"dir result create successed!"<<endl;
        }
        else
        {
            cout<<"dir result create failed!"<<endl;
            return -1;
        }
    }
    else
    {
        closedir(dir);
        cout<<"dir is already exist!"<<endl;
    }
    return 1;
}

//
// Check the filename whether is a file or directory
//
bool IsFile(char *filename)
{
    if(filename == NULL){
        return false;
    }
    if(access(filename, F_OK) == 0){
        return true;
    }
    return false;
}

//
// Check the file name whether is a directory
//
bool IsDir(char *filename)
{
    DIR *dir;
    if(filename == NULL){
        return false;
    }
    if((dir = opendir(filename)) != NULL){
        closedir(dir);
        return true;
    }
    return false;
}


//
// Get the absolute path of current directory
//

char *getAbsolutePath()
{
    char tmp[PATH_MAX_SIZE], *result;
    int i,cnt;
    cnt = readlink("/proc/self/exe", tmp, PATH_MAX_SIZE);
    if(cnt < 0 || cnt >= PATH_MAX_SIZE)
    {
        cout<<"Error in getAbsolutePath: readlink\n"<<endl;
        return NULL;
    }
    i = cnt;
    for(i = cnt; i >= 0; i --)
    {
        if(tmp[i] == '/')
        {
            tmp[i+1] = '\0';
            result = new char[sizeof(char)*(i+2)];
            strcpy(result,tmp);
            return result;
        }
    }
    return NULL;
}

//
// Get the absolute path of directory basePath
//
char *getAbsolutePath(char *basePath)
{
    char tmp[PATH_MAX_SIZE], *result;
    if(basePath == NULL)
    {
        return getAbsolutePath();
    }
    //cout<<"test in get absolute path function!"<<endl;
    if(basePath[0] != '/'){
        if(NULL == realpath(basePath, tmp)){
            cout<<"Error in getAbsolutePath: realpath\n"<<endl;
            return NULL;
        }
        strcat(tmp, "/");
    }else{
        strcpy(tmp, basePath);
        if(tmp[strlen(tmp)-1] != '/'){
            strcat(tmp, "/");
        }
    }
    result = new char[sizeof(char)*(strlen(tmp)+1)];
    strcpy(result, tmp);
    //cout<<"test2 in get absolute path function!"<<endl;
    return result;
}
//
// Get all the files contains string in the directory file_dir
//
list<char*> *getAllFileList(char *file_dir = NULL, char *subname = NULL)
{
    DIR *dir;
    struct dirent *ptr;
    char *cur_dir, tmp[PATH_MAX_SIZE], *basepath;
    queue<char*> dirs;
    list<char*> *result = NULL;
    //cur_dir = getAbsolutePath();
    //cout<<"test in get all files function!"<<endl;
    if(file_dir == NULL)
    {
        cout<<"test1 in get all files function!"<<endl;
        if((cur_dir = getAbsolutePath()) == NULL)
        {
            cout<<"Error in get the current path!"<<endl;
            return NULL;
        }
    }else if(IsDir(file_dir)){
        cout<<"test2 in get all files function!"<<endl;
        if((cur_dir = getAbsolutePath(file_dir)) == NULL){
            cout<<"Error in get the directory : "<<file_dir<<endl;
            //return NULL;
        }
        //cur_dir = new char[(strlen(file_dir)+1)*sizeof(char)];
        //strcpy(cur_dir, file_dir);
    }else if(IsFile(file_dir)){     //file_dir is only a file
        cout<<"test3 in get all files function!"<<endl;
        result = new list<char*>();
        basepath = new char[(strlen(file_dir)+1)*sizeof(char)];
        strcpy(basepath, file_dir);
        result->push_back(basepath);
        return result;
    }else{
        cout<<"Error: "<<file_dir <<" is not a file or directory!"<<endl;
        return NULL;
    }
    //
    //basepath = new char[sizeof(char)*(strlen(cur_dir+1))];
    //strcpy(basepath, cur_dir);
    dirs.push(cur_dir);

    //cout<<"test!"<<endl;

    result = new list<char*>();

    while(! dirs.empty())
    {
        cur_dir = dirs.front();
        dirs.pop();
        if((dir = opendir(cur_dir)) == NULL)
        {
            cout<<"Error in open the current directory!"<<endl;
            //return NULL;
            cout<<cur_dir<<endl;
            continue;
        }

        //free(cur_dir);
        //cur_dir = NULL;

        //
        while((ptr = readdir(dir)) != NULL)
        {
            if(ptr->d_type == 10 || strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            {
                continue;
            }
            else if(ptr->d_type == 8)   // file
            {
                sprintf(tmp, "%s%s", cur_dir, ptr->d_name);
                if(strContain(tmp, subname)){
                    basepath = new char[(strlen(tmp)+1)*sizeof(char)];
                    strcpy(basepath, tmp);
                    result->push_back(basepath);
                }
            }
            else if(ptr->d_type == 4)
            {
                sprintf(tmp, "%s%s", cur_dir, ptr->d_name);
                strcat(tmp, "/");
                basepath = new char[(strlen(tmp)+1)*sizeof(char)];
                strcpy(basepath, tmp);
                dirs.push(basepath);
            }
            else
            {
                continue;
            }
        }
        closedir(dir);
    }
    return result;
}

//
// Free the file list
//
void freeFileList(list<char *> *fl)
{
    if(NULL == fl)
        return;
    while(! fl->empty())
    {
        delete fl->front();
        fl->pop_front();
    }
    delete fl;
}
//////////////////////////////////////////////////////
// gk project and class MyRecord
//

//#define DEBUG

// is number
bool IsNumber(char c)
{
    if(c <= '9' && c >= '0')
	return true;
    else
	return false;
}

// WGS GK projection
WGSGK mygk;

int MyGK(double longitude, double latitude, double &x, double &y)
{
    Point p1,p2;
    int md = (int)(longitude+1.5)/3.0;
    mygk.setMeridian(md);
    mygk.setUseWGS(false);
    p1.Set(longitude, latitude);
    if(! mygk.project(p1, p2)){
        cout<<"ERROR: in mygk!"<<endl;
        return -1;
    }
    x = p2.GetX();
    y = p2.GetY();
    return 1;
}

int MyGK(Point &src, Point &result)   // src.x : longitude, src.y : latitude
{
    mygk.setMeridian((int)(src.GetX()+1.5)/3.0);
    mygk.setUseWGS(false);
    if(! mygk.project(src, result)){
        cout<<"ERROR: in mygk!"<<endl;
        return -1;
    }
    return 1;
}

// class MyRecord
class MyRecord {
public:
    char rstr[1024];	 //the string of record;
    bool is_defined;
    bool str_defined;
    int id;   //the record id of the car;
    int yy, mm, dd, h, m, s;   //the date, year, month, day, hour, minute, second;
    double X,Y;
    
    MyRecord();
    void SetRstr(char *str);  //set the record string;
    virtual int FreshData();    //fresh the data of record string;
    virtual int CheckStr();     //check the string of record whether is right;
    int Clear();   //clear all the data of record
    bool IsDefined();  //return the record whether it is available;
    friend ostream & operator <<(ostream &out, const MyRecord &right);
};
ostream &operator <<(ostream &out, const MyRecord &right)
{
    out<<"***********************************************************"<<endl;
    out<<"id:"<<right.id<<"\ndate:"<<right.yy<<"-"<<right.mm<<"-"<<right.dd<<" "<<right.h<<":"<<right.m<<":"<<right.s<<"\ncoordination:"<<right.X<<","<<right.Y<<endl;
    out<<"***********************************************************"<<endl;
    return out;
}

MyRecord::MyRecord() {
    id = yy = mm = dd = h = m = s = 0;
    X = Y = 0.0;
    is_defined = false;
    str_defined = false;
    memset(rstr, 0, sizeof(rstr));
}

int MyRecord::FreshData()
{
    if(false == str_defined)
    {
    	Clear();
    	return -1;
    }
    if(-1 == CheckStr())
    {
    	is_defined = false;
    	Clear();
    	return -1;
    }
    sscanf(rstr, "%d,%d-%d-%d %d:%d:%d,%lf,%lf", &id, &yy, &mm, &dd, &h, &m, &s, &X, &Y);

#ifdef DEBUG
	cout<<"record string: "<<rstr<<endl;
	cout<<"id: "<<id<<endl;
	cout<<"yy: "<<yy<<endl;
	cout<<"mm: "<<mm<<endl;
	cout<<"dd: "<<dd<<endl;
	cout<<"h: "<<h<<endl;
	cout<<"m: "<<m<<endl;
	cout<<"s: "<<s<<endl;
	cout<<"X: "<<X<<endl;
	cout<<"Y: "<<Y<<endl;
#endif
    
    is_defined = true;
    return 1;
}

int MyRecord::CheckStr()  // check the format of the string whether it is right
{
    int pointer = 0, status = 0;
    char temp_c;

    while('\0' != rstr[pointer])
    {
    	temp_c = rstr[pointer++];
    	switch(status)
    	{
	case 0:
	    if(IsNumber(temp_c))
	    {
		status = 1;
	    }
	    else
		status = 10;
	    break;
	case 1:
	    if(IsNumber(temp_c))
	    {
		status = 1;
	    }
	    else if(temp_c == ',')
	    {
		status = 2;
	    }
	    else
		status = 10;
	    break;
	case 2:
	    if(IsNumber(temp_c))
	    {
		status = 2;
	    }
	    else if(temp_c == '-')
		status = 3;
	    else if(temp_c == ' ')
	    	status = 4;
	    else
	    	status = 10;
            break;
	case 3:
	    if(IsNumber(temp_c))
	    	status = 2;
	    else
		status = 10;
	    break;
	case 4:
	    if(IsNumber(temp_c))
	        status = 4;
	    else if(':' == temp_c)
		status = 5;
	    else if(',' == temp_c)
		status = 6;
	    else
		status = 10;
	    break;
	case 5:
	    if(IsNumber(temp_c))
		status = 4;
	    else
		status = 10;
	    break;
	case 6:
	    if(IsNumber(temp_c))
		status = 6;
	    else if('.' == temp_c)
		status = 7;
	    else
		status = 10;
	    break;
	case 7:
	    if(IsNumber(temp_c) || temp_c == '\r')
		status = 7;
	    else if(',' == temp_c)
		status = 6;
	    else
		status = 10;
	    break;
	default:
	    return -1;
	}
    }
    if(7 == status)
	return 1;
    else
	return -1;
}

bool MyRecord::IsDefined()
{
	return is_defined;
}

void MyRecord::SetRstr(char *str)
{
	if(NULL != str)
	{
		strcpy(rstr, str);
		str_defined = true;
	}
	else
	{
		str_defined = false;
	}
}

int MyRecord::Clear()
{
	id = yy = mm = dd = h = m = s = 0;
	X = Y = 0.0;
        is_defined = false;
	memset(rstr, 0, sizeof(rstr));
	return 1;
}
/////////////////////////////////////////////////
// class GPSRecord
//
class GPSRecord
{
public:
    int Oid;   //object id
    int yy,mm,dd,h,m,s;    //date and time
    double lo, la;   //longitude and latitude
};

////////////////////////////////////////////////
// class RecordManager
//
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

    bool setExtension(const char *str)
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
        // check the string format
        while(1 != CheckStringFormat(buf)){
            //check whether the file is to end~
            if(curfile.eof()){
                if(! openNextFile()){
                    return false;
                }
            }
            curfile.getline(buf, 1024);
        }
        // get data from string
        sscanf(buf, "%d,%d-%d-%d %d:%d:%d,%lf,%lf", &gpsr.Oid, &gpsr.yy, &gpsr.mm, &gpsr.dd, &gpsr.h, &gpsr.m, &gpsr.s, &gpsr.lo, &gpsr.la);
        return true;
    }

    int CheckStringFormat(char *rstr)  // check the format of the string whether it is right
    {
        int pointer = 0, status = 0;
        char temp_c;

        while('\0' != rstr[pointer])
        {
            temp_c = rstr[pointer++];
            switch(status)
            {
                case 0:
                    if(IsNumber(temp_c))
                    {
                        status = 1;
                    }
                    else
                        status = 10;
                    break;
                case 1:
                    if(IsNumber(temp_c))
                    {
                        status = 1;
                    }
                    else if(temp_c == ',')
                    {
                        status = 2;
                    }
                    else
                        status = 10;
                    break;
                case 2:
                    if(IsNumber(temp_c))
                    {
                        status = 2;
                    }
                    else if(temp_c == '-')
                        status = 3;
                    else if(temp_c == ' ')
                        status = 4;
                    else
                        status = 10;
                    break;
                case 3:
                    if(IsNumber(temp_c))
                        status = 2;
                    else
                        status = 10;
                    break;
                case 4:
                    if(IsNumber(temp_c))
                        status = 4;
                    else if(':' == temp_c)
                        status = 5;
                    else if(',' == temp_c)
                        status = 6;
                    else
                        status = 10;
                    break;
                case 5:
                    if(IsNumber(temp_c))
                        status = 4;
                    else
                        status = 10;
                    break;
                case 6:
                    if(IsNumber(temp_c))
                        status = 6;
                    else if('.' == temp_c)
                        status = 7;
                    else
                        status = 10;
                    break;
                case 7:
                    if(IsNumber(temp_c) || temp_c == '\r')
                        status = 7;
                    else if(',' == temp_c)
                        status = 6;
                    else
                        status = 10;
                    break;
                default:
                    return -1;
            }
        }
        if(7 == status)
            return 1;
        else
            return -1;
    }
};

#endif
