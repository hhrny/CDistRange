#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "DateTime.h"
#include "Stream.h"
#include "WinUnix.h"
#include "SecondoSystem.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "DateTime.h"
#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"
#include "Symbols.h"
#include "RelationAlgebra.h"
#include "CDistRangeAlgebra.h"

extern NestedList     *nl;
extern QueryProcessor *qp;

/****************************************************************

    1.operator CDistRange

***************************************************************/

// type map function
//
ListExpr CDistRangeTM(ListExpr args)
{
    int    flag = 0;     //1: dist2 is a real, 2: dist2 is a inf, 0: error
    
    //error message;
    string msg = "stream x mpoint x real(dist1) x real(dist2) x int(attrindex) expected";
    //the number of args is 5: stream x mpoint x double x double x int
    if(nl->ListLength(args) != 5)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr mpoint = nl->Second(args);
    ListExpr dist1 = nl->Third(args);
    ListExpr dist2 = nl->Fourth(args);
    ListExpr attrindex = nl->Fifth(args);

    if(! listutils::isStream(stream))
    {
        ErrorReporter::ReportError(msg + " (first args is not a relation)");
        return listutils::typeError();
    }
    if(! nl->IsEqual(mpoint, MPoint::BasicType()))
    {
        ErrorReporter::ReportError(msg + " (second args is not a mpoint)");
        return nl->TypeError();
    }
    /*
    cout<<nl->ToString(stream)<<endl;
    cout<<nl->ToString(mpoint)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    cout<<nl->ToString(attrindex)<<endl;
    */
    if(! (nl->IsEqual(dist1, CcReal::BasicType()))){
        ErrorReporter::ReportError(msg + " (third args is not real)");
        return nl->TypeError();
    }
    if(nl->IsEqual(dist2, CcReal::BasicType())){     // 0.0 < d2 < inf
        flag = 1;
    }else if(nl->IsEqual(dist2, "inf") or nl->IsEqual(dist2, "INF")){       // d2 = inf
        flag = 2;
    }else{                                             // type error
        ErrorReporter::ReportError(msg + " (fourth args is not real or inf)");
        return nl->TypeError();
    }

    // check the attribute index
    string   attrname = nl->SymbolValue(attrindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != UPoint::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type upoint.");
        }
    }
    // construct the result type ListExpr
    //ListExpr resType = stream;
    //return NList( NList(Symbol::APPEND()), NList(j).enclose(), resType ).listExpr();
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->TwoElemList(nl->IntAtom(index), nl->IntAtom(flag)), stream);
}

//      1: d1 = 0.0,  0.0 < d2 < inf
//      2: 0.0 < d1 < inf, d2 = inf
//      3: d1 = 0.0, d2 = inf
//      4: 0.0 < d1 < inf, 0.0 < d2 < inf
//



//value map function
// stream x mpoint x real(dist1) x real(dist2) x string(attrindex) x int(attrindex) x int(condition)
int CDistRangeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    UPoint *up    = NULL;
    Tuple  *tuple = NULL;
    int    i,
           noattr = 0,
           flag = 0;

    // local information
    CDistRangeLocalInfo *localinfo = NULL;

    switch( message )
    {
        case OPEN:

            if(local.addr){
                localinfo = (CDistRangeLocalInfo *)(local.addr);
                delete localinfo;
            }
            localinfo = new CDistRangeLocalInfo();

            // get the arguments
            //
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            localinfo->mpoint = (MPoint *)(args[1].addr);
            localinfo->dist1 = ((CcReal *)(args[2].addr))->GetValue();
            localinfo->attrindex = ((CcInt *)(args[5].addr))->GetValue()-1;
            
            // deal with condition
            flag = ((CcInt *)(args[6].addr))->GetValue();
            if(flag == 1){
                localinfo->dist2 = ((CcReal *)(args[3].addr))->GetValue();
                if(localinfo->dist1 < 0.0 || localinfo->dist1 >= localinfo->dist2){
                    cout<<"Error: dist1 less than 0.0 or dist2 less than dist1!"<<endl;
                    return CANCEL;
                }else if(localinfo->dist1 == 0.0){
                    localinfo->condition = 1;
                }else{
                    localinfo->condition = 4;
                }
            }else if(flag == 2){
                localinfo->dist2 = -1.0;
                if(localinfo->dist1 < 0.0){
                    cout<<"Error: dist1 less than 0.0!"<<endl;
                    return CANCEL;
                }else if(localinfo->dist1 == 0.0){
                    localinfo->condition = 3;
                }else{
                    localinfo->condition = 2;
                }
            }else{
                cout<<"Error: dist2 error!"<<endl;
                return CANCEL;
            }

            localinfo->tupletype = nl->Second(GetTupleResultType(s));
            // open the stream
            //
            localinfo->stream->open();

            // set the local informantion
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            
            if(! local.addr){
                cout<<"local.addr is null"<<endl;
                return CANCEL;
            }
            localinfo = (CDistRangeLocalInfo *)local.addr;

            // the tmpresult queue is empty
            while(localinfo->isEmpty()){
                localinfo->tuple = localinfo->stream->request();   // get next tuple from stream

                if(localinfo->tuple == NULL){   
                    // There is no tuple to deal with
                    //cout<<"There is no tuple to deal with!"<<endl;
                    return CANCEL;
                }
                up = (UPoint *)(localinfo->tuple->GetAttribute(localinfo->attrindex));

                // computer the result upoints
                UMCDistRange(*up, *localinfo->mpoint, localinfo->dist1, localinfo->dist2, localinfo->tmpresult, localinfo->condition);
                
                delete up;
            }

            // deal with the result upoint, construct the tuple with result upoint and add to result stream
            tuple = new Tuple(localinfo->tupletype);
            // copy the data to new tuple
            noattr = localinfo->tuple->GetNoAttributes();
            for(i = 0; i < noattr; i ++){
                if(i != localinfo->attrindex){
                    tuple->CopyAttribute(i, localinfo->tuple, i);
                }else{
                    tuple->PutAttribute(i, (Attribute*)localinfo->getNextUPoint());
                }
            }
            result.setAddr(tuple);
            return YIELD;

        case CLOSE:
            
            if(local.addr){
                localinfo = (CDistRangeLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                delete localinfo;
            }
            //cout<<"local.addr is null"<<endl;
            return 0;
    }
    return 0;
}

//
// operator info
struct CDistRangeInfo : OperatorInfo {
    CDistRangeInfo()
    {
        name      = "cdist_range";
        signature = "stream(tuple([a1:d1, ..., ai:upoint, ..., an:dn])) x mpoint x double x double x ai"
                    " -> stream(tuple([a1:d1, ..., ai:upoint, ..., an:dn]))";
        syntax    = "_ cdist_range [ _, _, _, _ ]";
        meaning   = "continuous distace range query between upoint and mpoint";
    }
};


/****************************************************************

    2.operator RTreeFilter

***************************************************************/

