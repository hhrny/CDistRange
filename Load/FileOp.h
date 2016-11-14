#ifndef FILEOP_H
#define FILEOP_H

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <list>
#include <queue>
using namespace std;

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
/*
//
// Get all the files in the directory file_dir
//
list<char*> *getAllFileList(char *file_dir = NULL)
{
    DIR *dir;
    struct dirent *ptr;
    char *cur_dir, tmp[PATH_MAX_SIZE], *basepath;
    queue<char*> dirs;
    list<char*> *result = NULL;
    //cur_dir = getAbsolutePath();
    cout<<"test in get all files function!"<<endl;
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

    cout<<"test!"<<endl;

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
                basepath = new char[(strlen(tmp)+1)*sizeof(char)];
                strcpy(basepath, tmp);
                result->push_back(basepath);
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
*/

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
    cout<<"test in get all files function!"<<endl;
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

    cout<<"test!"<<endl;

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

#endif
