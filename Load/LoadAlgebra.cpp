#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "LoadAlgebra.h"
#include "Stream.h"
#include "../Spatial/Point.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../Temporal/TemporalAlgebra.h"
#include "../SETI/UploadUnit.h"

extern NestedList     *nl;
extern QueryProcessor *qp;

/****************************************************************

  1.operator LoadData

 ***************************************************************/
// Type mapping function for the operators -loaddata-
// string -> stream(tuple(Id, Datetime, Position))
ListExpr LoadDataTypeMap(ListExpr args)
{
    //error message;
    string msg = "string expected";

    if( nl->ListLength(args) != 1){
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    ListExpr filename = nl->First(args);
    if(nl->SymbolValue(filename) != "string"){
        ErrorReporter::ReportError(msg + " (first args is not a string)");
        return listutils::typeError();
    }
    return nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Datetime"),
                        nl->SymbolAtom("instant")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Position"),
                        nl->SymbolAtom("point"))
                    )));
}

// LoadData local information
class LoadDataLocalInfo
{
    public:
        RecordManager *rm;
        ListExpr      resulttype;

        LoadDataLocalInfo(){
            rm = NULL;
        }
        ~LoadDataLocalInfo(){
            if(rm){
                delete rm;
            }
        }
};

// LoadData value map function
int LoadDataValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    LoadDataLocalInfo *localinfo;
    GPSRecord         gr;
    Tuple             *tuple = NULL;
    DateTime          *dt;

    switch(message)
    {
        case OPEN:
            {
                if(! local.addr){
                    localinfo = (LoadDataLocalInfo *)local.addr;
                    delete localinfo;
                }
                localinfo = new LoadDataLocalInfo();
                string path_name  = ((CcString*)args[0].addr)->GetValue();
                // initial the record manager
                localinfo->rm = new RecordManager();
                localinfo->rm->setName(path_name.c_str());
                cout<<"file name: "<<path_name.c_str()<<endl;
                if(! localinfo->rm->Init()){
                    cout<<"record manager initial failed!"<<endl;
                    return CANCEL;
                }
                localinfo->resulttype = nl->Second(GetTupleResultType(s));
                local.setAddr(localinfo);
                return 0;
            }
        case REQUEST:
            {
                if(NULL == local.addr){
                    return CANCEL;
                }
                localinfo = (LoadDataLocalInfo *)local.addr;
                // get next record, gps record
                if(! localinfo->rm->getNextRecord(gr)){
                    cout<<"Warning: end to read record!"<<endl;
                    return CANCEL;
                }
                dt = new DateTime(instanttype);
                dt->Set(gr.yy, gr.mm, gr.dd, gr.h, gr.m, gr.s);
                tuple = new Tuple(localinfo->resulttype);
                tuple->PutAttribute(0, new CcInt(true, gr.Oid));
                tuple->PutAttribute(1, dt);
                tuple->PutAttribute(2, new Point(true, gr.lo, gr.la));
                result.setAddr(tuple);
                return YIELD;
            }
        case CLOSE:
            {
                localinfo = (LoadDataLocalInfo *)local.addr;
                delete localinfo;
                local.setAddr(Address(0));
                return 0;
            }
            return 0;
    }
    return 0;
}

// operator info of LoadData
struct LoadDataInfo : OperatorInfo {
    LoadDataInfo()
    {
        name      = "loaddata";
        signature = "string -> stream(tuple(Id, Datetime, Position))";
        syntax    = "loaddata ( _ )";
        meaning   = "load moving object trajectory data from files";
    }
};
/****************************************************************

  2.operator LoadDataFromDir

 ***************************************************************/
// Type mapping function for the operators -LoadDataFromDir-
// string -> stream(tuple(Id, Datetime, Position))
ListExpr LoadDataFromDirTypeMap(ListExpr args)
{
    //error message;
    string msg = "string x string expected";

    if( nl->ListLength(args) != 2){
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    ListExpr filename = nl->First(args);
    ListExpr extension = nl->Second(args);
    if(nl->SymbolValue(filename) != "string"){
        ErrorReporter::ReportError(msg + " (first args is not a string)");
        return listutils::typeError();
    }
    if(nl->SymbolValue(extension) != "string"){
        ErrorReporter::ReportError(msg + " (second args is not a string)");
        return listutils::typeError();
    }
    return nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Datetime"),
                        nl->SymbolAtom("instant")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Position"),
                        nl->SymbolAtom("point"))
                    )));
}

// LoadDataFromDir local information
class LoadDataFromDirLocalInfo
{
    public:
        RecordManager *rm;
        ListExpr      resulttype;

        LoadDataFromDirLocalInfo(){
            rm = NULL;
        }
        ~LoadDataFromDirLocalInfo(){
            if(rm){
                delete rm;
            }
        }
};

// LoadDataFromDir value map function
int LoadDataFromDirValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    LoadDataFromDirLocalInfo *localinfo;
    GPSRecord         gr;
    Tuple             *tuple;
    DateTime          *dt;

    switch(message)
    {
        case OPEN:
            {
                if(! local.addr){
                    localinfo = (LoadDataFromDirLocalInfo *)local.addr;
                    delete localinfo;
                }
                localinfo = new LoadDataFromDirLocalInfo();
                string path_name  = ((CcString*)args[0].addr)->GetValue();
                string file_extension  = ((CcString*)args[1].addr)->GetValue();
                // initial the record manager
                localinfo->rm = new RecordManager();
                localinfo->rm->setName(path_name.c_str());
                localinfo->rm->setExtension(file_extension.c_str());
                if(! localinfo->rm->Init()){
                    cout<<"record manager initial failed!"<<endl;
                    return CANCEL;
                }
                localinfo->resulttype = nl->Second(GetTupleResultType(s));
                local.setAddr(localinfo);
                return 0;
            }
        case REQUEST:
            {
                if(NULL == local.addr){
                    return CANCEL;
                }
                localinfo = (LoadDataFromDirLocalInfo *)local.addr;
                // get next record, gps record
                if(! localinfo->rm->getNextRecord(gr)){
                    cout<<"Warning: end to read record!"<<endl;
                    return CANCEL;
                }
                dt = new DateTime(instanttype);
                dt->Set(gr.yy, gr.mm, gr.dd, gr.h, gr.m, gr.s);
                tuple = new Tuple(localinfo->resulttype);
                tuple->PutAttribute(0, new CcInt(true, gr.Oid));
                tuple->PutAttribute(1, dt);
                tuple->PutAttribute(2, new Point(true, gr.lo, gr.la));
                result.setAddr(tuple);
                return YIELD;
            }
        case CLOSE:
            {
                localinfo = (LoadDataFromDirLocalInfo *)local.addr;
                delete localinfo;
                local.setAddr(Address(0));
                return 0;
            }
            return 0;
    }
    return 0;
}