// type map function
// rel (tuple([a1:d1, ..., an:dn])) x rtree x rect3 x real(dist1) x real(dist2)-> stream (tuple (a1:d1, ..., an:dn)
ListExpr RTreeFilterTM(ListExpr args)
{
    int    flag = 0;     //1: dist2 is a real, 2: dist2 is a inf, 0: error
    
    //error message;
    string msg = "rel x rtree x rect3 x real(dist1) x real(dist2) expected";
    //the number of args is 5: stream x mpoint x double x double x int
    if(nl->ListLength(args) != 5)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr rel = nl->First(args);
    ListExpr rtree = nl->Second(args);
    ListExpr rect = nl->Third(args);
    ListExpr dist1 = nl->Fourth(args);
    ListExpr dist2 = nl->Fifth(args);
    
    // check the relation
    if(! listutils::isRelDescription(rel))
    {
        ErrorReporter::ReportError(msg + " (first args is not a relation)");
        return listutils::typeError();
    }
    ListExpr relTupleDescription = nl->Second(rel);
    // check the rtree
    if(! listutils::isRTreeDescription(rtree)){
        ErrorReporter::ReportError(msg + " (second args is not a rtree)");
        return listutils::typeError();
    }
    ListExpr rtreeKeyType = listutils::getRTreeType(rtree);
    if((! listutils::isSpatialType(rtreeKeyType)) && (! listutils::isRectangle(rtreeKeyType))){
        ErrorReporter::ReportError(msg + " (rtree not over a spatial attribute)");
        return listutils::typeError();
    }
    ListExpr rtreeTupleDescription = nl->Second(rtree);
    if(! nl->Equal(relTupleDescription, rtreeTupleDescription)){
        ErrorReporter::ReportError(msg + " (type of rtree and relation are different!)");
        return listutils::typeError();
    }

    //
    if(! nl->IsEqual(rect, Rectangle<3>::BasicType()))
    {
        ErrorReporter::ReportError(msg + " (second args is not a mpoint)");
        return nl->TypeError();
    }
    /*
    cout<<nl->ToString(rel)<<endl;
    cout<<nl->ToString(rtree)<<endl;
    cout<<nl->ToString(mpoint)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    cout<<nl->ToString(attrindex)<<endl;
    */
    if(! (nl->IsEqual(dist1, CcReal::BasicType()))){
        ErrorReporter::ReportError(msg + " (fourth args is not real)");
        return nl->TypeError();
    }
    if(nl->IsEqual(dist2, CcReal::BasicType())){     // 0.0 < d2 < inf
        flag = 1;
    }else if(nl->IsEqual(dist2, "inf") or nl->IsEqual(dist2, "INF")){       // d2 = inf
        flag = 2;
    }else{                                             // type error
        ErrorReporter::ReportError(msg + " (fifth args is not real or inf)");
        return nl->TypeError();
    }

    // construct the result type ListExpr
    ListExpr resType = nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(rel));
    //cout<<"In CDistRangeRTree Type Map: END!"<<endl;
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->OneElemList(nl->IntAtom(flag)), resType);
}

class RTreeFilterLocalInfo{
public:
    RTreeVisitor rv;
    Relation     *rel;

    RTreeFilterLocalInfo(){
        rel = NULL;
    }
};

//value map function
//rel (tuple([a1:d1, ..., an:dn])) x rtree x rect3 x real(dist1) x real(dist2)-> stream (tuple (a1:d1, ..., an:dn)
int RTreeFilterVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    int                flag = 0, condition = 0;
    double             dist1, dist2;
    Rectangle<3>       *rect3;
    FilterResult       fr;
    Tuple              *tuple;

    R_Tree<3, TupleId>   *rtree;
    RTreeFilterLocalInfo *localinfo;
    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (RTreeFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new RTreeFilterLocalInfo();
            // get arguments
            localinfo->rel = (Relation *)(args[0].addr);
            rtree = (R_Tree<3, TupleId> *)(args[1].addr);
            localinfo->rv.setRTree(rtree);
            rect3 = (Rectangle<3> *)(args[2].addr);
            localinfo->rv.setRect(*rect3);
            dist1 = ((CcReal *)(args[3].addr))->GetValue();
            
            // deal with the condition
            flag = ((CcInt*)(args[5].addr))->GetValue();
            if(flag == 1){
                dist2 = ((CcReal *)(args[4].addr))->GetValue();
                if(dist1 < 0.0 || dist1 >= dist2){
                    cout<<"Error: dist1 less than 0.0 or dist2 less than dist1!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 1;
                }else{
                    condition = 4;
                }
            }else if(flag == 2){
                dist2 = -1.0;
                if(dist1 < 0.0){
                    cout<<"Error: dist1 less than 0.0!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 3;
                }else{
                    condition = 2;
                }
            }else{
                cout<<"Error: dist2 error!"<<endl;
                return CANCEL;
            }

            // set the rtree visitor
            localinfo->rv.setDistance(condition, dist1, dist2);

            if(! localinfo->rv.Init()){
                cout<<"rtree visitor initial failed!"<<endl;
                return CANCEL;
            }
            // set the local information
            local = SetWord(localinfo);
            return 0;
            
        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null"<<endl;
                return CANCEL;
            }
            localinfo = (RTreeFilterLocalInfo *)local.addr;
            
            // get a next rtree filter result
            while(localinfo->rv.hasNext()){
                if(localinfo->rv.getNext(fr)){
                    tuple = localinfo->rel->GetTuple(fr.tid, true);
                    result.setAddr(tuple);
                    return YIELD;
                }
            }
            return CANCEL;
            
        case CLOSE:
            if(local.addr){
                localinfo = (RTreeFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct RTreeFilterInfo : OperatorInfo {
    RTreeFilterInfo()
    {
        name      = "rtreefilter";
        signature = "((rel (tuple([a1:d1, ..., an:dn]))) x rtree x rect3 x real(dist1) x real(dist2) -> stream (tuple (a1:d1, ..., an:dn))";
        syntax    = "rtreefilter (_, _, _, _, _)";
        meaning   = "using rtree to filter the relation";
    }
};

/****************************************************************

    3.operator TBTreeFilter

***************************************************************/

// type map function
//rel (tuple([a1:d1, ..., an:dn])) x tbtree x rect3 x real(dist1) x real(dist2)-> stream (tuple (a1:d1, ..., an:dn)
ListExpr TBTreeFilterTM(ListExpr args)
{
    int    flag = 0;     //1: dist2 is a real, 2: dist2 is a inf, 0: error
    
    //error message;
    string msg = "rel x tbtree x rect3 x real(dist1) x real(dist2) expected";
    //the number of args is 5: stream x mpoint x double x double x int
    if(nl->ListLength(args) != 5)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr rel = nl->First(args);
    ListExpr tbt = nl->Second(args);
    ListExpr rect = nl->Third(args);
    ListExpr dist1 = nl->Fourth(args);
    ListExpr dist2 = nl->Fifth(args);
    
    // check the relation
    if(! listutils::isRelDescription(rel))
    {
        ErrorReporter::ReportError(msg + " (first args is not a relation)");
        return listutils::typeError();
    }
    //ListExpr relTupleDescription = nl->Second(rel);
    // check the tbtree
    if(! tbtree::TBTree::checkType(tbt)){
        ErrorReporter::ReportError(msg + " (second args is not a tbtree)");
        return listutils::typeError();
    }
    /*
    ListExpr rtreeKeyType = listutils::getRTreeType(rtree);
    if((! listutils::isSpatialType(rtreeKeyType)) && (! listutils::isRectangle(rtreeKeyType))){
        ErrorReporter::ReportError(msg + " (rtree not over a spatial attribute)");
        return listutils::typeError();
    }
    ListExpr rtreeTupleDescription = nl->Second(rtree);
    if(! nl->Equal(relTupleDescription, rtreeTupleDescription)){
        ErrorReporter::ReportError(msg + " (type of rtree and relation are different!)");
        return listutils::typeError();
    }
    */

    //
    if(! nl->IsEqual(rect, Rectangle<3>::BasicType()))
    {
        ErrorReporter::ReportError(msg + " (second args is not a mpoint)");
        return nl->TypeError();
    }
    /*
    cout<<nl->ToString(rel)<<endl;
    cout<<nl->ToString(tbt)<<endl;
    cout<<nl->ToString(mpoint)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    cout<<nl->ToString(attrindex)<<endl;
    */
    if(! (nl->IsEqual(dist1, CcReal::BasicType()))){
        ErrorReporter::ReportError(msg + " (fourth args is not real)");
        return nl->TypeError();
    }
    if(nl->IsEqual(dist2, CcReal::BasicType())){     // 0.0 < d2 < inf
        flag = 1;
    }else if(nl->IsEqual(dist2, "inf") or nl->IsEqual(dist2, "INF")){       // d2 = inf
        flag = 2;
    }else{                                             // type error
        ErrorReporter::ReportError(msg + " (fifth args is not real or inf)");
        return nl->TypeError();
    }

    // construct the result type ListExpr
    ListExpr resType = nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(rel));
    //cout<<"In CDistRangeTBTree Type Map: END!"<<endl;
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->OneElemList(nl->IntAtom(flag)), resType);
}

