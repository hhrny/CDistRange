#ifndef CDistRangeALGEBRA_H
#define CDistRangeALGEBRA_H
#include "StandardTypes.h"
#include "Symbols.h"
#include "DateTime.h"
#include "Stream.h"
#include "Attribute.h"
#include "DateTime.h"
#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"
#include "Symbols.h"
#include "../RTree/RTreeAlgebra.h"
#include "../TBTree/TBTree.h"
#include "../SETI/SETIAlgebra.h"
#include <stack>
#include <queue>
#include <list>
#include <iostream>

using namespace std;



//      1: d1 = 0.0,  0.0 < d2 < inf
//      2: 0.0 < d1 < inf, d2 = inf
//      3: d1 = 0.0, d2 = inf
//      4: 0.0 < d1 < inf, 0.0 < d2 < inf

const int FPART = 1,   // part of the upoint satisfy the query requirement
          FALL = 2,   // all of the upoint
          FNONE = 0;  // error!!!

struct FilterResult{
    int     type;
    TupleId tid;
};

class RTreeVisitor{
    R_Tree<3, TupleId>  *rtree;
    Rectangle<3>        rect3D;   //restore the query bounding box
    Rectangle<2>        rect2D;
    stack<SmiRecordId>  sidstack;
    queue<FilterResult> tidqueue;
    bool                isdefined;

    int    condition;
    double dist1, dist2;
    
public:
    RTreeVisitor(){
        rtree = NULL;
        isdefined = false;
        condition = 0;
        dist1 = dist2 = 0.0;
    }
    //
    RTreeVisitor(R_Tree<3, TupleId> *rt, Rectangle<3> r, int c, double d1, double d2 = 0.0):
        rtree(rt),
        rect3D(r),
        condition(c),
        dist1(d1)
    {
        isdefined = false;
        dist2 = d2;
    }
    //
    void setRTree(R_Tree<3, TupleId> *rt){
        rtree = rt;
    }
    //
    void setRect(Rectangle<3> r){
        rect3D = r;
    }
    //
    void setDistance(int c, double d1, double d2 = 0.0){
        condition = c;
        dist1 = d1;
        dist2 = d2;
    }
    // return true if there is a rtree leaf entry to calc
    bool hasNext();
    // initial the rtree
    bool Init();
    // get the next filter result from rtree
    bool getNext(FilterResult &result);
    // get the leaf entries from rtree and push into result queue
    bool perform();
};

class TBTreeVisitor{
    tbtree::TBTree      *tbt;
    Rectangle<3>        rect3D;   //restore the query bounding box
    Rectangle<2>        rect2D;
    stack<SmiRecordId>  sidstack;
    queue<FilterResult> tidqueue;
    bool                isdefined;

    int    condition;
    double dist1, dist2;
    
public:
    TBTreeVisitor(){
        tbt = NULL;
        isdefined = false;
        condition = 0;
        dist1 = dist2 = 0.0;
    }
    //
    TBTreeVisitor(tbtree::TBTree *tb, Rectangle<3> r, int c, double d1, double d2 = 0.0):
        tbt(tb),
        rect3D(r),
        condition(c),
        dist1(d1)
    {
        isdefined = false;
        dist2 = d2;
    }
    //
    void setTBTree(tbtree::TBTree *tb){
        tbt = tb;
    }
    //
    void setRect(Rectangle<3> r){
        rect3D = r;
    }
    //
    void setDistance(int c, double d1, double d2 = 0.0){
        condition = c;
        dist1 = d1;
        dist2 = d2;
    }
    // return true if there is a tbtree leaf entry to calc
    bool hasNext();
    // initial the tbtree
    bool Init();
    // get the next filter result from tbtree
    bool getNext(FilterResult &result);
    // get the leaf entries from tbtree and push into result queue
    bool perform();
};

//
class SETIVisitor{
    SETI                *seti;
    Rectangle<3>        rect3D;   //restore the query bounding box
    Rectangle<2>        rect2D, searchbox;
    stack<SmiRecordId>  sidstack;
    queue<SETICell *>   seticellqueue;
    list<TrjSeg *>      trjsegs;
    bool                isdefined;

    int    condition;
    double dist1, dist2;
    
public:
    SETIVisitor(){
        seti = NULL;
        isdefined = false;
        condition = 0;
        dist1 = dist2 = 0.0;
    }
    //
    SETIVisitor(SETI *setiptr, Rectangle<3> r, int c, double d1, double d2 = 0.0):
        seti(setiptr),
        rect3D(r),
        condition(c),
        dist1(d1)
    {
        isdefined = false;
        dist2 = d2;
    }
    //
    void setSETI(SETI *setiptr){
        seti = setiptr;
    }
    //
    void setRect(Rectangle<3> r){
        rect3D = r;
    }
    //
    void setDistance(int c, double d1, double d2 = 0.0){
        condition = c;
        dist1 = d1;
        dist2 = d2;
    }
    // return true if there is a tbtree leaf entry to calc
    bool hasNext();
    // initial the tbtree
    bool Init();
    // get the next filter result from tbtree
    TrjSeg *getNext();
    // get the leaf entries from tbtree and push into result queue
    bool perform();
};