// operator info of LoadDataFromDir
struct LoadDataFromDirInfo : OperatorInfo {
    LoadDataFromDirInfo()
    {
        name      = "loaddatafromdir";
        signature = "string x string -> stream(tuple(Id, Datetime, Position))";
        syntax    = "loaddatafromdir ( _ , _ )";
        meaning   = "load moving object trajectory data from directory";
    }
};

/****************************************************************

  3.operator LoadUploadUnit

 ***************************************************************/
//Type mapping function for the operators -loaddata-
//
ListExpr LoadUploadUnitTypeMap(ListExpr args)
{
    //error message;
    string msg = "string expected";

    if( nl->ListLength(args) != 1){
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    ListExpr filename = nl->First(args);
    if(nl->SymbolValue(filename) != "string"){
        ErrorReporter::ReportError(msg + " (first args is not a string)");
        return listutils::typeError();
    }
    return nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Pos"),
                        nl->SymbolAtom("uploadunit"))
                    )));
}
// local information
class LoadUploadUnitLocal
{
    public:
        int           counter;
        TupleType     *resulttype;
        RecordManager *rm;
        double        lamax, lamin, lomax, lomin;

        LoadUploadUnitLocal()
        {
            counter = 0;
            resulttype = NULL;
            rm = NULL;
            lamax = lomax = -1;
            lamin = lomin = 100000000000.0;
        }
};
//
// LoadUploadUnit value map
int LoadUploadUnitValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    LoadUploadUnitLocal *luul = NULL;
    GPSRecord           gr;
    DateTime            dt(instanttype);
    UploadUnit          *uu;
    UnitPos             up;
    Tuple               *tuple;

    switch(message)
    {
        case OPEN:
            {
                luul = new LoadUploadUnitLocal();
                string path_name = ((CcString*)args[0].addr)->GetValue();
                luul->resulttype = new TupleType(nl->Second(GetTupleResultType(s)));
                luul->rm = new RecordManager();
                luul->rm->setName(path_name.c_str());
                luul->rm->setExtension((char *)".txt");
                if(! luul->rm->Init()){
                    cout<<"Can not initlize the record manager!"<<endl;
                    return CANCEL;
                }
                local.setAddr(luul);
                return 0;
            }
        case REQUEST:
            {
                if(NULL == local.addr){
                    return CANCEL;
                }
                luul = (LoadUploadUnitLocal*)local.addr;
                if(! luul->rm->getNextRecord(gr)){
                    cout<<"Warning: end to read record!"<<endl;
                    return CANCEL;
                }

                if(gr.lo > luul->lomax) luul->lomax = gr.lo;
                if(gr.lo < luul->lomin) luul->lomin = gr.lo;
                if(gr.la > luul->lamax) luul->lamax = gr.la;
                if(gr.la < luul->lamin) luul->lamin = gr.la;

                dt.Set(gr.yy, gr.mm, gr.dd, gr.h, gr.m, gr.s);
                up.x = gr.lo;
                up.y = gr.la;
                uu = new UploadUnit(gr.Oid, dt, up);

                tuple = new Tuple(luul->resulttype);
                //tuple->PutAttribute(0, new CcInt(true, luul->counter));
                tuple->PutAttribute(0, new CcInt(true, gr.Oid));
                tuple->PutAttribute(1, uu);
                //luul->counter ++;
                result.setAddr(tuple);
                return YIELD;
            }
        case CLOSE:
            {
                if(local.addr)
                {
                    luul = (LoadUploadUnitLocal*)local.addr;
                    cout.precision(5);
                    cout<<std::fixed;
                    cout<<"longitude min: "<<luul->lomin<<endl;
                    cout<<"longitude max: "<<luul->lomax<<endl;
                    cout<<"latitude min: "<<luul->lamin<<endl;
                    cout<<"latitude max: "<<luul->lamax<<endl;
                    delete luul->rm;
                    delete luul->resulttype;
                    delete luul;
                    local.setAddr(Address(0));
                }
                return 0;
            }
    }
    return 0;
}

// loadUploadUnit Info
struct loadUploadUnitInfo : OperatorInfo
{
    loadUploadUnitInfo()
    {
        //cout<<"program is here: loadtrajectoryInfo~"<<endl;
        name      = "loaduploadunit";
        signature = "string->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
        syntax    = "loaduploadunit( _ )";
        meaning   = "load taxi's GPS data from file or directory";
    }
};

/****************************************************************

  4.operator TrajectorySplit

 ***************************************************************/
// type map for the ~trajectorysplit~
//     stream x real x int-> stream
ListExpr TrajectorySplitTypeMap(ListExpr args)
{
    ListExpr first, second, third, rest, listn, lastlistn, outList;
    string   argstr;

    cout<<"Trajectory Split Type Map~"<<endl;
    if(nl->ListLength(args) != 3)
    {
        return listutils::typeError("Two arguments expected");
    }

    first = nl->First(args);
    if(!listutils::isTupleStream(first))
    {
        return listutils::typeError("first arg : tuple stream expected");
    }

    second = nl->Second(args);
    if(!nl->IsEqual(second, CcReal::BasicType()))
    {
        return listutils::typeError("second arg is not a ccreal");
    }

    third = nl->Third(args);
    if(!nl->IsEqual(third, CcInt::BasicType()))
    {
        return listutils::typeError("third arg is not a ccint");
    }
    //
    //build resultlist
    //
    rest = nl->Second(nl->Second(first));
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    while(!(nl->IsEmpty(rest)))
    {
        lastlistn = nl->Append(lastlistn, nl->First(rest));
        rest = nl->Rest(rest);
    }
    lastlistn = 
        nl->Append(
                lastlistn,
                nl->TwoElemList(
                    nl->SymbolAtom("Sid"),
                    nl->SymbolAtom(CcInt::BasicType())));
    outList =
        nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    listn));
    return outList;
}


class TraSplitLocalInfo
{
    public:
        Tuple     *tuple;
        MPoint    *mpoint;
        int       upptr;
        int       numofupoint;
        TupleType *rtt;
        int       attrindex;
        double    sidelength;
        double    area;
        int       sid;

        //split with time
        double    duration;
        int       location;

        TraSplitLocalInfo()
        {
            mpoint = NULL;
            upptr = 0;
            numofupoint = 0;
            rtt = NULL;
            attrindex = -1;
            sidelength = 0;
            sid = 0;
            location = 0;
            duration = 0.0;
        }