class TBTreeFilterLocalInfo{
public:
    TBTreeVisitor tbtreev;
    Relation      *rel;

    TBTreeFilterLocalInfo(){
        rel = NULL;
    }
};

//value map function
//rel (tuple([a1:d1, ..., an:dn])) x tbtree x rect3 x real(dist1) x real(dist2)-> stream (tuple (a1:d1, ..., an:dn)
int TBTreeFilterVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    int                flag = 0, condition = 0;
    double             dist1, dist2;
    Rectangle<3>       *rect3;
    FilterResult       fr;
    Tuple              *tuple;
    tbtree::TBTree     *tbt;
    TBTreeFilterLocalInfo *localinfo;
    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (TBTreeFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new TBTreeFilterLocalInfo();
            // get arguments
            localinfo->rel = (Relation *)(args[0].addr);
            tbt = (tbtree::TBTree *)(args[1].addr);
            localinfo->tbtreev.setTBTree(tbt);
            rect3 = (Rectangle<3> *)(args[2].addr);
            localinfo->tbtreev.setRect(*rect3);
            dist1 = ((CcReal *)(args[3].addr))->GetValue();
            
            // deal with the condition
            flag = ((CcInt*)(args[5].addr))->GetValue();
            if(flag == 1){
                dist2 = ((CcReal *)(args[4].addr))->GetValue();
                if(dist1 < 0.0 || dist1 >= dist2){
                    cout<<"Error: dist1 less than 0.0 or dist2 less than dist1!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 1;
                }else{
                    condition = 4;
                }
            }else if(flag == 2){
                dist2 = -1.0;
                if(dist1 < 0.0){
                    cout<<"Error: dist1 less than 0.0!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 3;
                }else{
                    condition = 2;
                }
            }else{
                cout<<"Error: dist2 error!"<<endl;
                return CANCEL;
            }

            // set the tbtree visitor
            localinfo->tbtreev.setDistance(condition, dist1, dist2);

            if(! localinfo->tbtreev.Init()){
                cout<<"tbtree visitor initial failed!"<<endl;
                return CANCEL;
            }
            // set the local information
            local = SetWord(localinfo);
            return 0;
            
        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null"<<endl;
                return CANCEL;
            }
            localinfo = (TBTreeFilterLocalInfo *)local.addr;
            
            // get a next tbtree filter result
            while(localinfo->tbtreev.hasNext()){
                if(localinfo->tbtreev.getNext(fr)){
                    tuple = localinfo->rel->GetTuple(fr.tid, true);
                    result.setAddr(tuple);
                    return YIELD;
                }
            }
            return CANCEL;
            
        case CLOSE:
            if(local.addr){
                localinfo = (TBTreeFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct TBTreeFilterInfo : OperatorInfo {
    TBTreeFilterInfo()
    {
        name      = "tbtreefilter";
        signature = "((rel (tuple([a1:d1, ..., an:dn]))) x tbtree x rect3 x real(dist1) x real(dist2) -> stream (tuple (a1:d1, ..., an:dn))";
        syntax    = "tbtreefilter (_, _, _, _, _)";
        meaning   = "using tbtree to filter the relation";
    }
};

/****************************************************************

    4.operator SETIFilter

***************************************************************/

// type map function
// seti x rect3 x real(dist1) x real(dist2)-> stream (tuple (int:Id, UPoint:UTrip)
ListExpr SETIFilterTM(ListExpr args)
{
    int    flag = 0;     //1: dist2 is a real, 2: dist2 is a inf, 0: error
    
    //error message;
    string msg = "rel x seti x rect3 x real(dist1) x real(dist2) expected";
    //the number of args is 4: seti x rect2 x double x double
    if(nl->ListLength(args) != 4)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr seti = nl->First(args);
    ListExpr rect = nl->Second(args);
    ListExpr dist1 = nl->Third(args);
    ListExpr dist2 = nl->Fourth(args);
    
    // check the seti
    if(! SETI::checkType(seti))
    {
        ErrorReporter::ReportError(msg + " (first args is not a seti)");
        return listutils::typeError();
    }
    // check the rectangle
    if(! nl->IsEqual(rect, Rectangle<3>::BasicType()))
    {
        ErrorReporter::ReportError(msg + " (second args is not a rectangle)");
        return nl->TypeError();
    }
    /*
    cout<<nl->ToString(seti)<<endl;
    cout<<nl->ToString(rect)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    */
    if(! (nl->IsEqual(dist1, CcReal::BasicType()))){
        ErrorReporter::ReportError(msg + " (fourth args is not real)");
        return nl->TypeError();
    }
    if(nl->IsEqual(dist2, CcReal::BasicType())){     // 0.0 < d2 < inf
        flag = 1;
    }else if(nl->IsEqual(dist2, "inf") or nl->IsEqual(dist2, "INF")){       // d2 = inf
        flag = 2;
    }else{                                             // type error
        ErrorReporter::ReportError(msg + " (fifth args is not real or inf)");
        return nl->TypeError();
    }

    // construct the result type ListExpr
    ListExpr resType = nl->TwoElemList(
            nl->SymbolAtom("stream"), 
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("UTrip"),
                        nl->SymbolAtom("upoint")))));
    //cout<<nl->ToString(resType)<<endl;
    //cout<<"In CDistRange SETI Type Map: END!"<<endl;
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->OneElemList(nl->IntAtom(flag)), resType);
}

class SETIFilterLocalInfo{
public:
    SETIVisitor setiv;
    ListExpr    tupletype;
    SETIFilterLocalInfo(){
    }
};