// local information
//
class CDistRangeLocalInfo
{
public:
    Stream<Tuple>   *stream;
    MPoint          *mpoint;        // mpoint of query
    int             attrindex;     //index of the upoint in tuple 
    int             condition;  // the condition of distance [d1,d2]
    double          dist1,dist2;     
    Tuple           *tuple;
    ListExpr        tupletype;
    queue<UPoint *> tmpresult;


    UPoint *getNextUPoint(){
        UPoint *tmp = tmpresult.front();
        tmpresult.pop();
        return tmp;
    }

    bool isEmpty(){
        return tmpresult.empty();
    }
};
// local information
//
class CDistRangeMMLocalInfo
{
public:
    Stream<Tuple>   *stream;
    MPoint          *mpoint;        // mpoint of query
    int             attrindex;     //index of the upoint in tuple 
    int             condition;  // the condition of distance [d1,d2]
    double          dist1,dist2;     
    Tuple           *tuple;
    ListExpr        tupletype;
};

//upoint<->upoint query
int UUCDistRange(const UPoint &upq, const UPoint &up, double dist1, double dist2, queue<UPoint *> &result, int condition);  
//upoint<->mpoint query
int UMCDistRange(const UPoint &upoint, const MPoint &mpoint, double d1, double d2, queue<UPoint *> &result, int condition);
//mpoint<->mpoint query
int MMCDistRange(const MPoint &mp, const MPoint &mq, double d1, double d2, MPoint &result, int condition);
//
int CalcResultType(const Rectangle<2u> &r1, const Rectangle<2u> &r2, double dist1, double dist2, int cond);
//
UPoint *getUPointFromTrjSeg(const TrjSeg &trjseg);

//
//
bool RTreeVisitor::hasNext(){
    if(sidstack.empty() && tidqueue.empty()){
        isdefined = false;
        //cout<<"Filter Visitor is not defined or null!!!"<<endl;
        return false;
    }
    return true;
}
//
//
bool RTreeVisitor::Init(){
    if(rtree != NULL && rect3D.IsDefined() && condition > 0 && condition <= 4){
        sidstack.push(rtree->RootRecordId());
        rect2D = rect3D.Project2D(0,1);
        if(perform()){
            return true;
        }
    }
    return false;
}
//
//
bool RTreeVisitor::getNext(FilterResult &result){
    if(! hasNext()){
        return false;
    }
    if(tidqueue.empty()){
        if(! perform()){
            return false;
        }
    }
    result = tidqueue.front();
    tidqueue.pop();
    return true;
}
//
//
bool RTreeVisitor::perform(){
    R_TreeNode<3, TupleId> rtreenode(true, rtree->MinInternalEntries(), rtree->MaxInternalEntries());
    SmiRecordId            sid;
    int                    noentries = 0, i;
    Rectangle<2>           rect2;
    FilterResult           fresult;
    R_TreeEntry<3>         *re = NULL;
    
    if(! hasNext()){
        return false;
    }
    while(tidqueue.empty() && (! sidstack.empty())){
        // get a smirecordid from sid stack
        sid = sidstack.top();
        sidstack.pop();
        // get a rtree node
        rtree->GetNode(sid, rtreenode);
        noentries = rtreenode.EntryCount();
        for(i = 0; i < noentries; i ++){
            // get the entry
            re = rtreenode.GetEntry(i);
            // compare the time interval
            if(rect3D.MinD(2) >= re->box.MaxD(2) || rect3D.MaxD(2) <= re->box.MinD(2)){
                continue;
            }
            rect2 = re->box.Project2D(0,1);
            assert(rect2.IsDefined());

            // calculate the condition of two rectangle
            fresult.type = CalcResultType(rect2, rect2D, dist1, dist2, condition);
            if(fresult.type == FNONE){
                continue;
            }
            if(rtreenode.IsLeaf()){
                // rtree node is leaf node
                fresult.tid = ((R_TreeLeafEntry<3, TupleId> *)re)->info;
                // add to tupleid queue
                tidqueue.push(fresult);
            }
            else{
                // rtree node is internal node, and add SmiRecordId to sid stack
                sidstack.push(((R_TreeInternalEntry<3> *)re)->pointer);
            }
        }
    }
    return hasNext();
}
//
//
bool TBTreeVisitor::hasNext(){
    if(sidstack.empty() && tidqueue.empty()){
        isdefined = false;
        //cout<<"Filter Visitor is not defined or null!!!"<<endl;
        return false;
    }
    return true;
}
//
//
bool TBTreeVisitor::Init(){
    if(tbt != NULL && rect3D.IsDefined() && condition > 0 && condition <= 4){
        sidstack.push(tbt->getRootId());
        rect2D = rect3D.Project2D(0,1);
        if(perform()){
            return true;
        }
    }
    return false;
}
//
//
bool TBTreeVisitor::getNext(FilterResult &result){
    if(! hasNext()){
        return false;
    }
    if(tidqueue.empty()){
        if(! perform()){
            return false;
        }
    }
    result = tidqueue.front();
    tidqueue.pop();
    return true;
}
//
//
bool TBTreeVisitor::perform(){
    tbtree::BasicNode<3>   *tbtnode;
    SmiRecordId            sid;
    int                    noentries = 0, i;
    Rectangle<2>           rect2;
    FilterResult           fresult;
    
    const tbtree::Entry<3, tbtree::TBLeafInfo> *le = NULL;
    const tbtree::Entry<3, tbtree::InnerInfo>  *ie = NULL;
    
    if(! hasNext()){
        return false;
    }
    while(tidqueue.empty() && (! sidstack.empty())){
        // get a smirecordid from sid stack
        sid = sidstack.top();
        sidstack.pop();
        // get a tbtree node
        tbtnode = tbt->getNode(sid);
        noentries = tbtnode->entryCount();
        for(i = 0; i < noentries; i ++){
            if(tbtnode->isLeaf()){
                // get the leaf entry
                le = ((tbtree::Node<3, tbtree::TBLeafInfo> *)tbtnode)->getEntry(i);
                // compare the time interval
                if(rect3D.MinD(2) >= le->getBox().MaxD(2) || rect3D.MaxD(2) <= le->getBox().MinD(2)){
                    continue;
                }
                rect2 = le->getBox().Project2D(0,1);
                assert(rect2.IsDefined());

                // calculate the condition of two rectangle
                fresult.type = CalcResultType(rect2, rect2D, dist1, dist2, condition);
                if(fresult.type == FNONE){
                    continue;
                }
                // tbtree node is leaf node
                fresult.tid = le->getInfo().getTupleId();
                // add to tupleid queue
                tidqueue.push(fresult);
            }
            else{
                // get the inner entry
                ie = ((tbtree::Node<3, tbtree::InnerInfo> *)tbtnode)->getEntry(i);
                // compare the time interval
                if(rect3D.MinD(2) >= ie->getBox().MaxD(2) || rect3D.MaxD(2) <= ie->getBox().MinD(2)){
                    continue;
                }
                rect2 = ie->getBox().Project2D(0,1);
                assert(rect2.IsDefined());

                // calculate the condition of two rectangle
                fresult.type = CalcResultType(rect2, rect2D, dist1, dist2, condition);
                if(fresult.type == FNONE){
                    continue;
                }
                // tbtree node is internal node, and add SmiRecordId to sid stack
                sidstack.push(ie->getInfo().getPointer());
            }
        }
    }
    return hasNext();
}