        bool HasMore()
        {
            if(upptr >= numofupoint)
                return false;
            else
                return true;
        }

        void Set(MPoint *mp)
        {
            mpoint = mp;
            upptr = 0;
            numofupoint = mpoint->GetNoComponents();
            sid = 0;
            location = 0;
        }

        int GetSid()
        {
            int res = sid;
            sid ++;
            return res;
        }

        int GetNextUPoint(UPoint &up)
        {
            mpoint->Get(upptr, up);
            upptr ++;
            return 1;
        }
};

//
// Value Map for the ~trajectorysplit~
//
int TrajectorySplitValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    Word              t;
    Tuple             *tuple;
    MPoint            *mpoint, *resmpoint;
    UPoint            tempupoint, *upoint;
    //TupleType         *resultTupleType;
    ListExpr          resulttype;
    TraSplitLocalInfo *localinfo;

    int               nofattr, i;

    //cout<<"Trajectory Split Value Map : Begin~"<<endl;

    switch(message)
    {
        case OPEN:
            {
                //cout<<"Trajectory Split Value Map : Open~"<<endl;
                localinfo = new TraSplitLocalInfo();
                localinfo->sidelength = ((CcReal*)args[1].addr)->GetValue();
                localinfo->area = localinfo->sidelength * localinfo->sidelength;
                localinfo->attrindex = ((CcInt*)args[2].addr)->GetValue();
                resulttype = GetTupleResultType(s);
                localinfo->rtt = new TupleType(nl->Second(resulttype));
                qp->Open(args[0].addr);
                local.setAddr(localinfo);
                return 0;
            }
        case REQUEST:
            {
                //cout<<"Trajectory Split Value Map : Request Begin~"<<endl;
                if(!local.addr)
                {
                    cout<<"error: local is null"<<endl;
                    return CANCEL;
                }
                localinfo = (TraSplitLocalInfo *)local.addr;
                while(! localinfo->HasMore())
                {
                    qp->Request(args[0].addr, t);
                    if(! qp->Received(args[0].addr))
                    {
                        cout<<"finish!"<<endl;
                        return CANCEL;
                    }
                    tuple = (Tuple*)t.addr;
                    localinfo->tuple = tuple;
                    mpoint = (MPoint *)tuple->GetAttribute(localinfo->attrindex);
                    localinfo->Set(mpoint);
                }
                resmpoint = new MPoint(0);
                resmpoint->StartBulkLoad();
                while(localinfo->HasMore())
                {
                    upoint = new UPoint();
                    localinfo->GetNextUPoint(*upoint);
                    if(! upoint->IsDefined())
                    {
                        delete upoint;
                        continue;
                    }
                    resmpoint->MergeAdd(*upoint);
                    if(resmpoint->BoundingBoxSpatial().Area() >= localinfo->area)
                    {
                        break;
                    }
                }
                resmpoint->EndBulkLoad();
                if(resmpoint->IsEmpty())
                {
                    cout<<"result mpoint is null!"<<endl;
                    delete resmpoint;
                    return 0;
                }
                //resmpoint->Print(cout);
                tuple = new Tuple(localinfo->rtt);
                nofattr = localinfo->tuple->GetNoAttributes();
                for(i = 0; i < nofattr; i ++)
                {
                    if(i != localinfo->attrindex)
                    {
                        //tuple->PutAttribute(i, localinfo->tuple->GetAttribute(i));
                        tuple->CopyAttribute(i, localinfo->tuple, i);
                    }
                    else
                    {
                        tuple->PutAttribute(i, (Attribute*)resmpoint);
                    }
                }
                tuple->PutAttribute(i, (Attribute*)(new CcInt(localinfo->GetSid())));
                result.setAddr(tuple);
                //cout<<"Trajectory Split Value Map : Request end~"<<endl;
                return YIELD;
            }
        case CLOSE:
            {
                //cout<<"Trajectory Split Value Map : Close begin~"<<endl;
                qp->Close(args[0].addr);
                if(local.addr)
                {
                    localinfo = (TraSplitLocalInfo *)local.addr;
                    delete localinfo;
                    local.setAddr(Address(0));
                }
                return 0;
            }
    }
    return 0;
}

//
struct trasplitInfo : OperatorInfo
{
    trasplitInfo()
    {
        name      = "trajectorysplit";
        signature = "(stream(tuple((x1 t1)(x2 t2)...(xn tn))))->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
        syntax    = "_ trajectorysplit [_, _]";
        meaning   = "split trajectories of objects into a piece of segment";
    }
};

/****************************************************************

  4.operator sizetest

 ***************************************************************/
//
//
struct sizetest1Info : OperatorInfo
{
    sizetest1Info()
    {

        name      = "sizetest1";
        signature = "int->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
        syntax    = "sizetest1( _ )";
        meaning   = "create relation with size of x";
    }
};

//
//
struct sizetest2Info : OperatorInfo
{
    sizetest2Info()
    {

        name      = "sizetest2";
        signature = "int->(rel(tuple((x1 t1)(x2 t2)...(xn tn))))";
        syntax    = "sizetest2( _ )";
        meaning   = "create relation with size of x";
    }
};

//
// Type map for the ~sizetest1~
//       int -> stream
ListExpr SizeTest1TypeMap(ListExpr args)
{
    if(nl->ListLength( args) == 1)
    {
        ListExpr arg1 = nl->First(args);
        if( nl->IsEqual(arg1, CcInt::BasicType()))
        {
            ListExpr res = nl->TwoElemList(
                    nl->SymbolAtom("stream"),
                    nl->TwoElemList(
                        nl->SymbolAtom("tuple"),
                        nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("Id"),
                                nl->SymbolAtom("int")
                                )
                            )
                        )
                    );
            return res;
        }
    }
    return nl->SymbolAtom( Symbol::TYPEERROR());
}


//
// Type map for the ~sizetest2~
//       int -> relation
ListExpr SizeTest2TypeMap(ListExpr args)
{
    if(nl->ListLength( args) == 1)
    {
        ListExpr arg1 = nl->First(args);
        if( nl->IsEqual(arg1, CcInt::BasicType()))
        {
            ListExpr res = nl->TwoElemList(
                    nl->SymbolAtom("rel"),
                    nl->TwoElemList(
                        nl->SymbolAtom("tuple"),
                        nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("Id"),
                                nl->SymbolAtom("int")
                                )
                            )
                        )
                    );
            return res;
        }
    }
    return nl->SymbolAtom( Symbol::TYPEERROR());
}

//
// struct of size test local var
//
struct STLocal
{
    int sum;
    int count;
    GenericRelation *rel;
    ListExpr resulttype;
};


