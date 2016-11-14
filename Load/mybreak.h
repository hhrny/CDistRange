#include <iostream>
#include <cstdlib>
#include "../Temporal/TemporalAlgebra.h"
#include "../Spatial/Point.h"
#include <vector>
#include "DateTime.h"
using namespace std;

const double length = 1000.0;

double getUp(double cur)
{
	int temp = cur / length;
	return (temp + 1) * length;
}

double getDown(double cur)
{
	int temp = cur / length;
	return temp * length;
}

double getNext(double cur, int direct)
{
	int temp = cur / length;
	if (cur == temp*length)
	{
		return cur + direct * length;
	}
	if (direct == 1)
	{
		return getUp(cur);
	}
	else if (direct == -1)
	{
		return getDown(cur);
	}
	else
	{
		return cur;
	}
}


int getDirect(double p0, double p1)
{
	if (p1 > p0)
		return 1;
	else if (p1 < p0)
		return -1;
	else
		return 0;
}

double distQuad(Point p1, Point p2)
{
	return (p2.GetX() - p1.GetX())*(p2.GetX() - p1.GetX()) + (p2.GetY() - p1.GetY())*(p2.GetY() - p1.GetY());
}




void breakup(Point s, Point e)
{
	struct Point p1, p2;
	double segdist = 0.0;  //the quadratic of distance 
	double a1, b1, a2, b2;
	double curx, cury;
	double dist1, dist2;
        double tempd;
	int    dirx, diry, segpointer = 1;
	bool   conflag = true;

	
	cout << "(" << s.GetX() << "," << s.GetY() << ")->(" << e.GetX() << "," << e.GetY() << ")" << endl;
	//calculate the a1,b1,a2,b2

	a1 = (e.GetY() - s.GetY()) / (e.GetX() - s.GetX());
	b1 = s.GetY() - a1*s.GetX();

	a2 = (e.GetX() - s.GetX()) / (e.GetY() - s.GetY());
	b2 = s.GetX() - a2*s.GetY();

	//
	dirx = getDirect(s.GetX(), e.GetX());
	diry = getDirect(s.GetY(), e.GetY());

	//
	curx = s.GetX();
	cury = s.GetY();
	//
	segdist = (s.GetX() - e.GetX())*(s.GetX() - e.GetX()) + (s.GetY() - e.GetY())*(s.GetY() - e.GetY());

	//p1.x = getNext(curx, dirx);
	//p1.y = a1*p1.x + b1;
        tempd = getNext(curx, dirx);
        p1.Set(tempd, a1*tempd+b1);

	//p2.y = getNext(cury, diry);
	//p2.x = a2*p2.y + b2;
        tempd = getNext(cury, diry);
        p2.Set(tempd, a2*tempd+b2);

	dist1 = distQuad(p1, s);
	if (dist1 >= segdist)
	{
		//p1.x = e.x;
		//p1.y = e.y;
		p1.Set(e.GetX(), e.GetY());
                dist1 = segdist;
	}
	dist2 = distQuad(p2, s);
	if (dist2 >= segdist)
	{
		//p2.y = e.x;
		//p2.x = e.y;
		p2.Set(e.GetX(), e.GetY());
                dist2 = segdist;
	}
	while (conflag)
	{
		if (dist1 - dist2 > 0.0001)
		{
                        cout<<"1:"<<endl;
			cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p2.GetX() << "," << p2.GetY() << ")" << endl;
			curx = p2.GetX();
			cury = p2.GetY();
			//p2.y = getNext(cury, diry);
			//p2.x = a2*p2.y + b2;
                        tempd = getNext(cury, diry);
                        p2.Set(tempd, a2*tempd+b2);
                        
			dist2 = distQuad(p2, s);
			if (dist2 >= segdist)
			{
				//p2.y = e.x;
				//p2.x = e.y;
		                p2.Set(e.GetX(), e.GetY());
				dist2 = segdist;
			}
		}
		else if (dist1 - dist2 < -0.0001)
		{
                        cout<<"2:"<<endl;
			cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p1.GetX() << "," << p1.GetY() << ")" << endl;
			curx = p1.GetX();
			cury = p1.GetY();
			//p1.x = getNext(curx, dirx);
			//p1.y = a1*p1.x + b1;
                        tempd = getNext(curx, dirx);
                        p1.Set(tempd, a1*tempd+b1);
			dist1 = distQuad(p1, s);
			if (dist1 >= segdist)
			{
				//p1.x = e.x;
				//p1.y = e.y;
		                p1.Set(e.GetX(), e.GetY());
				dist1 = segdist;
			}
		}
		else
		{
                        cout<<"3:"<<endl;
			cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p1.GetX() << "," << p1.GetY() << ")" << endl;
			if (dist1 == segdist && dist2 == segdist)
			{
				conflag = false;
			}
			//curx = p1.x;
			//cury = p1.y;
			//p1.x = getNext(curx, dirx);
			//p1.y = a1*p1.x + b1;
			curx = p1.GetX();
			cury = p1.GetY();
                        tempd = getNext(curx, dirx);
                        p1.Set(tempd, a1*tempd+b1);
			dist1 = distQuad(p1, s);
			if (dist1 >= segdist)
			{
				//p1.x = e.x;
				//p1.y = e.y;
				p1.Set(e.GetX(), e.GetY());
                                dist1 = segdist;
			}
                        tempd = getNext(cury, diry);
                        p2.Set(tempd, a2*tempd+b2);
			dist2 = distQuad(p2, s);
			if (dist2 >= segdist)
			{
				//p2.y = e.x;
				//p2.x = e.y;
                                p2.Set(e.GetX(), e.GetY());
				dist2 = segdist;
			}
		}
	}
}