//
//
bool SETIVisitor::hasNext(){
    if(seticellqueue.empty() && trjsegs.empty()){
        isdefined = false;
        //cout<<"Filter Visitor is not defined or null!!!"<<endl;
        return false;
    }
    return true;
}
//
//
bool SETIVisitor::Init(){
    R_TreeLeafEntry<2, TupleId> e;
    if(seti != NULL && rect3D.IsDefined() && condition > 0 && condition <= 4){
        rect2D = rect3D.Project2D(0,1);
        // construct search box using time interval
        searchbox = Rectangle<2>(true, rect3D.MinD(2)-0.1, rect3D.MaxD(2)+0.1, -1.0, 1.0);
    
        if(! FindCells(seti, rect2D, dist1, dist2, condition, seticellqueue)){
            return false;
        }
        //cout<<"test:1"<<endl;
        while(! seticellqueue.empty()){
            //cout<<"test:2"<<endl;
            if(seticellqueue.front()->rtreePtr != NULL){
                //cout<<"test:3"<<endl;
                if(seticellqueue.front()->rtreePtr->First(searchbox, e)){
                    //cout<<"test:4"<<endl;
                    ReadTrjSegFromPage(seti, (db_pgno_t)e.info, trjsegs);
                    break;
                }
            }
            seticellqueue.pop();
        }
        return hasNext();
    }
    return false;
}
//
//
TrjSeg *SETIVisitor::getNext(){
    if(! hasNext()){
        return NULL;
    }
    if(trjsegs.empty()){
        if(! perform()){
            return NULL;
        }
    }
    TrjSeg *result = trjsegs.front();
    trjsegs.pop_front();
    return result;
}
//
//
bool SETIVisitor::perform(){
    R_TreeLeafEntry<2, TupleId> e;

    if(! hasNext()){
        return false;
    }
    while(trjsegs.empty() && (! seticellqueue.empty())){
        // get a data page trjsegs
        if(seticellqueue.front()->rtreePtr->Next(e)){
            ReadTrjSegFromPage(seti, (db_pgno_t)e.info, trjsegs);
        }
        else{
            seticellqueue.pop();
            while(! seticellqueue.empty()){
                if(seticellqueue.front()->rtreePtr != NULL){
                    if(seticellqueue.front()->rtreePtr->First(searchbox, e)){
                        ReadTrjSegFromPage(seti, (db_pgno_t)e.info, trjsegs);
                        break;
                    }
                }
                seticellqueue.pop();
            }
        }
    }
    return hasNext();
}
//
//
int CalcResultType(const Rectangle<2u> &r1, const Rectangle<2u> &r2, double dist1, double dist2, int cond){
    double maxdist, mindist;

    maxdist = r1.MaxDist(r2);
    mindist = r1.MinDist(r2);
    switch(cond){
    case 1:
        if(maxdist <= dist2){
            return FALL;
        }else if(mindist > dist2){
            return FNONE;
        }else{
            return FPART;
        }
    case 2:
        if(mindist >= dist1){
            return FALL;
        }else if(maxdist < dist2){
            return FNONE;
        }else{
            return FPART;
        }
    case 3:
        return FALL;
    case 4:
        if(maxdist < dist2 && mindist > dist1){
            return FALL;
        }else if(maxdist < dist1 || mindist > dist2){
            return FNONE;
        }else{
            return FPART;
        }
    default:
        cout<<"condition error in CalcResultType function"<<endl;
        return FNONE;
    }
    return FNONE;
}