//
//  size test value map
//
int SizeTest1ValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    STLocal *l;
    Tuple *tuple;
    CcInt *ccint;
    switch(message)
    {
        case OPEN:
            l = new STLocal;
            l->sum = ((CcInt*)(args[0].addr))->GetValue();
            l->count = 0;
            l->resulttype = nl->Second(GetTupleResultType(s));
            cout<<"sum:"<<l->sum<<endl;
            local.setAddr(l);
            return 0;
        case REQUEST:
            if(local.addr == NULL)
            {
                cout<<"error!!!"<<endl;
                return CANCEL;
            }
            l = (STLocal*)local.addr;
            if(l->count >= l->sum)
                return CANCEL;
            tuple = new Tuple(l->resulttype);
            ccint = new CcInt(l->count++);
            tuple->PutAttribute(0, ccint);
            result.setAddr(tuple);
            return YIELD;
        case CLOSE:
            if(local.addr)
            {
                l = (STLocal*)local.addr;
                delete l;
            }
            local.setAddr(Address(0));
            return 0;
    }
    return 0;
}


//
// size test2 value map
//
int SizeTest2ValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    int i = 0;
    STLocal *l;
    Tuple *tuple;
    switch(message)
    {
        case OPEN:
            l = new STLocal();
            cout<<"test1:"<<endl;
            l->resulttype = nl->Second(GetTupleResultType(s));
            //nl->TwoElemList(nl->SymbolAtom("tuple"),nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Id"),nl->SymbolAtom("int"))));
            l->sum = ((CcInt*)(args[0].addr))->GetValue();
            cout<<"sum:"<<l->sum<<endl;
            l->rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
            if(l->rel->GetNoTuples() > 0)
            {
                l->rel->Clear();
            }
            cout<<"test2:"<<endl;
            local.setAddr(l);
        case REQUEST:
            if(local.addr == NULL)
            {
                cout<<"error!!!"<<endl;
                return CANCEL;
            }
            l = (STLocal*)local.addr;
            if(l->sum <= l->count)
                return CANCEL;
            for(i = 0; i < l->sum; i ++)
            {
                tuple = new Tuple(l->resulttype);
                tuple->PutAttribute(0, new CcInt(i));
                l->rel->AppendTuple(tuple);
            }
            l->count = i;
            result.setAddr(l->rel);
            return 0;
        case CLOSE:
            if(local.addr)
            {
                l = (STLocal*)local.addr;
                delete l;
            }
            local.setAddr(Address(0));
            return 0;
    }
    return 0;
}

/****************************************************************

  6.operator ConvertUU2UP

 ***************************************************************/

// type map function
//
ListExpr ConvertUU2UPTM(ListExpr args)
{
    //error message;
    string msg = "stream x int(attrindex) expected";
    //the number of args is 2: stream x int
    if(nl->ListLength(args) != 2)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr attrindex = nl->Second(args);

    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg+ " (first args is not a stream tuple");
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
        if (nl->SymbolValue(attrtype) != UploadUnit::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type upload unit.");
        }
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
    // return the result type and attribute index
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->OneElemList(
                nl->IntAtom(index)),
            resType);
}
// local information
//
class ConvertUU2UPLocalInfo
{
    public:
        int             attrindex;
        UploadUnit      *uu;
        Stream<Tuple>   *stream;
        Tuple           *tuple;
        ListExpr        tupletype;
};

//value map function
//
int ConvertUU2UPVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Tuple                 *tuple = NULL;
    UploadUnit            *uploadunit;
    UPoint                *upoint;
    Interval<Instant>     uiv;
    ConvertUU2UPLocalInfo *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (ConvertUU2UPLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new ConvertUU2UPLocalInfo();

            // get the arguments
            //
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            localinfo->attrindex = ((CcInt *)(args[2].addr))->GetValue()-1;

            // get the result tuple type
            localinfo->tupletype = nl->Second(GetTupleResultType(s));

            // open the stream
            localinfo->stream->open();
            tuple = localinfo->stream->request();
            if(tuple == NULL){
                return CANCEL;
            }
            localinfo->uu = (UploadUnit *)tuple->GetAttribute(localinfo->attrindex);
            // set the local
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null!"<<endl;
                return CANCEL;
            }
            localinfo = (ConvertUU2UPLocalInfo *)local.addr;

            while(true){
                // get a tuple from stream
                tuple = localinfo->stream->request();
                if(tuple == NULL){
                    return CANCEL;
                }
                uploadunit = (UploadUnit *)tuple->GetAttribute(localinfo->attrindex);
                //cout<<"address of uploadunit(in convertUU2UP function): "<<uploadunit<<endl;
                // check the uu and uploadunit have the same id
                if(uploadunit->GetID() == localinfo->uu->GetID()){
                    if(uploadunit->GetTime() <= localinfo->uu->GetTime()){
                        continue;
                    }
                    // time interval should not bigger than 60 minutes
                    if(uploadunit->GetTime().ToDouble() - localinfo->uu->GetTime().ToDouble() >= DateTime(durationtype, MILLISECONDS/24).ToDouble()){
                        localinfo->uu = uploadunit;
                        continue;
                    }
                    // construct the time interval
                    uiv.CopyFrom(Interval<Instant>(localinfo->uu->GetTime(), uploadunit->GetTime(), true, false));
                    // construct the upoint
                    upoint = new UPoint(uiv, localinfo->uu->GetPos().x, localinfo->uu->GetPos().y, uploadunit->GetPos().x, uploadunit->GetPos().y);
                    // construct the tuple
                    tuple = new Tuple(localinfo->tupletype);
                    tuple->PutAttribute(0, new CcInt(uploadunit->GetID()));
                    tuple->PutAttribute(1, upoint);
                    result.setAddr(tuple);

                    //
                    localinfo->uu = uploadunit;

                    return YIELD;
                }
                //
                localinfo->uu = uploadunit;
            }

        case CLOSE:
            if(local.addr){
                localinfo = (ConvertUU2UPLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct ConvertUU2UPInfo : OperatorInfo {
    ConvertUU2UPInfo()
    {
        name      = "convertUU2UP";
        signature = "((stream (tuple([a1:d1, ..., ai:uploadunit, ..., an:dn]))) x ai)-> stream (tuple (UTrip upoint))";
        syntax    = "_ convertUU2UP [ _ ]";
        meaning   = "convert upload units to upoints";
    }
};
/****************************************************************

  7.operator MeanFilter

 ***************************************************************/

// type map function
//
ListExpr MeanFilterTM(ListExpr args)
{
    //error message;
    string msg = "stream x int(n) x int(attrindex) expected";
    //the number of args is 5: stream x mpoint x double x double x int
    if(nl->ListLength(args) != 3)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr n = nl->Second(args);
    ListExpr attrindex = nl->Third(args);

    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg+ " (first args is not a stream tuple)");
        return listutils::typeError();
    }

    if(! nl->IsEqual(n, CcInt::BasicType())){
        ErrorReporter::ReportError(msg+ " (second args is not a int)");
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
        if (nl->SymbolValue(attrtype) != UploadUnit::BasicType()) //basic type
        {
            return NList::typeError("Attribute type is not of type upoint.");
        }
    }
    // construct the result type ListExpr
    ListExpr resType = stream;
    // return the result type and attribute index
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->OneElemList(
                nl->IntAtom(index)),
            resType);
}
// local information
//
class MeanFilterLocalInfo
{
    public:
        int             attrindex;
        //UploadUnit      *uu;
        Stream<Tuple>   *stream;
        //Tuple           *tuple;
        ListExpr        tupletype;
        list<UploadUnit *>  uuqueue;
        unsigned int        n;
        double              sumofx, sumofy;
};