vector<UPoint> breakup(UPoint upoint)
{
	Point p1, p2;
        UPoint tempup;
	double segdist = 0.0;  //the quadratic of distance 
	double a1, b1, a2, b2;
	double curx, cury, curt, tint;  //current time, tint:time interval
	double dist1, dist2, t1, t2, starttime, tempd;
	int    dirx, diry;
	bool   conflag = true;
        vector<UPoint> result;
        DateTime temptime(instanttype);

	cout << "(" << upoint.p0.GetX() << "," << upoint.p0.GetY() << ")->(" << upoint.p1.GetX() << "," << upoint.p1.GetY() << ")" << endl;
	//calculate the a1,b1,a2,b2

        if(upoint.p0.GetX() == upoint.p1.GetX() || upoint.p0.GetY() == upoint.p1.GetY())
        {
            result.push_back(upoint);
            return result;
        }

	a1 = (upoint.p1.GetY() - upoint.p0.GetY()) / (upoint.p1.GetX() - upoint.p0.GetX());
	b1 = upoint.p0.GetY() - a1*upoint.p0.GetX();

	a2 = (upoint.p1.GetX() - upoint.p0.GetX()) / (upoint.p1.GetY() - upoint.p0.GetY());
	b2 = upoint.p0.GetX() - a2*upoint.p0.GetY();

        //
        //
        starttime = upoint.timeInterval.start.ToDouble();
	//
	dirx = getDirect(upoint.p0.GetX(), upoint.p1.GetX());
	diry = getDirect(upoint.p0.GetY(), upoint.p1.GetY());

	//
	curx = upoint.p0.GetX();
	cury = upoint.p0.GetY();
	curt = starttime;//upoint.timeInterval.start.ToDouble();
	//tint = (upoint.timeInterval.end.ToDouble() - upoint.timeInterval.start.ToDouble());//.ToDouble();
	//tint = (upoint.timeInterval.end - upoint.timeInterval.start).ToDouble();
	tint = (upoint.timeInterval.end.ToDouble() - curt);//upoint.timeInterval.start).ToDouble();
	//
	segdist = (upoint.p0.GetX() - upoint.p1.GetX())*(upoint.p0.GetX() - upoint.p1.GetX()) + (upoint.p0.GetY() - upoint.p1.GetY())*(upoint.p0.GetY() - upoint.p1.GetY());

	//p1.x = getNext(curx, dirx);
	//p1.y = a1*p1.x + b1;
        tempd = getNext(curx, dirx);
        p1.Set(tempd, a1*tempd+b1);

	//p2.y = getNext(cury, diry);
	//p2.x = a2*p2.y + b2;
        tempd = getNext(cury, diry);
        p2.Set(tempd, a2*tempd+b2);

	dist1 = distQuad(p1, upoint.p0);
	if (dist1 >= segdist)
	{
		//p1.x = upoint.p1.x;
		//p1.y = upoint.p1.y;
		p1.Set(upoint.p1.GetX(), upoint.p1.GetY());
                dist1 = segdist;
	}
	t1 = (dist1/segdist)*tint + starttime; //upoint.timeInterval.start.ToDouble();

	dist2 = distQuad(p2, upoint.p0);
	if (dist2 >= segdist)
	{
		//p2.y = upoint.p1.x;
		//p2.x = upoint.p1.y;
		p2.Set(upoint.p1.GetX(), upoint.p1.GetY());
		dist2 = segdist;
	}
	t2 = (dist2/segdist)*tint + starttime;//upoint.timeInterval.start.ToDouble();
	while (conflag)
	{
		if (dist1 - dist2 > 0.0001)
		{
                        
                        cout<<"1:"<<endl;
                        
                        cout<<"curx:"<<curx<<"\tcury:"<<cury<<endl;
                        cout<<"p1.x:"<<p1.GetX()<<"\tp1.y:"<<p1.GetY()<<endl;
                        cout<<"p2.x:"<<p2.GetX()<<"\tp2.y:"<<p2.GetY()<<endl;
                        cout<<"upoint.p0"<<upoint.p0.GetX()<<"\t"<<upoint.p0.GetY()<<endl;
                        cout<<"upoint.p1"<<upoint.p1.GetX()<<"\t"<<upoint.p1.GetY()<<endl;
                        cout<<"dist1:"<<dist1<<"\tdist2:"<<dist2<<endl;
                        /*
			cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p2.GetX() << "," << p2.GetY() << ")\t";
			cout<<"["<<curt<<"~"<<t2<<"]"<<endl;
                        */
                        //tempup.p0.x = curx;
                        //tempup.p0.y = cury;
                        //tempup.p1.x = p2.x;
                        //tempup.p1.y = p2.y;
                        tempup.p0.Set(curx,cury);
                        tempup.p1.Set(p2.GetX(), p2.GetY());
                        //cout<<"test begin"<<endl;
                        temptime.ReadFrom(curt);
                        tempup.timeInterval.start = temptime;//.ReadFrom(temptime.ToString());//curt);// = curt;
                        temptime.ReadFrom(t2);
                        tempup.timeInterval.end = temptime;//.ReadFrom(temptime.ToString());//t2); // = t2;
                        //cout<<"test end"<<endl;
                        cout<<"test begin:1"<<endl;
                        result.push_back(tempup);
                        cout<<"test end:1"<<endl;
			curx = p2.GetX();
			cury = p2.GetY();
			curt = t2;
			//p2.y = getNext(cury, diry);
			//p2.x = a2*p2.y + b2;
                        tempd = getNext(cury, diry);
                        p2.Set(tempd, a2*tempd+b2);
			dist2 = distQuad(p2, upoint.p0);
			if (dist2 >= segdist)
			{
				//p2.y = upoint.p1.x;
				//p2.x = upoint.p1.y;
                                p2.Set(upoint.p1.GetX(), upoint.p1.GetY());
				dist2 = segdist;
			}
			t2 = (dist2/segdist)*tint + starttime; //upoint.timeInterval.start.ToDouble();;
		}
		else if (dist1 - dist2 < -0.0001)
		{
                        cout<<"2:"<<endl;
			//cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p1.GetX() << "," << p1.GetY() << ")\t";
			//cout<<"["<<curt<<"~"<<t1<<"]"<<endl;
                        //tempup.p0.x = curx;
                        //tempup.p0.y = cury;
                        //tempup.p1.x = p1.x;
                        //tempup.p1.y = p1.y;
                        tempup.p0.Set(curx,cury);
                        tempup.p1.Set(p1.GetX(), p1.GetY());
                        //cout<<"test begin"<<endl;
                        //tempup.timeInterval.start.ReadFrom(curt);// = curt;
                        //tempup.timeInterval.end.ReadFrom(t1); // = t1;
                        temptime.ReadFrom(curt);
                        tempup.timeInterval.start = temptime;//.ReadFrom(temptime.ToString());//curt);// = curt;
                        temptime.ReadFrom(t1);
                        tempup.timeInterval.end = temptime;//.ReadFrom(temptime.ToString());//t1); // = t1;
                        //cout<<"test end"<<endl;
                        //result.push_back(tempup);
                        cout<<"test begin:2"<<endl;
                        result.push_back(tempup);
                        cout<<"test end:2"<<endl;
			curx = p1.GetX();
			cury = p1.GetY();
			curt = t1;
			//p1.x = getNext(curx, dirx);
			//p1.y = a1*p1.x + b1;
                        tempd = getNext(curx, dirx);
                        p1.Set(tempd, a1*tempd+b1);
			dist1 = distQuad(p1, upoint.p0);
			if (dist1 >= segdist)
			{
				//p1.x = upoint.p1.x;
				//p1.y = upoint.p1.y;
		                p1.Set(upoint.p1.GetX(), upoint.p1.GetY());
				dist1 = segdist;
			}
			t1 = (dist1/segdist)*tint + starttime; //upoint.timeInterval.start;
		}
		else
		{
                        
                        cout<<"3:"<<endl;
			/*
                        cout << "the " << segpointer++ << "th segment is (" << curx << "," << cury << ")->(" << p1.GetX() << "," << p1.GetY() << ")\t";
			cout<<"["<<curt<<"~"<<t1<<"]"<<endl;
                        */
                        //tempup.p0.x = curx;
                        //tempup.p0.y = cury;
                        //tempup.p1.x = p1.x;
                        //tempup.p1.y = p1.y;
                        tempup.p0.Set(curx,cury);
                        tempup.p1.Set(p1.GetX(), p1.GetY());
                        //cout<<"test begin"<<endl;
                        //tempup.timeInterval.start.ReadFrom(curt);// = curt;
                        //tempup.timeInterval.end.ReadFrom(t1); // = t1;
                        temptime.ReadFrom(curt);
                        tempup.timeInterval.start = temptime;//.ReadFrom(temptime.ToString());//curt);// = curt;
                        temptime.ReadFrom(t1);
                        tempup.timeInterval.end = temptime; //.ReadFrom(temptime.ToString());//t1); // = t1;
                        //cout<<"test end"<<endl;
                        result.push_back(tempup);
                        /*
                        cout<<"test begin:3"<<endl;
                        cout<<"curx:"<<curx<<"\tcury:"<<cury<<endl;
                        cout<<"p1.x:"<<p1.GetX()<<"\tp1.y:"<<p1.GetY()<<endl;
                        cout<<"p2.x:"<<p2.GetX()<<"\tp2.y:"<<p2.GetY()<<endl;
                        //cout<<"upoint.p0"<<upoint.p0.GetX()<<"\t"<<upoint.p0.GetY()<<endl;
                        //cout<<"upoint.p1"<<upoint.p1.GetX()<<"\t"<<upoint.p1.GetY()<<endl;
                        upoint.Print(cout);
                        cout<<"\ndist1:"<<dist1<<"\tdist2:"<<dist2<<endl;
                        cout<<"t1:"<<t1<<"\tt2:"<<t2<<endl;
                        cout<<"curt:"<<curt<<"\ttint:"<<tint<<endl;
                        tempup.Print(cout);cout<<endl;
                        result.push_back(tempup);
                        cout<<"test end:3"<<endl;
                        */
			if (dist1 == segdist || dist2 == segdist)
			{
				conflag = false;
			}
			curx = p1.GetX();
			cury = p1.GetY();
			curt = t1;
			//p1.x = getNext(curx, dirx);
			//p1.y = a1*p1.x + b1;
                        tempd = getNext(curx, dirx);
                        p1.Set(tempd, a1*tempd+b1);
			dist1 = distQuad(p1, upoint.p0);
			if (dist1 >= segdist)
			{
				//p1.x = upoint.p1.x;
				//p1.y = upoint.p1.y;
		                p1.Set(upoint.p1.GetX(), upoint.p1.GetY());
				dist1 = segdist;
			}
			t1 = (dist1/segdist)*tint + starttime;//upoint.timeInterval.start;
                        tempd = getNext(cury, diry);
                        p2.Set(tempd, a2*tempd+b2);
			dist2 = distQuad(p2, upoint.p0);
			if (dist2 >= segdist)
			{
				//p2.y = upoint.p1.x;
				//p2.x = upoint.p1.y;
		                p2.Set(upoint.p1.GetX(), upoint.p1.GetY());
				dist2 = segdist;
			}
			t2 = (dist2/segdist)*tint + starttime;//upoint.timeInterval.start;
		}
	}
        return result;
}


int test()
{
struct Point ps, pe;
//char temp[128];

//ps.x = 10;
//ps.y = 10;
ps.Set(10, 10);
//pe.x = 30;
//pe.y = 20;
pe.Set(30, 20);

breakup(ps, pe);

//ps.x = 30;
//ps.y = 20;
ps.Set(30, 20);
//pe.x = 7;
//pe.y = 8;
ps.Set(7, 8);

breakup(ps, pe);

//while (1)
//{
//	ps.x = rand() % 100;
//	ps.y = rand() % 100;

//	pe.x = rand() % 100;
//	pe.y = rand() % 100;

//	cout << "(" << ps.x << "," << ps.y << ")->(" << pe.x << "," << pe.y << ")" << endl;

//	breakup(ps, pe);

//	cin >> temp;
	//}

	struct UPoint up;
	up.p0 = ps;
	up.p1 = pe;
	up.timeInterval.start = 0.0;
	up.timeInterval.end = 10.0;


	breakup(up);


	return 0;
}