//
int UMCDistRange(const UPoint &upoint, const MPoint &mpoint, double d1, double d2, queue<UPoint *> &result, int condition){
    int     i, j, length, no_result = 0;
    UPoint  up1, up2;     //up1: upoint in mpoint, up2: upoint at time interval of up1
    UReal   udist(true);
    MPoint  *mp = new MPoint(0);
    Periods periods(0), result_periods(2), merge_periods(0);
    Interval<Instant> timeinterval;
    
    // get the mpoint at interval upoint.timeInterval
    periods.StartBulkLoad();
    periods.Add(upoint.timeInterval);
    periods.EndBulkLoad();
    periods.SetDefined(true);
    mpoint.AtPeriods(periods, *mp);
    if(! mp->IsDefined()){
        delete mp;
        return 0;
    }

    //
    //cout<<"upoint time interval: ";
    //upoint.timeInterval.Print(cout);
    //cout<<endl;


    merge_periods.Clear();
    merge_periods.StartBulkLoad();

    // get the number of upoint in temporal mpoint mp
    length = mp->GetNoComponents();
    for(i = 0; i < length; i ++){
        mp->Get(i, up1);

        //cout<<"upoint-"<<i<<" time interval: ";
        //up1.timeInterval.Print(cout);
        //cout<<endl;
        //upoint.AtInterval(up1.timeInterval, up2);
        
        // computer the distance between upoint up1 and upoint up2
        //up1.Distance(up2, udist, NULL);
        upoint.Distance(up1, udist, NULL);

        //cout<<"Distance of two upoint:";
        //udist.Print(cout);
        //cout<<endl;

        if(! udist.IsDefined()){
            cerr << __PRETTY_FUNCTION__ << "Invalid geographic coord found!"<<endl;
            delete mp;
            return -1;
        }

        result_periods.Clear();
        
        // deal with 4 different conditions
        switch(condition){
            case 1:
                // 1: d1 = 0.0,  0.0 < d2 < inf
                no_result = udist.PeriodsAtValB(d2, result_periods);
                break;
            case 2:
                // 2: 0.0 < d1 < inf, d2 = inf
                no_result = udist.PeriodsAtValA(d1, result_periods);
                break;
            case 3:
                // 3: d1 = 0.0, d2 = inf
                no_result = 1;
                //result_periods.Add(up1.timeInterval);
                result_periods.StartBulkLoad();
                result_periods.MergeAdd(udist.timeInterval);
                result_periods.EndBulkLoad();
                break;
            case 4:
                // 4: 0.0 < d1 < inf, 0.0 < d2 < inf
                no_result = udist.PeriodsAtValInterval(d1, d2, result_periods);
                break;
            default:
                return 0;
        }
        //UPoint *up3;
        for(j = 0; j < no_result; j ++){
            //up3 = new UPoint(0);
            result_periods.Get(j, timeinterval);
            //up2.AtInterval(timeinterval, *up3, 0);
            //result.push(up3);
            merge_periods.MergeAdd(timeinterval);
            //cout<<"Merge add to periods: ";
            //timeinterval.Print(cout);
            //cout<<endl;
        }
    }
    merge_periods.EndBulkLoad();
    
    UPoint *up3;
    no_result = merge_periods.GetNoComponents();
    for(j = 0; j < no_result; j ++){
        up3 = new UPoint(0);
        merge_periods.Get(j, timeinterval);
        upoint.AtInterval(timeinterval, *up3, 0);
        result.push(up3);
    }

    return result.size();
}