//value map function
//seti x rect3 x real(dist1) x real(dist2)-> stream (tuple (a1:d1, ..., an:dn)
int SETIFilterVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    int                 flag = 0, condition = 0;
    double              dist1, dist2;
    Rectangle<3>        *rect3;
    TrjSeg              *trajseg;
    Tuple               *tuple;
    SETI                *seti;
    SETIFilterLocalInfo *localinfo;
    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (SETIFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new SETIFilterLocalInfo();
            // get arguments
            seti = (SETI *)(args[0].addr);
            localinfo->setiv.setSETI(seti);
            rect3 = (Rectangle<3> *)(args[1].addr);
            localinfo->setiv.setRect(*rect3);
            dist1 = ((CcReal *)(args[2].addr))->GetValue();
            
            // deal with the condition
            flag = ((CcInt*)(args[4].addr))->GetValue();
            if(flag == 1){
                dist2 = ((CcReal *)(args[3].addr))->GetValue();
                if(dist1 < 0.0 || dist1 >= dist2){
                    cout<<"Error: dist1 less than 0.0 or dist2 less than dist1!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 1;
                }else{
                    condition = 4;
                }
            }else if(flag == 2){
                dist2 = -1.0;
                if(dist1 < 0.0){
                    cout<<"Error: dist1 less than 0.0!"<<endl;
                    return CANCEL;
                }else if(dist1 == 0.0){
                    condition = 3;
                }else{
                    condition = 2;
                }
            }else{
                cout<<"Error: dist2 error!"<<endl;
                return CANCEL;
            }

            // set the tbtree visitor
            localinfo->setiv.setDistance(condition, dist1, dist2);

            if(! localinfo->setiv.Init()){
                cout<<"tbtree visitor initial failed!"<<endl;
                return CANCEL;
            }
            localinfo->tupletype = nl->Second(GetTupleResultType(s));
            // set the local information
            local = SetWord(localinfo);
            return 0;
            
        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null"<<endl;
                return CANCEL;
            }
            localinfo = (SETIFilterLocalInfo *)local.addr;
            
            // get a next seti filter result
            while(localinfo->setiv.hasNext()){
                trajseg = localinfo->setiv.getNext();
                if(trajseg == NULL){
                    return CANCEL;
                }
                tuple = new Tuple(localinfo->tupletype);
                tuple->PutAttribute(0, new CcInt(trajseg->moID));
                tuple->PutAttribute(1, getUPointFromTrjSeg(*trajseg));
                delete trajseg;
                result.setAddr(tuple);
                return YIELD;
            }
            return CANCEL;
            
        case CLOSE:
            if(local.addr){
                localinfo = (SETIFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct SETIFilterInfo : OperatorInfo {
    SETIFilterInfo()
    {
        name      = "setifilter";
        signature = "seti x rect3 x real(dist1) x real(dist2) -> stream (tuple (a1:d1, ..., an:dn))";
        syntax    = "setifilter (_, _, _, _)";
        meaning   = "using seti to filter the relation";
    }
};

/****************************************************************

    5.operator BLUpdateRelRTree

***************************************************************/
// type map function
// stream(tuple(mpoint:Trip)) x rel x int(index of MBR)-> rtree
ListExpr UpdateRelRTreeTM(ListExpr args)
{
    //error message;
    string msg = "stream(tuple(mpoint:Trip)) x rel x S(index of mpoint) expected";
    //the number of args is 3: stream x rel x int
    if(nl->ListLength(args) != 3)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr rel = nl->Second(args);
    ListExpr attrindex = nl->Third(args);
    
    // check the stream
    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg + " (first args is not a stream)");
        return listutils::typeError();
    }   
    // check the rel
    if(! listutils::isRelDescription(rel))
    {
        ErrorReporter::ReportError(msg + " (second args is not a relation)");
        return listutils::typeError();
    }
    // check the stream tuple and rel tuple
    if(! nl->Equal(nl->Second(stream), nl->Second(rel))){
        ErrorReporter::ReportError(msg + " (stream and relation has different tuple type)");
        return listutils::typeError();
    }
    /*
    cout<<nl->ToString(seti)<<endl;
    cout<<nl->ToString(rect)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    */
    // check the attribute index
    string   attrname = nl->SymbolValue(attrindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != Rectangle<3u>::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    // construct the result type ListExpr
    ListExpr resType = nl->FourElemList(
            nl->SymbolAtom(R_Tree<3, TupleId>::BasicType()),
            nl->Second(stream),   // tuple type
            attrtype,
            nl->BoolAtom(false)); 
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->OneElemList(nl->IntAtom(index)), resType);
}

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int BLUpdateRelRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    int                 attrindex, counter = 0;
    R_Tree<3, TupleId>  *rtree;
    Rectangle<3>       *box;
    TupleId             tid;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    attrindex = ((CcInt *)(args[3].addr))->GetValue()-1;
    rtree = (R_Tree<3, TupleId>*)qp->ResultStorage(s).addr;
    result.setAddr(rtree);
    stream->open();
    // init the rtree bulk load
    bool bulkloadinitialized = rtree->InitializeBulkLoad();
    assert(bulkloadinitialized);
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        //mpoint = (MPoint *)tnew->GetAttribute(attrindex);
        tid = tnew->GetTupleId();
        //rect = mpoint->BoundingBox();
        box = (Rectangle<3> *)tnew->GetAttribute(attrindex);
        if(box->IsDefined() && tid != 0){
            R_TreeLeafEntry<3, TupleId> le(*box, tid);
            rtree->InsertBulkLoad(le);
        }
        told->DeleteIfAllowed();
    }
    bool finalizebulkload = rtree->FinalizeBulkLoad();
    assert(finalizebulkload);
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    return 0;
}

//
// operator info
struct BLUpdateRelRTreeInfo : OperatorInfo {
    BLUpdateRelRTreeInfo()
    {
        name      = "blupdaterelrtree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x int(index of mpoint) -> rtree";
        syntax    = "_ blupdaterelrtree [ _, _]";
        meaning   = "update the stream tuple to rel, and generate a new sub rtree of stream tuple";
    }
};

/****************************************************************

    6.operator MergeRTree

***************************************************************/
// type map function
// rtree x rtree -> rtree
ListExpr MergeRTreeTM(ListExpr args)
{
    //error message;
    string msg = "rtree x rtree expected";
    //the number of args is 2: rtree
    if(nl->ListLength(args) != 2)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr rtree1 = nl->First(args);
    ListExpr rtree2 = nl->Second(args);
    ListExpr rtreeKeyType;
    
    // check the rtree
    if(! listutils::isRTreeDescription(rtree1)){
        ErrorReporter::ReportError(msg + " (first args is not a rtree)");
        return listutils::typeError();
    }
    rtreeKeyType = listutils::getRTreeType(rtree1);
    if((! listutils::isSpatialType(rtreeKeyType)) && (! listutils::isRectangle(rtreeKeyType))){
        ErrorReporter::ReportError(msg + " (first rtree not over a spatial attribute)");
        return listutils::typeError();
    }
    if(! listutils::isRTreeDescription(rtree2)){
        ErrorReporter::ReportError(msg + " (second args is not a rtree)");
        return listutils::typeError();
    }
    rtreeKeyType = listutils::getRTreeType(rtree2);
    if((! listutils::isSpatialType(rtreeKeyType)) && (! listutils::isRectangle(rtreeKeyType))){
        ErrorReporter::ReportError(msg + " (second rtree not over a spatial attribute)");
        return listutils::typeError();
    }
    if(! nl->Equal(rtree1, rtree2)){
        ErrorReporter::ReportError(msg + " (type of rtree1 and rtree2 are different!)");
        return listutils::typeError();
    }
    /*
    cout<<nl->ToString(rtree1)<<endl;
    cout<<nl->ToString(rtree2)<<endl;
    */
    return rtree1;
}

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int MergeRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    R_Tree<3, TupleId>  *mrtree, *srtree, *tmp;  // master rtree and sub rtree
    mrtree = (R_Tree<3, TupleId> *)args[0].addr;
    srtree = (R_Tree<3, TupleId> *)args[1].addr;
    result.setAddr(mrtree);
    if(mrtree->FileId() == srtree->FileId()){
        // mrtree and srtree in the same file
        mrtree->SwitchHeader(srtree);
    }
    else{
        // mrtree and srtree in different file
        // create a new rtree in file of mrtree
        // mrtree->CloseFile();
        tmp = new R_Tree<3, TupleId>(mrtree->FileId(), 4000);
        tmp->Clone(srtree);
        mrtree->SwitchHeader(tmp);
    }
    // merge the mrtree and srtree in file of mrtree, and return mrtree
    mrtree->MergeRtree();
    return 0;
}