//value map function
//
int MeanFilterVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    int        tmpn;
    Tuple                *tuple = NULL, *tuplenew = NULL;
    UploadUnit           *uploadunit, *tmpuu;
    MeanFilterLocalInfo  *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (MeanFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new MeanFilterLocalInfo();

            // get the arguments
            //
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            tmpn = ((CcInt *)(args[1].addr))->GetValue();    // get n
            if(tmpn <= 0){
                cout<<"Error: number of the n is less than 1!"<<endl;
                delete localinfo;
                return CANCEL;
            }
            localinfo->n = (unsigned int)tmpn;
            localinfo->attrindex = ((CcInt *)(args[3].addr))->GetValue()-1;  // get attribute index

            // get the result tuple type
            localinfo->tupletype = nl->Second(GetTupleResultType(s));

            // open the stream
            localinfo->stream->open();
            /*
               tuple = localinfo->stream->request();
               if(tuple == NULL){
               return CANCEL;
               }
               localinfo->uu = (UploadUnit *)tuple->GetAttribute(localinfo->attrindex);
               */
            //
            localinfo->sumofx = localinfo->sumofy = 0.0;
            localinfo->uuqueue.clear();
            // set the local
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null!"<<endl;
                return CANCEL;
            }
            localinfo = (MeanFilterLocalInfo *)local.addr;

            while(true){
                // get a tuple from stream
                tuple = localinfo->stream->request();
                if(tuple == NULL){
                    //cout<<"test 1"<<endl;
                    return CANCEL;
                }
                uploadunit = new UploadUnit(*((UploadUnit *)tuple->GetAttribute(localinfo->attrindex)));
                //uploadunit = (UploadUnit *)tuple->GetAttribute(localinfo->attrindex);
                //cout<<"address of uploadunit(in mean filter function): "<<uploadunit<<endl;
                //cout<<"uploadunit: "<<endl;
                //uploadunit->GetTime().Print(cout);
                //cout<<endl;
                // the upload unit queue is empty
                if(localinfo->uuqueue.empty()){
                    //cout<<"test 2"<<endl;
                    localinfo->uuqueue.push_back(uploadunit);
                    localinfo->sumofx = uploadunit->GetPos().x;
                    localinfo->sumofy = uploadunit->GetPos().y;
                    result.setAddr(tuple);
                    return YIELD;
                }
                // check the uu and uploadunit in the queue have the same id
                if(uploadunit->GetID() == localinfo->uuqueue.front()->GetID()){
                    if(uploadunit->GetTime() <= localinfo->uuqueue.back()->GetTime()){
                        continue;
                    }
                    // time interval should not bigger than 20 minutes
                    if(uploadunit->GetTime().ToDouble() - localinfo->uuqueue.back()->GetTime().ToDouble() >= DateTime(durationtype, MILLISECONDS/24).ToDouble()){
                        //localinfo->uuqueue.clear();
                        while(! localinfo->uuqueue.empty()){
                            delete localinfo->uuqueue.front();
                            localinfo->uuqueue.pop_front();
                        }
                        localinfo->uuqueue.push_back(uploadunit);
                        localinfo->sumofx = uploadunit->GetPos().x;
                        localinfo->sumofy = uploadunit->GetPos().y;
                        result.setAddr(tuple);
                        return YIELD;
                    }
                    // deal with upload unit
                    localinfo->uuqueue.push_back(uploadunit);
                    localinfo->sumofx += uploadunit->GetPos().x;
                    localinfo->sumofy += uploadunit->GetPos().y;
                    // elements in upload unit queue is less than n-1
                    if(localinfo->uuqueue.size() < localinfo->n){
                        //cout<<"test 5"<<endl;
                        result.setAddr(tuple);
                        return YIELD;
                    }
                    //cout<<"test 6"<<endl;
                    // construct the new upload unit
                    UnitPos up;
                    up.x = localinfo->sumofx/localinfo->n;
                    up.y = localinfo->sumofy/localinfo->n;
                    tmpuu = new UploadUnit(uploadunit->GetID(), uploadunit->GetTime(), up);
                    //
                    // deal with the n elements
                    uploadunit = localinfo->uuqueue.front();
                    localinfo->uuqueue.pop_front();
                    localinfo->sumofx -= uploadunit->GetPos().x;
                    localinfo->sumofy -= uploadunit->GetPos().y;
                    delete uploadunit;
                    // construct the tuple
                    tuplenew = new Tuple(localinfo->tupletype);
                    for(int i = 0; i < tuple->GetNoAttributes(); i ++){
                        if(i == localinfo->attrindex){
                            tuplenew->PutAttribute(i, tmpuu);
                        }else{
                            tuplenew->CopyAttribute(i, tuple, i);
                        }
                    }
                    result.setAddr(tuplenew);
                    return YIELD;
                }
                else{
                    // the id is different
                    //localinfo->uuqueue.clear();
                    while(! localinfo->uuqueue.empty()){
                        delete localinfo->uuqueue.front();
                        localinfo->uuqueue.pop_front();
                    }
                    localinfo->uuqueue.push_back(uploadunit);
                    localinfo->sumofx = uploadunit->GetPos().x;
                    localinfo->sumofy = uploadunit->GetPos().y;
                    result.setAddr(tuple);
                    return YIELD;
                }
            }
            //cout<<"test 8"<<endl;
            return CANCEL;

        case CLOSE:
            if(local.addr){
                localinfo = (MeanFilterLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                while(! localinfo->uuqueue.empty()){
                    delete localinfo->uuqueue.front();
                    localinfo->uuqueue.pop_front();
                }
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct MeanFilterInfo : OperatorInfo {
    MeanFilterInfo()
    {
        name      = "meanfilter";
        signature = "((stream (tuple([a1:d1, ..., ai:uploadunit, ..., an:dn]))) x n x ai)-> stream (tuple (UTrip upoint))";
        syntax    = "_ meanfilter [ _ , _ ]";
        meaning   = "mean filter of upload units with n elements";
    }
};
/****************************************************************

  8.operator MedianFilter

 ***************************************************************/
// local information
//
class MedianFilterLocalInfo
{
    public:
        int             attrindex;
        //UploadUnit      *uu;
        Stream<Tuple>   *stream;
        //Tuple           *tuple;
        ListExpr        tupletype;
        list<UploadUnit *>  uuqueue;
        unsigned int        n;

        bool getMedianPos(UnitPos &upos){
            vector<double> x;
            vector<double> y;
            list<UploadUnit *>::iterator it;;
            if(uuqueue.empty() || uuqueue.size() != n){
                return false;
            }
            for(it = uuqueue.begin(); it != uuqueue.end(); it++){
                x.push_back((*it)->GetPos().x);
                y.push_back((*it)->GetPos().y);
            }
            sort(x.begin(),x.end());
            sort(y.begin(),y.end());
            upos.x = x[(x.size()-1)/2];
            upos.y = y[(y.size()-1)/2];
            return true;
        }
};

//value map function
//
int MedianFilterVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    int        tmpn;
    Tuple                *tuple = NULL, *tuplenew = NULL;
    UploadUnit           *uploadunit, *tmpuu;
    MedianFilterLocalInfo  *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (MedianFilterLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new MedianFilterLocalInfo();

            // get the arguments
            //
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            tmpn = ((CcInt *)(args[1].addr))->GetValue();    // get n
            if(tmpn <= 0){
                cout<<"Error: number of the n is less than 1!"<<endl;
                delete localinfo;
                return CANCEL;
            }
            localinfo->n = (unsigned int)tmpn;
            localinfo->attrindex = ((CcInt *)(args[3].addr))->GetValue()-1;  // get attribute index

            // get the result tuple type
            localinfo->tupletype = nl->Second(GetTupleResultType(s));

            // open the stream
            localinfo->stream->open();
            //
            localinfo->uuqueue.clear();
            // set the local
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null!"<<endl;
                return CANCEL;
            }
            localinfo = (MedianFilterLocalInfo *)local.addr;

            while(true){
                // get a tuple from stream
                tuple = localinfo->stream->request();
                if(tuple == NULL){
                    return CANCEL;
                }
                uploadunit = new UploadUnit(*((UploadUnit *)tuple->GetAttribute(localinfo->attrindex)));
                // the upload unit queue is empty
                if(localinfo->uuqueue.empty()){
                    localinfo->uuqueue.push_back(uploadunit);
                    result.setAddr(tuple);
                    return YIELD;
                }
                // check the uu and uploadunit in the queue have the same id
                if(uploadunit->GetID() == localinfo->uuqueue.front()->GetID()){
                    if(uploadunit->GetTime() <= localinfo->uuqueue.back()->GetTime()){
                        continue;
                    }
                    // time interval should not bigger than 20 minutes
                    if(uploadunit->GetTime().ToDouble() - localinfo->uuqueue.back()->GetTime().ToDouble() >= DateTime(durationtype, MILLISECONDS/24).ToDouble()){
                        //localinfo->uuqueue.clear();
                        // clear and delete
                        while(! localinfo->uuqueue.empty()){
                            delete localinfo->uuqueue.front();
                            localinfo->uuqueue.pop_front();
                        }
                        localinfo->uuqueue.push_back(uploadunit);
                        result.setAddr(tuple);
                        return YIELD;
                    }
                    // deal with upload unit
                    localinfo->uuqueue.push_back(uploadunit);
                    // elements in upload unit queue is less than n-1
                    if(localinfo->uuqueue.size() < localinfo->n){
                        result.setAddr(tuple);
                        return YIELD;
                    }
                    //cout<<"test 6"<<endl;
                    // construct the new upload unit
                    UnitPos up;
                    assert(localinfo->getMedianPos(up));
                    tmpuu = new UploadUnit(uploadunit->GetID(), uploadunit->GetTime(), up);
                    //
                    // deal with the n elements
                    uploadunit = localinfo->uuqueue.front();
                    localinfo->uuqueue.pop_front();
                    delete uploadunit;
                    // construct the tuple
                    tuplenew = new Tuple(localinfo->tupletype);
                    for(int i = 0; i < tuple->GetNoAttributes(); i ++){
                        if(i == localinfo->attrindex){
                            tuplenew->PutAttribute(i, tmpuu);
                        }else{
                            tuplenew->CopyAttribute(i, tuple, i);
                        }
                    }
                    result.setAddr(tuplenew);
                    return YIELD;
                }
                else{
                    // the id is different, clear and delete
                    //localinfo->uuqueue.clear();
                    while(! localinfo->uuqueue.empty()){
                        delete localinfo->uuqueue.front();
                        localinfo->uuqueue.pop_front();
                    }
                    localinfo->uuqueue.push_back(uploadunit);
                    result.setAddr(tuple);
                    return YIELD;
                }
            }
            return CANCEL;

        case CLOSE:
            if(local.addr){
                localinfo = (MedianFilterLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                // clear and delete
                while(! localinfo->uuqueue.empty()){
                    delete localinfo->uuqueue.front();
                    localinfo->uuqueue.pop_front();
                }
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct MedianFilterInfo : OperatorInfo {
    MedianFilterInfo()
    {
        name      = "medianfilter";
        signature = "((stream (tuple([a1:d1, ..., ai:uploadunit, ..., an:dn]))) x n x ai)-> stream (tuple (UTrip upoint))";
        syntax    = "_ medianfilter [ _ , _ ]";
        meaning   = "median filter of upload units with n elements";
    }
};
/****************************************************************

  9.operator ConvertUP2MP

 ***************************************************************/

// type map function
//
ListExpr ConvertUP2MPTM(ListExpr args)
{
    //error message;
    string msg = "stream x int(index of upoint) x int(index of id) expected";
    //the number of args is 3: stream x int x int
    if(nl->ListLength(args) != 3)
    {
        ErrorReporter::ReportError(msg + " (invalid number of arguments)");
        return nl->TypeError();
    }
    //
    ListExpr stream = nl->First(args);
    ListExpr attrindex = nl->Second(args);  // index of upoint
    ListExpr indexofid = nl->Third(args);

    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg+ " (first args is not a stream tuple");
        return listutils::typeError();
    }

    // check the attribute index of upoint
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
            return NList::typeError("Attribute type of first is not of type upoint.");
        }
    }
    // check the attribute index of id
    attrname = nl->SymbolValue(indexofid);
    attrtype = nl->Empty();
    int index2 = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index2)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != CcInt::BasicType()) //basic type
        {
            return NList::typeError("Attribute type of second is not of type int.");
        }
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
                        nl->SymbolAtom("Trip"),
                        nl->SymbolAtom("mpoint")))));
    // return the result type and attribute index
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->TwoElemList(
                nl->IntAtom(index),
                nl->IntAtom(index2)),
            resType);
}
// local information
//
class ConvertUP2MPLocalInfo
{
    public:
        int             attrindex, indexofid, id;
        UPoint          *upoint;
        Stream<Tuple>   *stream;
        Tuple           *tuple = NULL, *told = NULL;
        ListExpr        tupletype;
        MPoint          *mpoint;

