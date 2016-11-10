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

//upoint<->upoint query
int UUCDistRange(const UPoint &upq, const UPoint &up, double dist1, double dist2, queue<UPoint *> &result, int condition);  
//upoint<->mpoint query
int UMCDistRange(const UPoint &upoint, const MPoint &mpoint, double d1, double d2, queue<UPoint *> &result, int condition);
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



#endif