//
// operator info
struct MergeRTreeInfo : OperatorInfo {
    MergeRTreeInfo()
    {
        name      = "mergertree";
        signature = "rtree x rtree -> rtree";
        syntax    = "mergertree ( _, _ )";
        meaning   = "merge master rtree and sub rtree to the file master rtree and return the master rtree";
    }
};

/****************************************************************

    7.operator StreamUpdateRTree

***************************************************************/
// type map function
// stream(tuple(mpoint:Trip)) x rel x rtree x attr(index of MBR)-> rtree
ListExpr StreamUpdateRTreeTM(ListExpr args)
{
    //error message;
    string msg = "stream(tuple(mpoint:Trip)) x rel x S(index of mpoint) expected";
    //the number of args is 4: stream x rel x rtree x attr
    if(nl->ListLength(args) != 4)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr rel = nl->Second(args);
    ListExpr rtree = nl->Third(args);
    ListExpr attrindex = nl->Fourth(args);
    /*
    cout<<nl->ToString(stream)<<endl;
    cout<<nl->ToString(rel)<<endl;
    cout<<nl->ToString(rtree)<<endl;
    cout<<nl->ToString(attrindex)<<endl;
    */
    // check the stream
    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg + " (first args is not a stream)");
        return listutils::typeError();
    }   
    // check the rel
    if(! listutils::isRelDescription(rel))
    {
        ErrorReporter::ReportError(msg + " (second args is not a relation)");
        return listutils::typeError();
    }
    // check the rtree
    if(! listutils::isRTreeDescription(rtree)){
        ErrorReporter::ReportError(msg + " (third args is not a rtree)");
        return listutils::typeError();
    }
    ListExpr rtreeKeyType = listutils::getRTreeType(rtree);
    if((! listutils::isSpatialType(rtreeKeyType)) && (! listutils::isRectangle(rtreeKeyType))){
        ErrorReporter::ReportError(msg + " (third rtree not over a spatial attribute)");
        return listutils::typeError();
    }
    // check the stream tuple and rel tuple
    if(! nl->Equal(nl->Second(stream), nl->Second(rel))){
        ErrorReporter::ReportError(msg + " (stream and relation has different tuple type)");
        return listutils::typeError();
    }
    // check the rtree and rel tuple
    if(! nl->Equal(nl->Second(rtree), nl->Second(rel))){
        ErrorReporter::ReportError(msg + " (rtree and relation has different tuple type)");
        return listutils::typeError();
    }
    // check the attribute index
    string   attrname = nl->SymbolValue(attrindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != Rectangle<3u>::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    // construct the result type ListExpr
    ListExpr resType = nl->SymbolAtom(CcBool::BasicType());
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->OneElemList(nl->IntAtom(index)), resType);
}

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int StreamBLUpdateRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    int                 attrindex, counter = 0;
    R_Tree<3, TupleId>  *rtree, *orginrtree;
    Rectangle<3>       *box;
    TupleId             tid;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    orginrtree = (R_Tree<3, TupleId> *)args[2].addr;
    attrindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    stream->open();
    rtree = new R_Tree<3, TupleId>(orginrtree->FileId(), 4000);
    // init the rtree bulk load
    bool bulkloadinitialized = rtree->InitializeBulkLoad();
    assert(bulkloadinitialized);
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        //mpoint = (MPoint *)tnew->GetAttribute(attrindex);
        tid = tnew->GetTupleId();
        //rect = mpoint->BoundingBox();
        box = (Rectangle<3> *)tnew->GetAttribute(attrindex);
        if(box->IsDefined() && tid != 0){
            R_TreeLeafEntry<3, TupleId> le(*box, tid);
            rtree->InsertBulkLoad(le);
        }
        told->DeleteIfAllowed();
    }
    bool finalizebulkload = rtree->FinalizeBulkLoad();
    assert(finalizebulkload);
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    // merge rtree to ogrin rtree
    orginrtree->SwitchHeader(rtree);
    orginrtree->MergeRtree();
    result.setAddr(new CcBool(true, true));
    return 0;
}

//
// operator info
struct StreamBLUpdateRTreeInfo : OperatorInfo {
    StreamBLUpdateRTreeInfo()
    {
        name      = "streamblupdatertree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x rtree x attr -> rtree";
        syntax    = "_ streamblupdatertree [ _, _, _]";
        meaning   = "update the stream tuple to rel, and generate a new sub rtree of stream tuple, and merge to orgin rtree";
    }
};

/****************************************************************

    7.operator StreamUpdateTBTree

***************************************************************/
// type map function
// stream(tuple(int:Id,upoint:UTrip)) x rel x tbtree -> tbtree
ListExpr StreamUpdateTBTreeTM(ListExpr args)
{
    //error message;
    string msg = "stream(tuple(int:Id, mpoint:Trip)) x rel x tbtree expected";
    //the number of args is 5: stream x rel x tbtree
    if(nl->ListLength(args) != 3)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr rel = nl->Second(args);
    ListExpr tbt = nl->Third(args);   // tbtree ((Id int)(UTrip upoint)(MBR rect3)(TID tid)) Id UTrip
    /*
    cout<<nl->ToString(stream)<<endl;
    cout<<nl->ToString(rel)<<endl;
    cout<<nl->ToString(tbt)<<endl;
    */
    // check the stream
    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg + " (first args is not a stream)");
        return listutils::typeError();
    }   
    // check the rel
    if(! listutils::isRelDescription(rel))
    {
        ErrorReporter::ReportError(msg + " (second args is not a relation)");
        return listutils::typeError();
    }
    // check the TBTree
    if(! tbtree::TBTree::checkType(tbt)){
        ErrorReporter::ReportError(msg + " (third args is not a TBTree)");
        return listutils::typeError();
    }
    // check the stream tuple and rel tuple
    if(! nl->Equal(nl->Second(stream), nl->Second(rel))){
        ErrorReporter::ReportError(msg + " (stream and relation has different tuple type)");
        return listutils::typeError();
    }
    // check the TBTree and rel tuple
    if(! nl->Equal(nl->Second(tbt), nl->Second(nl->Second(rel)))){
        ErrorReporter::ReportError(msg + " (tbtree and relation has different tuple type)");
        return listutils::typeError();
    }
    // check the attribute index
    string   attrname = nl->SymbolValue(nl->Third(tbt));  // Id: nl->Third(tbt)
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != CcInt::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    int idindex = index;
    attrname = nl->SymbolValue(nl->Fourth(tbt));   // UTrip: nl->Fouth(tbt)
    attrtype = nl->Empty();
    index = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != UPoint::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    int upointindex = index;
    attrname = "TID";
    attrtype = nl->Empty();
    index = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != TupleIdentifier::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    int tidindex = index;
    // construct the result type ListExpr
    ListExpr resType = tbt;
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->ThreeElemList(nl->IntAtom(idindex), nl->IntAtom(upointindex), nl->IntAtom(tidindex)), resType);
}

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x tbtree -> tbtree
int StreamUpdateTBTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    UPoint              *upoint;
    int                 id, idindex, upointindex, tidindex, counter = 0;
    tbtree::TBTree      *tbt;
    TupleId             tid;
    vector<int>         cIndex;
    vector<Attribute *> newAttr;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    tbt = (tbtree::TBTree *)args[2].addr;
    idindex = ((CcInt *)(args[3].addr))->GetValue()-1;
    upointindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    tidindex = ((CcInt *)(args[5].addr))->GetValue()-1;
    stream->open();
    // push the change index of tuple of tid to cIndexs;
    cIndex.push_back(tidindex);
    // insert the trajectory into rel
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        tid = tnew->GetTupleId();
        // push the new attribute into newAttr
        newAttr.clear();
        newAttr.push_back(new TupleIdentifier(true, tid));
        // update the rel with tuple tnew
        rel->UpdateTuple(tnew, cIndex, newAttr);
        // construct the leaf entry and insert into tbtree
        id = ((CcInt *)told->GetAttribute(idindex))->GetValue();
        upoint = (UPoint *)told->GetAttribute(upointindex);
        tbt->insert(*upoint, id, tid);
        told->DeleteIfAllowed();
    }
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    result.setAddr(tbt);
    return 0;
}