// computer the upoint in up which distance from upq between dist1 and dist2
//
int UUCDistRange(const UPoint &upq, const UPoint &up, double dist1, double dist2, queue<UPoint *> &result, int condition)
{
    int     j, no_result;
    Periods result_periods(2);
    UReal   udist(true);
    
    Interval<Instant> timeinterval;

    assert(upq.IsDefined());
    assert(up.IsDefined());

    upq.Distance(up, udist, NULL);
    if(! udist.IsDefined()){
        cerr << __PRETTY_FUNCTION__ << "Invalid geographic coord found!"<<endl;
        return -1;
    }

    // deal with 4 different conditions
    switch(condition){
        case 1:
            // 1: d1 = 0.0,  0.0 < d2 < inf
            no_result = udist.PeriodsAtValB(dist2, result_periods);
            break;
        case 2:
            // 2: 0.0 < d1 < inf, d2 = inf
            no_result = udist.PeriodsAtValA(dist1, result_periods);
            break;
        case 3:
            // 3: d1 = 0.0, d2 = inf
            no_result = 1;
            result_periods.StartBulkLoad();
            result_periods.Add(udist.timeInterval);
            result_periods.EndBulkLoad();
            break;
        case 4:
            // 4: 0.0 < d1 < inf, 0.0 < d2 < inf
            no_result = udist.PeriodsAtValInterval(dist1, dist2, result_periods);
            break;
        default:
            return 0;
    }
    UPoint *up3;
    for(j = 0; j < no_result; j ++){
        up3 = new UPoint(0);
        result_periods.Get(j, timeinterval);
        up.AtInterval(timeinterval, *up3, 0);
        result.push(up3);
    }
    return result.size();
}

//mpoint<->mpoint query
int MMCDistRange(const MPoint &mp, const MPoint &mq, double d1, double d2, MPoint &result, int condition)
{
    int               rplen, i, j, pos1, pos2, no_result = 0;
    UPoint            up1, up2, up3;
    Periods           result_periods(2);
    Interval<Instant> tiv;
    result.Clear();
    if(! mp.IsDefined() || ! mq.IsDefined()){
        result.SetDefined(false);
        return 0;
    }
    result.SetDefined(true);
    result.StartBulkLoad();

    UReal udist(true);
    RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(mp, mq);
    rplen = rp.Size();
    for(i = 0; i < rplen; i ++){
        rp.Get(i, tiv, pos1, pos2);
        if(pos1 == -1 || pos2 == -1){
            continue;
        }
        // get the upoint in the same time interval
        mp.Get(pos1, up1);
        mq.Get(pos2, up2);

        // assert up1 and up2 defined
        assert(up1.IsDefined());
        assert(up2.IsDefined());

        up1.Distance(up2, udist, NULL);
        if(! udist.IsDefined())
        {
            cerr << __PRETTY_FUNCTION__ << "Invalid geographic coord found!" << endl;
            result.EndBulkLoad(false, false);
            result.Clear();
            result.SetDefined(false);
            return -1;
        }
        // deal with 4 different conditions
        switch(condition){
            case 1:
                // 1: d1 = 0.0,  0.0 < d2 < inf
                no_result = udist.PeriodsAtValB(d2, result_periods);
                break;
            case 2:
                // 2: 0.0 < d1 < inf, d2 = inf
                no_result = udist.PeriodsAtValA(d1, result_periods);
                break;
            case 3:
                // 3: d1 = 0.0, d2 = inf
                no_result = 1;
                result_periods.StartBulkLoad();
                result_periods.Add(udist.timeInterval);
                result_periods.EndBulkLoad();
                break;
            case 4:
                // 4: 0.0 < d1 < inf, 0.0 < d2 < inf
                no_result = udist.PeriodsAtValInterval(d1, d2, result_periods);
                break;
            default:
                return 0;
        }
        for(j = 0; j < no_result; j ++){
            result_periods.Get(j, tiv);
            up1.AtInterval(tiv, up3, 0);
            result.MergeAdd(up3);
        }
    }
    result.EndBulkLoad();
    if(result.IsEmpty()){
        result.SetDefined(false);
        return 0;
    }else{
        return 1;
    }
}

//
UPoint *getUPointFromTrjSeg(const TrjSeg &trjseg)
{
    UPoint   *result;
    DateTime ustart(instanttype), uend(instanttype);
    Interval<Instant> uiv;

    ustart.ReadFrom(trjseg.tivStart);
    uend.ReadFrom(trjseg.tivEnd);
    uiv.CopyFrom(Interval<Instant>(ustart, uend, true, true));

    result = new UPoint(uiv, trjseg.pos1.x, trjseg.pos1.y, trjseg.pos2.x, trjseg.pos2.y);
    return result;
}

/*
 * 3D Point
 */

class Point3D
{
    public:
        static const int MaxVal = 1<<20, MinVal = -(1<<20);
        int x, y, t;
        unsigned long long toULL(){
            unsigned long long result = 0, tmp;
            if(! (t >= MinVal && t <= MaxVal)){
                cout<<"Error: value t out of range!"<<endl;
                return 0;
            }
            if(! (y >= MinVal && y <= MaxVal)){
                cout<<"Error: value y out of range!"<<endl;
                return 0;
            }
            if(! (x >= MinVal && x <= MaxVal)){
                cout<<"Error: value x out of range!"<<endl;
                return 0;
            }
            result = t + MaxVal;
            result = result << 21;
            tmp = y + MaxVal;
            result |= tmp;
            result = result << 21;
            tmp = x + MaxVal;
            result |= tmp;
            return result;
        }
        void ReadFromULL(unsigned long long val){
            unsigned long long tmp = (1 << 21) - 1;
            x = (int)(val & tmp) - MaxVal;
            val = val >> 21;
            y = (int)(val & tmp) - MaxVal;
            val = val >> 21;
            t = (int)(val & tmp) - MaxVal;
        }
        void Print(ostream &out){
            out<<"3D Point:(x:"<<x<<",\ty:"<<y<<",\tt:"<<t<<")";
        }
        double DistanceSquare(Point3D &p){
            return (p.x-x)*(p.x-x) + (p.y-y)*(p.y-y) + (p.t-t)*(p.t-t);
        }
        double Distance(Point3D &p){
            return sqrt(DistanceSquare(p));
        }
        bool operator==(Point3D &p) const{
            if(p.x == x && p.y == y && p.t == t){
                return true;
            }
            return false;
        }
        bool operator!=(Point3D &p) const{
            if(p.x == x && p.y == y && p.t == t){
                return false;
            }
            return true;
        }
};

