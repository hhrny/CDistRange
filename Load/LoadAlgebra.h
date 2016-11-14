#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <DateTime.h>
#include "MyRecord.h"
#include "breakupinfo.h"
#include "record.h"
#include "FileOp.h"

using namespace std;


int split(char in[128], char out[4][30], char ch)
{
  int p1, row, col;
  p1 = row = col = 0;
  for(p1 = 0; in[p1] != '\0'; p1 ++)
  {
	if(in[p1] == '\n')
		break;
    if(in[p1] == ch)
    {
      out[row][col] = '\0';
      row ++;
      col = 0;
    }
    else
    {
      out[row][col] = in[p1];
      col ++;
    }
  }
  out[row][col] = '\0';
  return row+1;
}

int formatdate(char sd[30])  //fix the date format;  from y-m-d h:m:s to y-m-d-h:m:s
{
	int i;
	for(i = 0; sd[i] != '\0'; i ++)
	{
		if(sd[i] == ' ')
		{
			sd[i] = '-';
			break;
		}
	}
	return 0;
}

int producetaxtrip(char rec1[4][30], char rec2[4][30])  //using the two recorder to produce a taxtrip;
{
	if(0 == strcmp(rec1[0],rec2[0]))
	{
		printf("((\"%s\" \"%s\" TURE FALSE)(%s %s %s %s))\n",rec1[1],rec2[1],rec1[2],rec1[3],rec2[2],rec2[3]);
	}
	else if(rec1[0][0] == '\0' || rec2[0][0] == '\0')
	{
		printf("%s%s\n",rec1[0],rec2[0]);
		printf("(\n");
	}
	else
	{
		printf(")\n%s\n(\n",rec2[0]);
	}
	return 0;
}

class ListRec{    //list recorder
  private:	
  char        rec[4][30];
  ListRec          *next;
  public:
  ListRec()
  {
    next = NULL;
  }
  ~ListRec() {}
  
  //set record
  //
  int setrec(int p,char *s)
  {
    strcpy(rec[p],s);
    return 0;
  }


  //get id
  int getid()
  {
    int res = 0;
    res = atoi(rec[0]);
    return res;
  }

  //get time(string)
  string getdate()
  {
    string temp;
    formatdate(rec[1]);
    temp = rec[1];
    return temp;
  }

  double gettime()
  {
    datetime::DateTime tt(datetime::instanttype);
    int y,m,d,h,mi,s;
    sscanf(rec[1], "%d-%d-%d %d-%d-%d", &y, &m, &d, &h, &mi, &s);
    tt.Set(y,m,d,h,mi,s);
    return tt.ToDouble();
  }

  char *getdate2()
  {
    return rec[1];
  }

  //get loc_x
  double getloc_x()
  {
    double res = 0.0;
    res = strtod(rec[2],NULL);
    return res;
  }

  //get loc_y
  double getloc_y()
  {
    double res = 0.0;
    res = strtod(rec[3],NULL);
    return res;
  }
};