//
// operator info
struct StreamUpdateTBTreeInfo : OperatorInfo {
    StreamUpdateTBTreeInfo()
    {
        name      = "streamupdatetbtree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x tbtree -> tbtree";
        syntax    = "_ streamupdatetbtree [ _, _ ]";
        meaning   = "update the stream tuple to rel, and insert the new entry to tbtree";
    }
};


/****************************************************************

    8.operator StreamUpdateBLTBTree

***************************************************************/
//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x tbtree -> tbtree
int StreamUpdateBLTBTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    UPoint              *upoint;
    int                 id, idindex, upointindex, tidindex, counter = 0;
    tbtree::TBTree      *tbt;
    TupleId             tid;
    vector<int>         cIndex;
    vector<Attribute *> newAttr;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    tbt = (tbtree::TBTree *)args[2].addr;
    idindex = ((CcInt *)(args[3].addr))->GetValue()-1;
    upointindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    tidindex = ((CcInt *)(args[5].addr))->GetValue()-1;
    stream->open();
    tbt->startBulkLoad();
    // push the change index of tuple of tid to cIndexs;
    cIndex.push_back(tidindex);
    // insert the trajectory into rel
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        tid = tnew->GetTupleId();
        // push the new attribute into newAttr
        newAttr.clear();
        newAttr.push_back(new TupleIdentifier(true, tid));
        // update the rel with tuple tnew
        rel->UpdateTuple(tnew, cIndex, newAttr);
        // construct the leaf entry and insert into tbtree
        id = ((CcInt *)told->GetAttribute(idindex))->GetValue();
        upoint = (UPoint *)told->GetAttribute(upointindex);
        tbt->bulkLoadInsert(*upoint, id, tid);
        told->DeleteIfAllowed();
    }
    tbt->endBulkLoad();
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    result.setAddr(tbt);
    return 0;
}

//
// operator info
struct StreamUpdateBLTBTreeInfo : OperatorInfo {
    StreamUpdateBLTBTreeInfo()
    {
        name      = "streamupdatebltbtree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x tbtree -> tbtree";
        syntax    = "_ streamupdatebltbtree [ _, _ ]";
        meaning   = "update the stream tuple to rel, and insert the new entry to tbtree using bulk load";
    }
};


/****************************************************************

    10.operator CDistRangeMM

***************************************************************/

// type map function
//
ListExpr CDistRangeMMTM(ListExpr args)
{
    int    flag = 0;     //1: dist2 is a real, 2: dist2 is a inf, 0: error
    
    //error message;
    string msg = "stream x mpoint x real(dist1) x real(dist2) x int(attrindex) expected";
    //the number of args is 5: stream x mpoint x double x double x int
    if(nl->ListLength(args) != 5)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr mpoint = nl->Second(args);
    ListExpr dist1 = nl->Third(args);
    ListExpr dist2 = nl->Fourth(args);
    ListExpr attrindex = nl->Fifth(args);

    if(! listutils::isStream(stream))
    {
        ErrorReporter::ReportError(msg + " (first args is not a relation)");
        return listutils::typeError();
    }
    if(! nl->IsEqual(mpoint, MPoint::BasicType()))
    {
        ErrorReporter::ReportError(msg + " (second args is not a mpoint)");
        return nl->TypeError();
    }
    /*
    cout<<nl->ToString(stream)<<endl;
    cout<<nl->ToString(mpoint)<<endl;
    cout<<nl->ToString(dist1)<<endl;
    cout<<nl->ToString(dist2)<<endl;
    cout<<nl->ToString(attrindex)<<endl;
    */
    if(! (nl->IsEqual(dist1, CcReal::BasicType()))){
        ErrorReporter::ReportError(msg + " (third args is not real)");
        return nl->TypeError();
    }
    if(nl->IsEqual(dist2, CcReal::BasicType())){     // 0.0 < d2 < inf
        flag = 1;
    }else if(nl->IsEqual(dist2, "inf") or nl->IsEqual(dist2, "INF")){       // d2 = inf
        flag = 2;
    }else{                                             // type error
        ErrorReporter::ReportError(msg + " (fourth args is not real or inf)");
        return nl->TypeError();
    }

    // check the attribute index
    string   attrname = nl->SymbolValue(attrindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != MPoint::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type mpoint.");
        }
    }
    // construct the result type ListExpr
    //ListExpr resType = stream;
    //return NList( NList(Symbol::APPEND()), NList(j).enclose(), resType ).listExpr();
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->TwoElemList(nl->IntAtom(index), nl->IntAtom(flag)), stream);
}

//      1: d1 = 0.0,  0.0 < d2 < inf
//      2: 0.0 < d1 < inf, d2 = inf
//      3: d1 = 0.0, d2 = inf
//      4: 0.0 < d1 < inf, 0.0 < d2 < inf
//