Point3D GetCenterofPoints(vector<Point3D> points){
    int sumx = 0, sumy = 0, sumt = 0;
    vector<Point3D>::iterator it;
    Point3D result;

    for(it = points.begin(); it != points.end(); it ++){
        sumx += it->x;
        sumy += it->y;
        sumt += it->t;
    }
    result.x = sumx/points.size();
    result.y = sumy/points.size();
    result.t = sumt/points.size();
    return result;
}

// k-means 
int KMeans(unsigned int k, vector<Point3D> &points, vector<vector<Point3D> > &result){
    unsigned int j = 0, i = 0, min;
    bool changed;
    double mindist, dist;
    Point3D tmppoint;
    vector<Point3D> center, tmp;
    vector<Point3D>::iterator it;
    tmp.clear();
    result.clear();
    // size < k
    if(points.size() < k){
        cout<<"error: the number of points is less than k value!"<<endl;
        return -1;
    }
    // size == k
    if(points.size() == k){
        for(i = 0; i < k; i ++){
            tmp.push_back(points[i]);
            result.push_back(tmp);
            tmp.clear();
        }
        return k;
    }
    // initialize k center
    for(i = 0; i < k; i ++){
        center.push_back(points[i]);
        result.push_back(tmp);
    }
    for(j = 0; j < 100; j ++){
        changed = false;
        for(it = points.begin(); it != points.end(); it++){
            // despatch all the points to k collection
            mindist = it->DistanceSquare(center[0]);
            min = 0;
            for(i = 1; i < k; i ++){
                // find the minimum distance center
                dist = it->DistanceSquare(center[i]);
                if(dist < mindist){
                    min = i;
                    mindist = dist;
                }
            }
            // push the point to minimum distance center
            result[min].push_back(*it);
        }
        for(i = 0; i < k; i ++){
            // compute the new center
            tmppoint = GetCenterofPoints(result[i]);
            if(tmppoint != center[i]){
                // update the center
                center[i] = tmppoint;
                changed = true;
            }
        }
        if(! changed){
            // end to k-means
            return k;
        }
        // clear the result
        for(i = 0; i < k; i ++){
            result[i].clear();
        }
    }
    cout<<"Warning: can not divide points to k collection!"<<endl;
    return -1;
}

/*
   bulk load using grid

   class RTreeLevel : the one level of rtree node when bulk load using grid

*/
// 3 dimission, x, y and t
class RTreeLevel
{
    private:
        // rtree to bulk load
        R_Tree<3, TupleId> *rtree;
        // the next level pointer, when the number of node entry in this level gird is bigger than max entries number
        // generate a next level, and insert the entries to next level
        RTreeLevel *nextlevel;
        // all the entries in this level
        int noentries;
        int entrycount, nodecount;
        // length, width and time of each grid unit 
        double  unitx, unity, unittime;
        // leaf
        bool isleaf;
        int maxentries, minentries;
        map<unsigned long long, R_TreeNode<3, TupleId> *> grid;
        
        //
        int Insert(R_TreeNode<3, TupleId> *node);

    public:
        RTreeLevel(bool isleaf, R_Tree<3, TupleId> *rt);
        RTreeLevel(bool isleaf, R_Tree<3, TupleId> *rt, double ux, double uy, double ut);
        ~RTreeLevel();

        Point3D convertMBR2P(const Rectangle<3> &box){
            Point3D result;
            result.x = (box.MaxD(0)-box.MinD(0))/2/unitx;
            result.y = (box.MaxD(1)-box.MinD(1))/2/unity;
            result.t = (box.MaxD(2)-box.MinD(2))/2/unittime;
            return result;
        }
        int SaveNode2RTree(R_TreeNode<3, TupleId> *node);
        //int Insert(TupleId tid, const Rectangle<3> &box);
        int Insert(SmiRecordId sid, const Rectangle<3> &box);
        // get the root of all the node
        SmiRecordId GetRoot();
        int GetHeight();
        int Append(RTreeLevel *rl);
        void SetRTreeRoot();
        int EntryCount();
        int NodeCount();
};