        // function get next mpoint from stream
        MPoint *genNextMPoint(){
            int tmpid;
            mpoint = new MPoint(0);
            mpoint->StartBulkLoad();
            if(tuple == NULL){
                // get the first tuple in the stream
                tuple = stream->request();
                if(tuple == NULL){
                    return NULL;
                }
            }
            told = tuple;
            id = ((CcInt *)tuple->GetAttribute(indexofid))->GetValue();
            upoint = (UPoint *)tuple->GetAttribute(attrindex);
            mpoint->MergeAdd(*upoint);
            //
            while((tuple = stream->request()) != NULL){
                tmpid = ((CcInt *)tuple->GetAttribute(indexofid))->GetValue();
                if(tmpid != id){
                    // id is different
                    mpoint->EndBulkLoad();
                    return mpoint;
                }
                //upoint = ((UPoint *)tuple->GetAttribute(attrindex))->Clone();
                upoint = (UPoint *)tuple->GetAttribute(attrindex);
                mpoint->MergeAdd(*upoint);
            }
            mpoint->EndBulkLoad();
            return mpoint;
        }
        Tuple* genNextTuple(){
            int i;
            Tuple *tnew = NULL;
            // get next mpoint
            MPoint *tmpmpoint = genNextMPoint();
            if(tmpmpoint == NULL){
                return NULL;
            }
            // construct the tuple
            tnew = new Tuple(tupletype);
            for(i = 0; i < told->GetNoAttributes(); i ++){
                if(indexofid == i){
                    tnew->PutAttribute(i, new CcInt(id));
                }else if(attrindex == i){
                    tnew->PutAttribute(i, tmpmpoint);
                }else{
                    tnew->CopyAttribute(i, told, i);
                }
            }
            return tnew;
        }
};