//value map function
// stream x mpoint x real(dist1) x real(dist2) x string(attrindex) x int(attrindex) x int(condition)
int CDistRangeMMVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    MPoint *mp = NULL, *mpoint = NULL;
    Tuple  *tuple = NULL;
    int    i,
           noattr = 0,
           flag = 0;

    // local information
    CDistRangeMMLocalInfo *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (CDistRangeMMLocalInfo *)(local.addr);
                delete localinfo;
            }
            localinfo = new CDistRangeMMLocalInfo();

            // get the arguments
            //
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            localinfo->mpoint = (MPoint *)(args[1].addr);
            localinfo->dist1 = ((CcReal *)(args[2].addr))->GetValue();
            localinfo->attrindex = ((CcInt *)(args[5].addr))->GetValue()-1;
            
            // deal with condition
            flag = ((CcInt *)(args[6].addr))->GetValue();
            if(flag == 1){
                localinfo->dist2 = ((CcReal *)(args[3].addr))->GetValue();
                if(localinfo->dist1 < 0.0 || localinfo->dist1 >= localinfo->dist2){
                    cout<<"Error: dist1 less than 0.0 or dist2 less than dist1!"<<endl;
                    return CANCEL;
                }else if(localinfo->dist1 == 0.0){
                    localinfo->condition = 1;
                }else{
                    localinfo->condition = 4;
                }
            }else if(flag == 2){
                localinfo->dist2 = -1.0;
                if(localinfo->dist1 < 0.0){
                    cout<<"Error: dist1 less than 0.0!"<<endl;
                    return CANCEL;
                }else if(localinfo->dist1 == 0.0){
                    localinfo->condition = 3;
                }else{
                    localinfo->condition = 2;
                }
            }else{
                cout<<"Error: dist2 error!"<<endl;
                return CANCEL;
            }

            localinfo->tupletype = nl->Second(GetTupleResultType(s));
            // open the stream
            //
            localinfo->stream->open();

            // set the local informantion
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null"<<endl;
                return CANCEL;
            }
            localinfo = (CDistRangeMMLocalInfo *)local.addr;

            mpoint = new MPoint(0);

            while((localinfo->tuple = localinfo->stream->request()) != NULL){
                // get next tuple from stream
                mp = (MPoint *)(localinfo->tuple->GetAttribute(localinfo->attrindex));
                // computer the result mpoints
                if(1 == MMCDistRange(*mp, *localinfo->mpoint, localinfo->dist1, localinfo->dist2, *mpoint, localinfo->condition)){
                    // deal with the result upoint, construct the tuple with result upoint and add to result stream
                    tuple = new Tuple(localinfo->tupletype);
                    // copy the data to new tuple
                    noattr = localinfo->tuple->GetNoAttributes();
                    for(i = 0; i < noattr; i ++){
                        if(i != localinfo->attrindex){
                            tuple->CopyAttribute(i, localinfo->tuple, i);
                        }else{
                            tuple->PutAttribute(i, (Attribute*)mpoint);
                        }
                    }
                    result.setAddr(tuple);
                    return YIELD;
                }
                delete mp;
            }
            delete mpoint;
            // There is no tuple to deal with
            //cout<<"There is no tuple to deal with!"<<endl;
            return CANCEL;

        case CLOSE:
            if(local.addr){
                localinfo = (CDistRangeMMLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                delete localinfo;
            }
            //cout<<"local.addr is null"<<endl;
            return 0;
    }
    return 0;
}

//
// operator info
struct CDistRangeMMInfo : OperatorInfo {
    CDistRangeMMInfo()
    {
        name      = "cdrange";
        signature = "stream(tuple([a1:d1, ..., ai:upoint, ..., an:dn])) x mpoint x double x double x ai"
                    " -> stream(tuple([a1:d1, ..., ai:upoint, ..., an:dn]))";
        syntax    = "_ cdrange [ _, _, _, _ ]";
        meaning   = "continuous distace range query between mpoint and mpoint";
    }
};

/****************************************************************

    11.Data preprocessing

***************************************************************/

/****************************************************************

    12.operator GridBLUpdateRelRTree

***************************************************************/

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int GridBLUpdateRelRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    int                 attrindex, counter = 0;
    R_Tree<3, TupleId>  *rtree;
    Rectangle<3>        *box;
    TupleId             tid;
    RTreeLevel          *rtreelevel;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    attrindex = ((CcInt *)(args[3].addr))->GetValue()-1;
    rtree = (R_Tree<3, TupleId>*)qp->ResultStorage(s).addr;
    result.setAddr(rtree);
    stream->open();
    rtreelevel = new RTreeLevel(true, rtree, 100.0, 100.0, 1.0/24/12);
    // deal with the stream
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        //mpoint = (MPoint *)tnew->GetAttribute(attrindex);
        tid = tnew->GetTupleId();
        //rect = mpoint->BoundingBox();
        box = (Rectangle<3> *)tnew->GetAttribute(attrindex);
        if(box->IsDefined() && tid != 0){
            //R_TreeLeafEntry<3, TupleId> le(*box, tid);
            //rtree->InsertBulkLoad(le);
            rtreelevel->Insert(tid, *box);
        }
        told->DeleteIfAllowed();
    }
    rtreelevel->SetRTreeRoot();
    delete rtreelevel;
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    return 0;
}

//
// operator info
struct GridBLUpdateRelRTreeInfo : OperatorInfo {
    GridBLUpdateRelRTreeInfo(){
        name      = "gridblupdaterelrtree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x int(index of mpoint) -> rtree";
        syntax    = "_ gridblupdaterelrtree [ _, _]";
        meaning   = "update the stream tuple to rel, and generate a new sub rtree of stream tuple";
    }
};

/****************************************************************

    13.operator StreamGridBLUpdateRTree

***************************************************************/
//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int StreamGridBLUpdateRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    int                 attrindex, counter = 0;
    R_Tree<3, TupleId>  *rtree, *orginrtree;
    Rectangle<3>        *box;
    TupleId             tid;
    RTreeLevel          *rtreelevel;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    orginrtree = (R_Tree<3, TupleId> *)args[2].addr;
    rtree = new R_Tree<3, TupleId>(orginrtree->FileId(), 4000);
    attrindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    stream->open();
    // init the rtree bulk load
    rtreelevel = new RTreeLevel(true, rtree, 100.0, 100.0, 1.0/24/12);
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        rel->AppendTuple(tnew);
        //mpoint = (MPoint *)tnew->GetAttribute(attrindex);
        tid = tnew->GetTupleId();
        //rect = mpoint->BoundingBox();
        box = (Rectangle<3> *)tnew->GetAttribute(attrindex);
        if(box->IsDefined() && tid != 0){
            //R_TreeLeafEntry<3, TupleId> le(*box, tid);
            //rtree->InsertBulkLoad(le);
            rtreelevel->Insert(tid, *box);
        }
        told->DeleteIfAllowed();
    }
    rtreelevel->SetRTreeRoot();
    delete rtreelevel;
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    // merge rtree to ogrin rtree
    orginrtree->SwitchHeader(rtree);
    orginrtree->MergeRtree();
    result.setAddr(new CcBool(true, true));
    return 0;
}

//
// operator info
struct StreamGridBLUpdateRTreeInfo : OperatorInfo {
    StreamGridBLUpdateRTreeInfo()
    {
        name      = "streamgridblupdatertree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x rtree x attr -> rtree";
        syntax    = "_ streamgridblupdatertree [ _, _, _]";
        meaning   = "update the stream tuple to rel, and generate a new sub rtree of stream tuple, and merge to orgin rtree";
    }
};

/****************************************************************

    14.operator StreamOBOUpdateRTree

***************************************************************/
//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int StreamOBOUpdateRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *told, *tnew;
    Relation            *rel;
    int                 attrindex, counter = 0;
    R_Tree<3, TupleId>  *rtree;
    Rectangle<3>        *box;
    TupleId             tid;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    rel = (Relation *)args[1].addr;
    rtree = (R_Tree<3, TupleId> *)args[2].addr;
    attrindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    stream->open();
    while((told = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        tnew = told->Clone();
        told->DeleteIfAllowed();
        // append the tuple to relation
        rel->AppendTuple(tnew);
        //mpoint = (MPoint *)tnew->GetAttribute(attrindex);
        tid = tnew->GetTupleId();
        //rect = mpoint->BoundingBox();
        box = (Rectangle<3> *)tnew->GetAttribute(attrindex);
        if(box->IsDefined() && tid != 0){
            //R_TreeLeafEntry<3, TupleId> le(*box, tid);
            rtree->Insert(R_TreeLeafEntry<3, TupleId>(*box, tid));
        }
    }
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    // merge rtree to ogrin rtree
    result.setAddr(new CcBool(true, true));
    return 0;
}

