
/*
 * MyRecord.h
 *
 *  Created on: Jul 19, 2015
 *      Author: hhr
 */

#ifndef MYRECORD_H_
#define MYRECORD_H_

//#define DEBUG

//#include "MyRecord.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string.h>

#include "SpatialAlgebra.h"
using namespace std;

WGSGK mygk;

int MyGK(double la, double lo, double &x, double &y)
{
    Point p1,p2;
    int md = (int)(la+1.5)/3.0;
    mygk.setMeridian(md);
    p1.Set(la, lo);
    if(! mygk.project(p1, p2)){
        cout<<"ERROR: in mygk!"<<endl;
        return -1;
    }
    x = p2.GetX();
    y = p2.GetY();
    return 1;
}

class MyRecord {
public:
    char rstr[1024];	 //the string of record;
    bool is_defined;
    bool str_defined;
    int  dt;    //date and time store in int; 
    int id;   //the record id of the car;
    int yy, mm, dd, h, m, s;   //the date, year, month, day, hour, minute, second;
    double X, Y;  //the coordinate of the car with GPS;
    double tx,ty;
    MyRecord();

    double centerx, centery;

    virtual ~MyRecord();

    void SetRstr(char *str);  //set the record string;

    virtual int FreshData();    //fresh the data of record string;

    virtual int CheckStr();     //check the string of record whether is right;

    int Clear();   //clear all the data of record

    bool IsDefined();  //return the record whether it is available;

    bool operator == (const MyRecord &right);

    friend ostream & operator <<(ostream &out, const MyRecord &right);

    friend MyRecord operator-(const MyRecord &left, const MyRecord &right);
};
ostream &operator <<(ostream &out, const MyRecord &right)
{
    out<<"***********************************************************"<<endl;
    out<<"id:"<<right.id<<"\ndate:"<<right.yy<<"-"<<right.mm<<"-"<<right.dd<<" "<<right.h<<":"<<right.m<<":"<<right.s<<"\ncoordination:"<<right.X<<","<<right.Y<<endl;
    out<<"***********************************************************"<<endl;
    return out;
}

bool MyRecord::operator == (const MyRecord &right)
{
    if(id == right.id && yy == right.yy && mm == right.mm && dd == right.dd && h == right.h && m == right.m && s == right.s)
        return true;
    else
        return false;
}

MyRecord operator-(const MyRecord &left, const MyRecord &right)
{
    MyRecord res;

    res.yy = left.yy - right.yy;
    res.mm = left.mm - right.mm;
    res.dd = left.dd - right.dd;
    res.h = left.h - right.h;
    res.m = left.m - right.m;
    res.s = left.s - right.s;
    res.X = left.X - right.X;
    res.Y = left.Y - right.Y;

    if(res.s < 0)
    {
        res.s += 60;
        res.m --;
    }
    if(res.m < 0)
    {
        res.m += 60;
        res.h --;
    }
    if(res.h < 0)
    {
        res.h += 24;
        res.dd --;
    }
    if(res.dd < 0)
    {
        res.dd += 30;
        res.mm --;
    }
    if(res.mm < 0)
    {
        res.mm += 12;
        res.yy --;
    }
    if(res.yy < 0)
    {
        res.Clear();
        res.is_defined = false;
    }
    else
    {
        res.is_defined = true;
    }
    res.str_defined = false;
    return res;
}

bool IsNumber(char c)
{
    if(c <= '9' && c >= '0')
	return true;
    else
	return false;
}

MyRecord::MyRecord() {
    // TODO Auto-generated constructor stub
    id = yy = mm = dd = h = m = s = dt = 0;
    X = Y = 0.0;
    is_defined = false;
    str_defined = false;
    memset(rstr, 0, sizeof(rstr));
    MyGK(116.424722, 39.905555, centerx, centery);
}

MyRecord::~MyRecord() {
    // TODO Auto-generated destructor stub
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

    if(X < 113 || X > 119 || Y < 37 || Y > 43){
        return -1;
    }

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
    if(MyGK(X,Y,X,Y) == -1){
        cout<<"Error: in MyRecord::FreshData::MyGK!"<<endl;
        is_defined = false;
        Clear();
        return -1;
    }
    
    X -= centerx;
    Y -= centery;
    /*
    if(X < 0 || Y < 0){
        cout<<"x and y is out of the region!"<<endl;
    }
    */
    dt =dd*86400 + h*3600 + m*60 + s;
    is_defined = true;
//  str_defined = false;
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
	id = yy = mm = dd = h = m = s = dt = 0;
	X = Y = 0.0;
	is_defined = false;
	memset(rstr, 0, sizeof(rstr));
	return 1;
}

#endif /* MYRECORD_H_ */
