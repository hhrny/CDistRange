#include <iostream>
#include <stdlib.h>

#include "../Temporal/TemporalAlgebra.h"
#include "mybreak.h"
#include "../Relation-C++/RelationAlgebra.h"
using namespace std;


const double breakupsize = 10000.0;
const int attrindex = 3;

class BreakUpInfo
{
public:
    Tuple    *tuple;
    MPoint         *mpoint;
    UPoint         *upoint;
    int            nofup;  //num of the result upoint
    int            pointer, numofupoint;
    vector<UPoint> breakupoint;
    bool           isempty;
    int            sid;

    BreakUpInfo()
    {
        tuple = NULL;
        mpoint = NULL;
        upoint = NULL;
        pointer = 1;
        numofupoint = 0;
        isempty = true;
        sid = 0;
    }

    bool IsEmpty()
    {
        return isempty;
    }

    int Init(Tuple *tup)
    {
        if(tup == NULL)
        {
            return 0;
        }
        tuple  = tup->Clone();
        mpoint = (MPoint *)(tuple->GetAttribute(attrindex));
        if(mpoint->IsEmpty())
        {
            cout<<"Init: mpoint is null!"<<endl;
        }
        //mpoint->Print(cout);
        numofupoint = mpoint->GetNoComponents();
        pointer = 1;
        sid = 0;
        isempty = false;
        return 1;
    }

    bool HasNext()
    {
        if(pointer > 0 && pointer < numofupoint)
        {
            return true;
        }
        return false;
    }

    int GetNextUPoint(UPoint &up)
    {
        if(HasNext())
        {
            mpoint->Get(pointer,up);
            pointer ++;
            return 1;
        }
        return -1;
    }

    int GetSId()
    {
        return sid;
    }
    
    MPoint * GetNextMPoint()
    {
        MPoint *mpresult = new MPoint(0);
        UPoint up(false);
        Rectangle<2> tempmbr;
        bool   endflag = false;
        //cout<<"program is here: GetNextMPoint!"<<endl;
        while(mpresult->BoundingBoxSpatial().Area() < 1000.0*1000.0)
        {
            //cout<<"program is here: while in GetNextMPoint!"<<endl;

            if(breakupoint.empty())
            {
                if(1==GetNextUPoint(up))
                {
                    //up.Print(cout);
                    breakupoint = breakup(up);
                }
                else
                {
                    //up.Print(cout);
                    isempty = true;
                    endflag = true;
                }
            }
            else
            {
                mpresult->Add(breakupoint.back());
                breakupoint.pop_back();
            }
            if(endflag)
            {
                break;
            }
        }
        if(mpresult->IsEmpty())
        {
            return NULL;
        }
        else
        {
            sid ++;
            return mpresult;
        }
    }
};