//value map function
//
int ConvertUP2MPVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Tuple *tuple = NULL;
    ConvertUP2MPLocalInfo *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (ConvertUP2MPLocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new ConvertUP2MPLocalInfo();
            // get the arguments
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            localinfo->attrindex = ((CcInt *)(args[3].addr))->GetValue()-1;
            localinfo->indexofid = ((CcInt *)(args[4].addr))->GetValue()-1;
            // get the result tuple type
            localinfo->tupletype = nl->Second(GetTupleResultType(s));
            // open the stream
            localinfo->stream->open();
            // set the local
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null!"<<endl;
                return CANCEL;
            }
            localinfo = (ConvertUP2MPLocalInfo *)local.addr;
            tuple = localinfo->genNextTuple();
            if(tuple == NULL){
                return CANCEL;
            }
            result.setAddr(tuple);
            return YIELD;

        case CLOSE:
            if(local.addr){
                localinfo = (ConvertUP2MPLocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}

//
// operator info
struct ConvertUP2MPInfo : OperatorInfo {
    ConvertUP2MPInfo()
    {
        name      = "convertUP2MP";
        signature = "((stream (tuple([aj:int, ..., ai:uploadunit, ..., an:dn]))) x ai x aj)-> stream (tuple (UTrip upoint))";
        syntax    = "_ convertUP2MP [ _ , _ ]";
        meaning   = "convert upoints to mpoints";
    }
};
/****************************************************************

  10.operator GKProject

 ***************************************************************/
/*
   Type Mapping for ~gkproject~

*/
ListExpr GKProjectTM(ListExpr args){
    int len = nl->ListLength(args);
    if( len != 1 ){
        return listutils::typeError(" one arguments expected");
    }
    string err = "point expected";
    ListExpr arg = nl->First(args);
    if(!listutils::isSymbol(arg)){
        return listutils::typeError(err);
    }
    string t = nl->SymbolValue(arg);
    if(t != Point::BasicType()){
        return listutils::typeError(err);
    }
    return nl->SymbolAtom(t); // Zone provided by user
}
/*
   Value Mapping for ~gkproject~

*/
int GKProjectVM(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Point* p = static_cast<Point*>(args[0].addr);
    Point* res = static_cast<Point*>(result.addr);
    MyGK(*p, *res);
    return 0;
}
//
// operator info
struct GKProjectInfo : OperatorInfo {
    GKProjectInfo()
    {
        name      = "gkproject";
        signature = "point -> point";
        syntax    = "gkproject(  _ )";
        meaning   = "gk projection of point";
    }
};
/****************************************************************

  11.operator PointMinus

 ***************************************************************/
/*
   Type Mapping for ~PointMinus~

*/
ListExpr PointMinusTM(ListExpr args){
    int len = nl->ListLength(args);
    if( len != 2 ){
        return listutils::typeError(" one arguments expected");
    }
    string err = "point x point expected";
    ListExpr p1 = nl->First(args);
    ListExpr p2 = nl->Second(args);
    if(! nl->IsEqual(p1, Point::BasicType())){
        return listutils::typeError("first arg is not a point");
    }
    if(! nl->IsEqual(p2, Point::BasicType())){
        return listutils::typeError("second arg is not a point");
    }
    return nl->SymbolAtom(Point::BasicType()); // Zone provided by user
}
/*
   Value Mapping for ~PointMinus~

*/
int PointMinusVM(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Point* p1 = static_cast<Point*>(args[0].addr);
    Point* p2 = static_cast<Point*>(args[1].addr);
    Point* res = static_cast<Point*>(result.addr);
    *res = *p1 - *p2;
    return 0;
}
//
// operator info
struct PointMinusInfo : OperatorInfo {
    PointMinusInfo()
    {
        name      = "pointminus";
        signature = "point x point -> point";
        syntax    = "_ pointminus _";
        meaning   = "point minus";
    }
};
/****************************************************************

  12.operator ConvertP2UU

 ***************************************************************/
/*
   Type Mapping for ~ConvertP2UU~

*/
ListExpr ConvertP2UUTM(ListExpr args){
    int len = nl->ListLength(args);
    if( len != 4 ){
        return listutils::typeError(" four arguments expected");
    }
    string msg = "stream(tuple(point)) x id x datetime x position expected";
    ListExpr stream = nl->First(args);
    ListExpr idindex = nl->Second(args);
    ListExpr dtindex = nl->Third(args);
    ListExpr posindex = nl->Fourth(args);
    if(! listutils::isStream(stream)){
        ErrorReporter::ReportError(msg+ " (first args is not a stream tuple");
        return listutils::typeError();
    }
    // check the attribute index of upoint
    string   attrname = nl->SymbolValue(idindex);
    ListExpr attrtype = nl->Empty();
    ListExpr attrlist = nl->Second(nl->Second(stream));
    int index = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != CcInt::BasicType()) //basic type
        {
            return NList::typeError("Attribute type of first is not of type int.");
        }
    }
    // check the attribute index of id
    attrname = nl->SymbolValue(dtindex);
    attrtype = nl->Empty();
    int index2 = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index2)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != DateTime::BasicType()) //basic type
        {
            return NList::typeError("Attribute type of second is not of type instant.");
        }
    }
    // check the attribute index of id
    attrname = nl->SymbolValue(posindex);
    attrtype = nl->Empty();
    int index3 = listutils::findAttribute(attrlist, attrname, attrtype);
    if (0 == index3)
    {
        return NList::typeError( "Attribute name '" + attrname +"' is not known!");
    }else{
        if (nl->SymbolValue(attrtype) != Point::BasicType()) //basic type
        {
            return NList::typeError("Attribute type of second is not of type point.");
        }
    }
    // construct the result type ListExpr
    ListExpr resType = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Pos"),
                        nl->SymbolAtom(UploadUnit::BasicType())))));
    // return the result type and attribute index
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->ThreeElemList(
                nl->IntAtom(index),
                nl->IntAtom(index2),
                nl->IntAtom(index3)),
            resType);
}