RTreeLevel::RTreeLevel(bool isleaf, R_Tree<3, TupleId> *rt){
    this->isleaf = isleaf;
    this->rtree = rt;
    if(isleaf){
        // this is leaf level
        maxentries = rt->MaxLeafEntries();
        minentries = rt->MinLeafEntries();
    }
    else{
        // this is internal level
        maxentries = rt->MaxInternalEntries();
        minentries = rt->MinInternalEntries();
    }
    this->unitx = 1000.0;
    this->unity = 1000.0;
    this->unittime = 1.0/24;  // one hour
    this->nextlevel = NULL;
    noentries = 0;
    entrycount = 0;
    nodecount = 0;
}

RTreeLevel::RTreeLevel(bool isleaf, R_Tree<3, TupleId> *rt, double ux, double uy, double ut){
    this->isleaf = isleaf;
    this->rtree = rt;
    if(isleaf){
        // this is leaf level
        maxentries = rt->MaxLeafEntries();
        minentries = rt->MinLeafEntries();
    }
    else{
        // this is internal level
        maxentries = rt->MaxInternalEntries();
        minentries = rt->MinInternalEntries();
    }
    this->unitx = ux;
    this->unity = uy;
    this->unittime = ut;
    this->nextlevel = NULL;
    noentries = 0;
    entrycount = 0;
    nodecount = 0;
}

RTreeLevel::~RTreeLevel(){
    map<unsigned long long, R_TreeNode<3, TupleId> *>::iterator it;
    if(nextlevel != NULL){
        delete nextlevel;
    }
    for(it = grid.begin(); it != grid.end(); it++){
        delete it->second;
    }
    grid.clear();
}

// save the data of node to rtree
// and insert the index entry of this node to nextlevel
// and minus the number of entries of this node
int RTreeLevel::SaveNode2RTree(R_TreeNode<3, TupleId> *node){
    int AppendRecord = 0, tmp = 0;
    Rectangle<3> bbox;
    SmiRecord    record;
    SmiRecordId  sid;

    if(node == NULL){
        cout<<"node is null!"<<endl;
        return -1;
    }
    bbox = node->BoundingBox();
    tmp = node->EntryCount();
    AppendRecord = rtree->file->AppendRecord(sid, record);
    assert(AppendRecord);
    node->SetModified();
    node->Write(record);
    if(nextlevel == NULL){
        nextlevel = new RTreeLevel(false, rtree, unitx, unity, unittime);
    }
    nextlevel->Insert(sid, bbox);
    // minus the save entries
    noentries -= tmp;
    return 1;
}

// insert a entry
int RTreeLevel::Insert(SmiRecordId id, const Rectangle<3> &box){
    Point3D     p;
    unsigned long long key;
    map<unsigned long long, R_TreeNode<3, TupleId> *>::iterator it;
    
    p = convertMBR2P(box);
    key = p.toULL();
    it = grid.find(key);
    if(it == grid.end()){
        // this is no node with key
        grid[key] = new R_TreeNode<3, TupleId>(isleaf, minentries, maxentries);
    }
    // insert the entry to node
    if(isleaf){
        grid[key]->Insert(R_TreeLeafEntry<3, TupleId>(box, id));
    }
    else{
        grid[key]->Insert(R_TreeInternalEntry<3>(box, id));
    }
    noentries ++;
    entrycount ++;
    // make sure the node is no full
    if(grid[key]->EntryCount() >= maxentries){
        // node is full
        SaveNode2RTree(grid[key]);
        grid[key]->Clear();
        nodecount ++;
    }
    return 1;
}
// insert all entries of a node
int RTreeLevel::Insert(R_TreeNode<3, TupleId> *node){
    int            i; 
    Point3D        p;
    unsigned long long key;
    map<unsigned long long, R_TreeNode<3, TupleId> *>::iterator it;

    if(node == NULL){
        cout<<"node is null!"<<endl;
        return -1;
    }
    //cout<<"test 1"<<endl;
    //cout<<"isleaf: "<<isleaf;
    //cout<<"\t"<<node->IsLeaf()<<endl;
    if(isleaf != node->IsLeaf()){
        cout<<"error in insert node to rtreelevel!"<<endl;
        return -1;
    }
    //cout<<"test 2"<<endl;
    if(node->EntryCount() <= 0){
        return 0;
    }
    p = convertMBR2P(node->BoundingBox());
    key = p.toULL();
    it = grid.find(key);
    if(it == grid.end()){
        // there is no node in p
        grid[key] = new R_TreeNode<3, TupleId>(isleaf, minentries, maxentries);
    }
    // insert all the entry in node to new node
    for(i = 0; i < node->EntryCount(); i ++){
        // get a entry of this node
        // insert a entry to this node
        grid[key]->Insert((*node)[i]);
        noentries ++;
        entrycount ++;
        // make sure the node is no full
        if(grid[key]->EntryCount() >= maxentries){
            // node is full
            SaveNode2RTree(grid[key]);
            grid[key]->Clear();
            nodecount ++;
        }
    }
    return 1;
}

int RTreeLevel::Append(RTreeLevel *rl){
    map<unsigned long long, R_TreeNode<3, TupleId> *>::iterator it;
    if(rl->isleaf != isleaf){
        cout<<"error: in append rtree level!"<<endl;
        return -1;
    }
    //cout<<"test append 1"<<endl;
    for(it = rl->grid.begin(); it != rl->grid.end(); it++){
        Insert(it->second);
    }
    //cout<<"test append 2"<<endl;
    if(rl->nextlevel != NULL){
        nextlevel->Append(rl->nextlevel);
    }
    //cout<<"test append 3"<<endl;
    return 1;
}

