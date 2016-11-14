
#ifndef RECORD_H
#define RECORD_H

#include "MyRecord.h"

// The data struct of date
struct MyDate
{
    int year;
    int month;
    int day;
};

// The data struct of time
struct MyTime
{
    int hour;
    int minute;
    int second;
};

// The data struct of record
struct MyRecordExt
{
    int    id;
    double longitude;   //
    double latitude;
    MyDate date;
    MyTime tm;
};

class MyRecordTaxi : public MyRecord
{
    public:
        int CheckStr();

        int FreshData();

        bool operator == (const MyRecordTaxi &right);

        friend ostream & operator <<(ostream &out, const MyRecord &right);

        friend MyRecordTaxi operator - (const MyRecordTaxi &left, const MyRecordTaxi &right);
};

bool MyRecordTaxi::operator == (const MyRecordTaxi &right)
{
    if(id == right.id && yy == right.yy && mm == right.mm && dd == right.dd && h == right.h && m == right.m && s == right.s)
        return true;
    else
        return false;
}

ostream &operator <<(ostream &out, const MyRecordTaxi &right)
{
    out<<"***********************************************************"<<endl;
    out<<"id:"<<right.id<<"\ndate:"<<right.yy<<"-"<<right.mm<<"-"<<right.dd<<" "<<right.h<<":"<<right.m<<":"<<right.s<<"\ncoordination:"<<right.X<<","<<right.Y<<endl;
    out<<"***********************************************************"<<endl;
    return out;
}

MyRecordTaxi operator-(const MyRecordTaxi &left, const MyRecordTaxi &right)
{
    MyRecordTaxi res;

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

int MyRecordTaxi::CheckStr()
{
    int pointer = 0, nodot = 0;
    char tmpc;
    while(rstr[pointer] != '\0')
    {
        tmpc = rstr[pointer++];
        if(IsNumber(tmpc) || tmpc == '.' || tmpc == '-' || tmpc == ':' || tmpc == '\n' || tmpc == '\r')
        {
            continue;
        }
        else if(tmpc == ',')
        {
            nodot ++;
        }
        else
        {
            return -1;
        }
    }
    if(nodot != 6)
        return -1;
    return 1;
}

int MyRecordTaxi::FreshData()
{
    //int tmp;
    double dtmp, tmp;
    if(false ==  str_defined)
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
    sscanf(rstr, "%lf,%lf,%d,%lf,%lf,%d-%d-%d,%d:%d:%d", &Y, &X, &id, &tmp, &dtmp, &yy, &mm, &dd, &h, &m, &s);

#ifdef DEBUG
    cout<<"taxi record string: "<< rstr <<endl;
    cout<<"id: "<<id<<endl;
    cout<<"yy: "<<yy<<endl;
    cout<<"mm: "<<mm<<endl;
    cout<<"dd: "<<dd<<endl;
    cout<<"h:  "<<h<<endl;
    cout<<"m:  "<<m<<endl;
    cout<<"s:  "<<s<<endl;
    cout<<"X:  "<<X<<endl;
    cout<<"Y:  "<<Y<<endl;
#endif

    if(MyGK(X,Y,X,Y) == -1){
        cout<<"Error: in MyRecord::FreshData::MyGK!"<<endl;
        is_defined = false;
        Clear();
        return -1;
    }
    X -= 48700000;
    Y -= 50400000;
    if(X < 0 || Y < 0){
        cout<<"x and y is out of the region!"<<endl;
    }
    dt =dd*86400 + h*3600 + m*60 + s;
    is_defined = true;
    return 1;
}

#endif