class ConvertP2UULocalInfo
{
    public:
        Stream<Tuple> *stream;
        ListExpr      resulttype;
        int    id, dt, pos;
};
/*
   Value Mapping for ~ConvertP2UU~

*/
int ConvertP2UUVM(Word* args, Word& result, int message, Word& local, Supplier s){
    int      oid;
    DateTime *datetime;
    Point    *point;
    Tuple *tuple = NULL, *res = NULL;
    ConvertP2UULocalInfo *localinfo = NULL;

    switch( message )
    {
        case OPEN:
            if(local.addr){
                localinfo = (ConvertP2UULocalInfo *)local.addr;
                delete localinfo;
            }
            localinfo = new ConvertP2UULocalInfo();
            // get the arguments
            localinfo->stream = new Stream<Tuple>(args[0].addr);
            localinfo->id = ((CcInt *)(args[4].addr))->GetValue()-1;
            localinfo->dt = ((CcInt *)(args[5].addr))->GetValue()-1;
            localinfo->pos = ((CcInt *)(args[6].addr))->GetValue()-1;
            // get the result tuple type
            localinfo->resulttype = nl->Second(GetTupleResultType(s));
            // open the stream
            localinfo->stream->open();
            // set the local
            local = SetWord(localinfo);
            return 0;

        case REQUEST:
            if(! local.addr){
                cout<<"local.addr is null!"<<endl;
                return CANCEL;
            }
            localinfo = (ConvertP2UULocalInfo *)local.addr;
            while((tuple = localinfo->stream->request()) != NULL){
                oid = ((CcInt *)tuple->GetAttribute(localinfo->id))->GetValue();
                datetime = (DateTime *)tuple->GetAttribute(localinfo->dt);
                point = (Point *)tuple->GetAttribute(localinfo->pos);
                res = new Tuple(localinfo->resulttype);
                res->PutAttribute(0, new UploadUnit(oid, *datetime, UnitPos(point->GetX(), point->GetY())));
                result.setAddr(res);
                tuple->DeleteIfAllowed();
                return YIELD;
            }
            return CANCEL;

        case CLOSE:
            if(local.addr){
                localinfo = (ConvertP2UULocalInfo *)local.addr;
                localinfo->stream->close();
                delete localinfo->stream;
                delete localinfo;
            }
            return 0;
    }
    return 0;
}
//
// operator info
struct ConvertP2UUInfo : OperatorInfo {
    ConvertP2UUInfo()
    {
        name      = "convertP2UU";
        signature = "stream(tuple(point)) -> uploadunit";
        syntax    = "_ ConvertP2UU [ _, _, _]";
        meaning   = "convert point to uploadunit";
    }
};
/****************************************************************

  Algebra Load

 ***************************************************************/
class LoadAlgebra : public Algebra
{
    public:
        LoadAlgebra() : Algebra()
    {
        AddOperator(LoadDataInfo(), LoadDataValueMap, LoadDataTypeMap);
        AddOperator(LoadDataFromDirInfo(), LoadDataFromDirValueMap, LoadDataFromDirTypeMap);
        AddOperator(trasplitInfo(), TrajectorySplitValueMap, TrajectorySplitTypeMap);
        AddOperator(sizetest1Info(), SizeTest1ValueMap, SizeTest1TypeMap);
        AddOperator(sizetest2Info(), SizeTest2ValueMap, SizeTest2TypeMap);
        AddOperator(loadUploadUnitInfo(), LoadUploadUnitValueMap, LoadUploadUnitTypeMap);
        AddOperator(ConvertUU2UPInfo(), ConvertUU2UPVM, ConvertUU2UPTM);
        AddOperator(ConvertUP2MPInfo(), ConvertUP2MPVM, ConvertUP2MPTM);
        AddOperator(MeanFilterInfo(), MeanFilterVM, MeanFilterTM);
        AddOperator(MedianFilterInfo(), MedianFilterVM, MeanFilterTM);
        AddOperator(GKProjectInfo(), GKProjectVM, GKProjectTM);
        AddOperator(PointMinusInfo(), PointMinusVM, PointMinusTM);
        AddOperator(ConvertP2UUInfo(), ConvertP2UUVM, ConvertP2UUTM);
    }

        ~LoadAlgebra() {}
};


//
//
    extern "C"
Algebra* InitializeLoadAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    cout<<"program is here: InitializeLoadAlgebra()~"<<endl;
    return (new LoadAlgebra());
}