//
// operator info
struct StreamOBOUpdateRTreeInfo : OperatorInfo {
    StreamOBOUpdateRTreeInfo()
    {
        name      = "streamoboupdatertree";
        signature = "((stream (tuple([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x (rel(stream([a1:d1, ..., Trip:mpoint, ..., an:dn]))) x rtree x attr -> rtree";
        syntax    = "_ streamoboupdatertree [ _, _, _]";
        meaning   = "update the stream tuple to rel, and generate a new sub rtree of stream tuple, and merge to orgin rtree";
    }
};

/****************************************************************

    15.operator GridBulkLoadRTree

***************************************************************/
// type map function
// stream(tuple(MBR:rect3,TID:tid)) x attr(index of MBR) x attr(index of TID)-> rtree
ListExpr GridBulkLoadRTreeTM(ListExpr args)
{
    //error message;
    string msg = "stream(tuple(MBR:rect3,TID:tid)) x MBR x TID expected";
    //the number of args is 4: stream x rel x rtree x attr
    if(nl->ListLength(args) != 3)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr mbrindex = nl->Second(args);
    ListExpr tidindex = nl->Third(args);
    // check the stream
    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg + " (first args is not a stream)");
        return listutils::typeError();
    }   
    // check the attribute index of mbr
    string   attrname = nl->SymbolValue(mbrindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != Rectangle<3u>::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    // check the attribute index of tid
    attrname = nl->SymbolValue(tidindex);
    attrtype = nl->Empty();
    int index2 = listutils::findAttribute(attrlist, attrname, attrtype);

    if (0 == index2)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != TupleIdentifier::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type rectangle.");
        }
    }
    // construct the new attrlist, ignore the tid index
    ListExpr first, rest, newAttrList=nl->Empty(), lastAttrList;
    rest = attrlist;
    int j = 1;
    bool firstcall = true;
    while(! nl->IsEmpty(rest)){
        first = nl->First(rest);
        rest = nl->Rest(rest);
        
        if(j != index2){  // index2 is tid index
            if(firstcall){
                firstcall = false;
                newAttrList = nl->OneElemList(first);
                lastAttrList = newAttrList;
            }else{
                lastAttrList = nl->Append(lastAttrList, first);
            }
        }
        j++;
    }
    // construct the result type ListExpr
    ListExpr resType = nl->FourElemList(
            nl->SymbolAtom(R_Tree<3, TupleId>::BasicType()),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                newAttrList),
            //nl->Second(stream),   // tuple type
            nl->SymbolAtom(Rectangle<3u>::BasicType()),   // key type, spaial attribute
            //attrlist,
            nl->BoolAtom(false)); 
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), nl->TwoElemList(nl->IntAtom(index), nl->IntAtom(index2)), resType);
}

//value map function
//stream(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) x rel(tuple([a1:d1, ..., Trip:mpoint, ..., an:dn])) -> rtree
int GridBulkLoadRTreeVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Stream<Tuple>       *stream;
    Tuple               *tuple;
    int                 mbrindex, tidindex, counter = 0;
    R_Tree<3, TupleId>  *rtree;
    Rectangle<3>        *box;
    TupleId             tid;
    RTreeLevel          *rtreelevel;
    static MessageCenter *msg = MessageCenter::GetInstance();
    stream = new Stream<Tuple>(args[0].addr);
    mbrindex = ((CcInt *)(args[3].addr))->GetValue()-1;
    tidindex = ((CcInt *)(args[4].addr))->GetValue()-1;
    //cout<<"index of tid:"<<tidindex<<endl;
    stream->open();
    rtree = (R_Tree<3, TupleId> *)qp->ResultStorage(s).addr;
    result.setAddr(rtree);
    rtreelevel = new RTreeLevel(true, rtree, 100.0, 100.0, 1.0/24/12);
    // deal with the stream
    while((tuple = stream->request()) != NULL){
        if((counter++ % 10000) == 0){
            NList msgList(NList("simple"), NList(counter));
            msg->Send(msgList);
        }
        box = (Rectangle<3> *)tuple->GetAttribute(mbrindex);
        tid = ((TupleIdentifier *)tuple->GetAttribute(tidindex))->GetTid();
        //box->Print(cout);
        //cout<<"tid:"<<tid<<endl;
        if(box->IsDefined() && tid != 0){
            rtreelevel->Insert(tid, *box);
        }
        tuple->DeleteIfAllowed();
    }
    rtreelevel->SetRTreeRoot();
    delete rtreelevel;
    NList msgList(NList("simple"), NList(counter));
    msg->Send(msgList);
    return 0;
}

//
// operator info
struct GridBulkLoadRTreeInfo : OperatorInfo {
    GridBulkLoadRTreeInfo()
    {
        name      = "gridbulkloadrtree";
        signature = "((stream (tuple([a1:d1, ..., MBR:rect3, TID:tid, ..., an:dn]))) x attr x attr -> rtree";
        syntax    = "_ gridbulkloadrtree [ _, _]";
        meaning   = "bulk loading rtree using grid";
    }
};


/****************************************************************************
 
   CDistRangeAlgebra
 
 ***************************************************************************/

class CDistRangeAlgebra : public Algebra
{
public:
    CDistRangeAlgebra() : Algebra()
    {
        AddOperator( CDistRangeInfo(), CDistRangeVM, CDistRangeTM );
        AddOperator( CDistRangeMMInfo(), CDistRangeMMVM, CDistRangeMMTM );
        AddOperator( RTreeFilterInfo(), RTreeFilterVM, RTreeFilterTM );
        AddOperator( TBTreeFilterInfo(), TBTreeFilterVM, TBTreeFilterTM );
        AddOperator( SETIFilterInfo(), SETIFilterVM, SETIFilterTM );
        AddOperator( GridBLUpdateRelRTreeInfo(), GridBLUpdateRelRTreeVM, UpdateRelRTreeTM );
        AddOperator( BLUpdateRelRTreeInfo(), BLUpdateRelRTreeVM, UpdateRelRTreeTM );
        AddOperator( MergeRTreeInfo(), MergeRTreeVM, MergeRTreeTM );
        AddOperator( StreamGridBLUpdateRTreeInfo(), StreamGridBLUpdateRTreeVM, StreamUpdateRTreeTM );
        AddOperator( GridBulkLoadRTreeInfo(), GridBulkLoadRTreeVM, GridBulkLoadRTreeTM );
        AddOperator( StreamBLUpdateRTreeInfo(), StreamBLUpdateRTreeVM, StreamUpdateRTreeTM );
        AddOperator( StreamOBOUpdateRTreeInfo(), StreamOBOUpdateRTreeVM, StreamUpdateRTreeTM );
        AddOperator( StreamUpdateTBTreeInfo(), StreamUpdateTBTreeVM, StreamUpdateTBTreeTM );
        AddOperator( StreamUpdateBLTBTreeInfo(), StreamUpdateBLTBTreeVM, StreamUpdateTBTreeTM );
    }
};


/******************************************************************************

  Initialization of CDistRangeAlgebra

 ******************************************************************************/

extern "C"
    Algebra*
InitializeCDistRangeAlgebra( NestedList *nlRef,
        QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (new CDistRangeAlgebra());
}