int RTreeLevel::GetHeight(){
    if(nextlevel == NULL){
        return 0;
    }
    return nextlevel->GetHeight() + 1;
}

SmiRecordId RTreeLevel::GetRoot(){
    int          i, j;
    unsigned long long key;
    SmiRecord    record;
    //SmiRecordId  sid;
    //int          AppendRecord = 0;
    R_TreeNode<3, TupleId> *n;
    Point3D                p;
    vector<Point3D>        points;
    map<unsigned long long, R_TreeNode<3, TupleId> *>::iterator it;
    vector<Point3D>::iterator pit;
    vector<vector<Point3D> > result;
    RTreeLevel             *rl, *oldrl = this;
    while(oldrl->noentries > (maxentries * 2)){
        // more than two node to save
        rl = new RTreeLevel(isleaf, rtree, oldrl->unitx*2, oldrl->unity*2, oldrl->unittime*2);
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            rl->Insert(it->second);
        }
        if(rl->nextlevel != NULL){
            // the new rtreelevel's next level is no NULL
            // add all the entry in the new rtreelevel's next level to this rtreelevel
            if(this->nextlevel == NULL){
                // check the nextlevel whether is null
                this->nextlevel = new RTreeLevel(false, rtree, unitx, unity, unittime);
            }
            // problem is here
            this->nextlevel->Append(rl->nextlevel);
            //cout<<"test delete 1"<<endl;
            delete rl->nextlevel;
            //cout<<"test delete 2"<<endl;
            rl->nextlevel = NULL;
        }
        if(oldrl != this){
            delete oldrl;
        }
        //cout<<"test get root 1"<<endl;
        oldrl = rl;
    }
    // old rtreelevel
    if(oldrl->noentries > maxentries){
        //cout<<"test get root 3"<<endl;
        // devide all the entries in this level to two node
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
        //cout<<"test get root 2"<<endl;
            p.ReadFromULL(it->first);
            points.push_back(p);
        }
        if(2 != KMeans(2, points, result)){
            cout<<"error in dispatch all entries to two node!"<<endl;
            return 0;
        }
        for(i = 0; i < 2; i ++){
            n = new R_TreeNode<3, TupleId>(isleaf, minentries, maxentries);
            for(pit = result[i].begin(); pit != result[i].end(); pit++){
                key = pit->toULL();
                for(j = 0; j < (oldrl->grid[key])->EntryCount(); j++){
                    n->Insert((*(oldrl->grid[key]))[j]);
                }
            }
            SaveNode2RTree(n);
            delete n;
        }
    }
    else if(oldrl->noentries >= minentries){
        //cout<<"test get root 4"<<endl;
        // the left entries of this level is less than max entries, collect all the entries as a node
        n = new R_TreeNode<3, TupleId>(isleaf, minentries, maxentries);
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            for(i = 0; i < (it->second)->EntryCount(); i++){
                n->Insert((*(it->second))[i]);
            }
        }
        SaveNode2RTree(n);
        delete n;
        if(oldrl != this){
            delete oldrl;
        }
    }
    else if(this->nextlevel == NULL){
        //cout<<"test get root 5"<<endl;
        // this is a root level
        // collect all the entries in this level as a root node
        //n = new R_TreeNode<3, TupleId>(isleaf, minentries, maxentries);
        // get the root node from rtree
        n = rtree->GetNode(rtree->RootRecordId(), 0, minentries, maxentries);
        n->Clear();
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            for(i = 0; i < (it->second)->EntryCount(); i++){
                n->Insert((*(it->second))[i]);
            }
        }
        //AppendRecord = rtree->file->AppendRecord(sid, record);
        //assert(AppendRecord);
        //n->SetModified();
        //n->Write(record);
        rtree->PutNode(rtree->RootRecordId(),&n);
        //delete n;
        if(oldrl != this){
            delete oldrl;
        }
        return rtree->RootRecordId();
    }
    else{
        cout<<"error in function GetRoot(): the number of entries is less than the min number of entry!"<<endl;
        return 0;
    }
    return nextlevel->GetRoot();
}

int RTreeLevel::EntryCount(){
    if(nextlevel == NULL){
        return entrycount;
    }
    return entrycount + nextlevel->EntryCount();
}

int RTreeLevel::NodeCount(){
    if(nextlevel == NULL){
        return nodecount;
    }
    return nodecount + nextlevel->NodeCount();
}

void RTreeLevel::SetRTreeRoot(){
    GetRoot();
    rtree->header.height = GetHeight();
    rtree->header.entryCount = EntryCount();
    rtree->header.nodeCount = NodeCount();
    if(rtree->nodePtr != NULL){
        delete rtree->nodePtr;
    }
    rtree->nodePtr = rtree->GetNode(rtree->RootRecordId(), 0, rtree->MinEntries(0), rtree->MaxEntries(0));
}

#endif
