#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "LoadAlgebra.h"
#include <dirent.h>
#include "Stream.h"
#include "../Spatial/Point.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../Temporal/TemporalAlgebra.h"
#include "../SETI/UploadUnit.h"
#include "RecordManager.h"
#include <list>


extern NestedList     *nl;
extern QueryProcessor *qp;


//
//
struct loaddataInfo : OperatorInfo
{
  loaddataInfo()
  {
    //cout<<"program is here: loaddataInfo~"<<endl;
    name      = "loaddata";
    signature = "string->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
    syntax    = "loaddata( _ )";
    meaning   = "load taxi data from file";
  }
};

//
//
struct loadtrajectoryInfo : OperatorInfo
{
  loadtrajectoryInfo()
  {
    //cout<<"program is here: loadtrajectoryInfo~"<<endl;
    name      = "loadtrajectory";
    signature = "string->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
    syntax    = "loadtrajectory( _ )";
    meaning   = "load taxi data from file or directory";
  }
};
//
//
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
//
//
struct breakupInfo : OperatorInfo
{
  breakupInfo()
  {
    //cout<<"program is here: loaddataInfo~"<<endl;
    name      = "breakup";
    signature = "(stream(tuple((x1 t1)(x2 t2)...(xn tn))))->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
    syntax    = "_ breakup";
    meaning   = "break up trajectories of objects into piece";
  }
};

//
//
struct trasplitInfo : OperatorInfo
{
  trasplitInfo()
  {
    //cout<<"program is here: loaddataInfo~"<<endl;
    name      = "trajectorysplit";
    signature = "(stream(tuple((x1 t1)(x2 t2)...(xn tn))))->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
    syntax    = "_ trajectorysplit [_, _]";
    meaning   = "split trajectories of objects into a piece of segment";
  }
};

/*
//
//
struct breakupInfo : OperatorInfo
{
  breakupInfo()
  {

    cout<<"program is here: breakupInfo~"<<endl;
    name      = "breakup";
    signature = "(stream(tuple((x1 t1)(x2 t2)...(xn tn))))->(stream(tuple((x1 t1)(x2 t2)...(xn tn))))";
    syntax    = " _ breakup";
    meaning   = "query trains feed breakup consume;";
  }
};
*/
//LoadData
//
class LD
{
  public:
  ifstream     *fin;
  TupleType    *resulttype;
  int          count,count2;
  int          rec_count;
  bool         isend;
  char         *buf;
  MyRecord     *mr;

  LD()
  {
    fin = NULL;
    resulttype = NULL;
    count = count2 = 0;
    rec_count = 0;
    isend = false;
    buf = NULL;
    mr = NULL;
  }

  ~LD()
  {
    if(fin != NULL)
    {
      fin->close();
    }
    if(resulttype !=  NULL)
    {
      delete resulttype;
      resulttype = NULL;
    }
    if(buf != NULL)
    {
      delete [] buf;
      buf = NULL;
    }
    if(mr != NULL)
    {
        delete mr;
    }
  }

  void clear()
  {
    if(fin != NULL)
    {
      fin->close();
    }
    if(resulttype !=  NULL)
    {
      delete resulttype;
      resulttype = NULL;
    }
    if(buf != NULL)
    {
      delete [] buf;
      buf = NULL;
    }
  }
};

/*
//
//
class BreakUpInfo
{
    public:
    Tuple *tuple;


};
*/

int myatoi(char *str)
{
  int i;
  int res = 0;
  for(i = 0; str[i] != '\0'; i ++)
  {
    if(str[i] >= '0' && str[i] <= '9')
    {
      res = res*10 + (str[i] - '0');
    }
  }
  return res;
}

const double SIZEOFBLOCK = 1000.0;
//****************************
//**getNumBlock
//***************************
int getNumBlock(double d)
{
    int res = 0;
    res = d / SIZEOFBLOCK;
    return res;
}


//****************************
//**recordNext
//***************************
int recordNext(int r)
{
	return (r+1)%2;
}

int recordBefore(int r)
{
	return (r+1)%2;
}

//****************************
//**genUTripCheck
//***************************
int genUTripCheck(const MyRecord &mr1, const MyRecord &mr2)
{
    return 1;
}


//************************
//**generate the unit trip
//************************
int genUTrip(const MyRecord &mr1, const MyRecord &mr2, UPoint &res) // mr1 < mr2
{
    DateTime start(instanttype), end(instanttype);

    if(mr1.id != mr2.id)
    {
        //cout<<mr1;
        //cout<<mr2;
        return -2;  // the id of record is different
    }
    
    //  UPoint(interval, x0, y0, x1, y1);
    //  Interval(start, end, true, false);
    //
    //  DateTime.Set(int year, int month, int day, int hour, int minute, int second, int millisecond);
    //if(mr1.yy != mr2.yy || mr1.mm != mr2.mm || mr1.dd != mr2.dd || mr1.h != mr2.h)
    MyRecord temp = mr2 - mr1;
    if(0 != temp.yy || 0 != temp.mm || 0 != temp.dd || 0 != temp.h)
    {
        return -3;   // the time between two record is bigger than 1 hour
    }

    if(0 == genUTripCheck(mr1, mr2))
    {
        return -1;   // can not through the check
    }

    start.Set(mr1.yy, mr1.mm, mr1.dd, mr1.h, mr1.m, mr1.s);
    end.Set(mr2.yy, mr2.mm, mr2.dd, mr2.h, mr2.m, mr2.s);
    
    //cout<<start<<endl;
    //cout<<end<<endl;

    Interval<Instant> interval(start, end, true, false);
    //cout<<"DEBUG:test"<<endl;
    res = UPoint(interval, mr1.X, mr1.Y, mr2.X, mr2.Y);
    return 1;
}

//************************
//**generate the unit trip
//************************
int genUTrip2(const MyRecord &mr1, const MyRecord &mr2, UPoint &res) // mr1 < mr2
{
    DateTime start(instanttype), end(instanttype);
    start.Set(mr1.yy, mr1.mm, mr1.dd, mr1.h, mr1.m, mr1.s);
    end.Set(mr2.yy, mr2.mm, mr2.dd, mr2.h, mr2.m, mr2.s);
    Interval<Instant> interval(start, end, true, false);
    res = UPoint(interval, mr1.X, mr1.Y, mr2.X, mr2.Y);
    return 1;
}

//************************
//**generate the unit trip
//************************
int genUTrip3(const MyRecord &s, const MyRecord &mr1, const MyRecord &mr2, UPoint &res) // mr1 < mr2
{
    DateTime start(instanttype), end(instanttype);
    MyRecord tmp1, tmp2;
    tmp1 = mr1-s;
    tmp2 = mr2-s;
    start.Set(2016, 1, 1+tmp1.dd, tmp1.h, tmp1.m, tmp1.s);
    end.Set(2016, 1, 1+tmp2.dd, tmp2.h, tmp2.m, tmp2.s);
    //start.Print(cout);
    //cout<<endl;
    //end.Print(cout);
    //cout<<endl;
    Interval<Instant> interval(start, end, true, false);
    res = UPoint(interval, mr1.X, mr1.Y, mr2.X, mr2.Y);
    return 1;
}

//*****************************
//**generate the trip from file
//*****************************
bool genMPoint(char *filename, vector<MPoint*> &result)
{
    MyRecord *mr1=NULL, *mr2=NULL, *mrtmp=NULL;
    char     buf[1024];
    ifstream input;
    MPoint   *mpoint;
    UPoint   *upoint;
    MyRecord *tmprecord, *temprecord2;
    DateTime start(instanttype), end(instanttype);
    
    // Clear the result vector
    result.clear();
    // Open the file
    input.open(filename, ifstream::in);
    if(! input.is_open()){
        cout<<"file " <<filename<<" open failed!"<<endl;
        return false;
    }
    //
    if(strContain(filename, ".plt")){
        mr1 = new MyRecordTaxi();
        mr2 = new MyRecordTaxi();
        tmprecord = new MyRecordTaxi();
        temprecord2 = new MyRecordTaxi();
    }else if(strContain(filename, ".txt")){
        mr1 = new MyRecord();
        mr2 = new MyRecord();
        tmprecord = new MyRecord();
        temprecord2 = new MyRecord();
    }else{
        cout<<filename<<": file type of trajectory is error!"<<endl;
        input.close();
        return false;
    }
    cout<<"Handling file: "<<filename<<endl;
    // deal with the data
    while(! input.eof()){
        mpoint = new MPoint(0);
        mpoint->StartBulkLoad();
        // if mr1 is not defined
        if(! mr1->IsDefined()){
            input.getline(buf, 1024, '\n');
            if(! input.good()){
                break;
            }
            //cout<<"deal with string: "<<buf<<endl;
            mr1->SetRstr(buf);
            while(-1 == mr1->FreshData() && input.good()){
                input.getline(buf, 1024, '\n');
                mr1->SetRstr(buf);
                //cout<<"deal with string: "<<buf<<endl;
            }
            if(! input.good()){
                break;
            }
        }
        //store the begin time
        *tmprecord = *mr1;

        while(! input.eof()){
            // Get the next record
            input.getline(buf, 1024, '\n');
            //cout<<"deal with string: "<<buf<<endl;
            mr2->SetRstr(buf);
            if(-1 == mr2->FreshData()){
                continue;
            }
            if(mr2->dt < tmprecord->dt)
                break;
            if(mr2->X == mr1->X && mr2->Y == mr1->Y){
                //cout<<buf<<endl;
                continue;
            }
            if(mr1->dt > mr2->dt){
                cout<<buf<<endl;
                continue;
            }
            if(mr1->dt == mr2->dt){
                while((mr1->dt == mr2->dt || mr2->FreshData())&& (!input.eof())){
                    *temprecord2 = *mr2;
                    input.getline(buf, 1024, '\n');
                    mr2->SetRstr(buf);
                }
                if(! input.eof()){
                    upoint = new UPoint(0);
                    genUTrip3(*tmprecord, *mr1, *temprecord2, *upoint);
                    mpoint->MergeAdd(*upoint);
                    *mr1 = *temprecord2;
                }

            }

            // Whether the time between two record is bigger than 1 hour?
            //if(mr2->dt - mr1->dt > 3600 || *mr1 == *mr2){
            if(mr2->dt - mr1->dt > 3600){
                break;
            }
            upoint = new UPoint(0);
            if(! genUTrip3(*tmprecord, *mr1, *mr2, *upoint))
            {
                cout<<"failed to generate utrip!"<<endl;
                delete upoint;
                continue;
            }
            upoint->Print(cout);
            mpoint->MergeAdd(*upoint);
            // turn to next record
            mrtmp = mr1;
            mr1 = mr2;
            mr2 = mrtmp;
        }
        mpoint->EndBulkLoad();
        
        if(! mpoint->IsEmpty()){
            result.push_back(mpoint);
        }
    }
    cout<<filename<<": data process successed!"<<endl;
    input.close();
    delete mr1;
    delete mr2;
    delete tmprecord;
    delete temprecord2;
    return true;
}

//*****************************
//**generate the trip from file "Geo life Trajectory"
//*****************************
bool genMPoint2(char *filename, vector<MPoint*> &result)
{
    MyRecordTaxi *mr1=NULL, *mr2=NULL, *mrtmp=NULL;
    char         buf[1024];
    ifstream     input;
    MPoint       *mpoint;
    UPoint       *upoint;
    MyRecordTaxi *tmprecord, *temprecord2;
    DateTime start(instanttype), end(instanttype);
    
    bool flag_same_time = false;


    // Clear the result vector
    result.clear();
    // Open the file
    if(filename == NULL){
        return false;
    }
    input.open(filename, ifstream::in);
    if(! input.is_open()){
        cout<<"file " <<filename<<" open failed!"<<endl;
        return false;
    }
    mr1 = new MyRecordTaxi();
    mr2 = new MyRecordTaxi();
    tmprecord = new MyRecordTaxi();
    temprecord2 = new MyRecordTaxi();
    
    cout<<"Handling file: "<<filename<<endl;
    // deal with the data
    while(! input.eof()){
        mpoint = new MPoint(0);
        mpoint->StartBulkLoad();
        // if mr1 is not defined
        if(! mr1->IsDefined()){
            input.getline(buf, 1024, '\n');
            //cout<<"deal with string: "<<buf<<endl;
            mr1->SetRstr(buf);
            while(-1 == mr1->FreshData() && (!input.eof())){
                //cout<<"ERROR: mr1 is error!"<<endl;
                input.getline(buf, 1024, '\n');
                mr1->SetRstr(buf);
                //cout<<"deal with string: "<<buf<<endl;
            }
            if(input.eof()){
                //cout<<"file is end!"<<endl;
                break;
            }
        }
        //store the begin time
        *tmprecord = *mr1;

        while(! input.eof()){
            // Get the next record
            input.getline(buf, 1024, '\n');
            //cout<<"deal with string: "<<buf<<endl;
            mr2->SetRstr(buf);
            if(-1 == mr2->FreshData()){
                cout<<"ERROR: mr2 fresh failed!"<<endl;
                continue;
            }
            if(mr2->dt < tmprecord->dt){
                //cout<<"ERROR: the date time of mr2 is less than begin time!"<<endl;
                //cout<<*mr2<<endl;
                //cout<<*tmprecord<<endl;
                
                continue;
            }
            //if(mr2->X == mr1->X && mr2->Y == mr1->Y){
                //cout<<buf<<endl;
                //cout<<"Warming: mr1 and mr2 have the same location!"<<endl;
                //cout<<*mr1<<endl;
                //cout<<*mr2<<endl;
                //continue;
                /*while((mr1->dt == mr2->dt || mr2->FreshData())&& (!input.eof())){
                    *temprecord2 = *mr2;
                    input.getline(buf, 1024, '\n');
                    mr2->SetRstr(buf);
                }
                if(! input.eof()){
                    upoint = new UPoint(0);
                    genUTrip3(*tmprecord, *mr1, *temprecord2, *upoint);
                    mpoint->MergeAdd(*upoint);
                    *mr1 = *temprecord2;
                }*/
            //}
            if(mr1->dt > mr2->dt){
                //cout<<"Warming: mr1's date time bigger than mr2!"<<endl;
                //cout<<*mr1<<endl;
                //cout<<*mr2<<endl;
                continue;
            }
            if(mr1->dt == mr2->dt){
                //cout<<"Warming: mr1's date time is same as mr2!"<<endl;
                //cout<<"mr1->dt"<<mr1->dt<<endl;
                //cout<<"mr2->dt"<<mr2->dt<<endl;
                //cout<<*mr1<<endl;
                //cout<<*mr2<<endl;
                flag_same_time = true;
                continue;
            }

            // Whether the time between two record is bigger than 1 hour?
            //if(mr2->dt - mr1->dt > 3600 || *mr1 == *mr2){
            if(mr2->dt - mr1->dt > 1800){
                //cout<<"Warning: date time between mr1 and mr2 is bigger than half hour!"<<endl;
                break;
            }
            upoint = new UPoint(true);
            if(! genUTrip3(*tmprecord, *mr1, *mr2, *upoint))
            {
                cout<<"failed to generate utrip!"<<endl;
                delete upoint;
                continue;
            }
            //upoint->Print(cout);
            if(! upoint->IsDefined()){
                cout<<"ERROR: upoint is undefined!"<<endl;
                delete upoint;
                continue;
            }

            mpoint->MergeAdd(*upoint);
            // turn to next record
            mrtmp = mr1;
            mr1 = mr2;
            mr2 = mrtmp;
        }
        mrtmp = mr1;
        mr1 = mr2;
        mr2 = mrtmp;
        mpoint->EndBulkLoad();
        
        if(mpoint->IsEmpty()){
            cout<<"ERROR: mpoint is empty!"<<endl;
            mpoint->DeleteIfAllowed();
            continue;
        }
        result.push_back(mpoint);
    }
    if(flag_same_time){
        cout<<"Warning: "<<filename<<" have the same time records!"<<endl;
    }
    cout<<filename<<": data process successed!"<<endl;
    input.close();
    delete mr1;
    delete mr2;
    delete tmprecord;
    delete temprecord2;
    return true;
}

//*****************************
//**generate the trip from file "Geo life Trajectory"
//*****************************
bool genMPoint3(char *filename, vector<MPoint*> &result)
{
    MyRecord *mr1=NULL, *mr2=NULL, *mrtmp=NULL;
    char     buf[1024];
    ifstream input;
    MPoint   *mpoint;
    UPoint   *upoint;
    MyRecord *tmprecord, *temprecord2;
    DateTime start(instanttype), end(instanttype);
    
    bool flag_same_time = false;

    // Clear the result vector
    result.clear();
    // Open the file
    if(filename == NULL){
        return false;
    }
    input.open(filename, ifstream::in);
    if(! input.is_open()){
        cout<<"file " <<filename<<" open failed!"<<endl;
        return false;
    }
    mr1 = new MyRecord();
    mr2 = new MyRecord();
    tmprecord = new MyRecord();
    temprecord2 = new MyRecord();
    
    cout<<"Handling file: "<<filename<<endl;
    // deal with the data
    while(! input.eof()){
        mpoint = new MPoint(0);
        mpoint->StartBulkLoad();
        // if mr1 is not defined
        if(! mr1->IsDefined()){
            input.getline(buf, 1024, '\n');
            mr1->SetRstr(buf);
            while(-1 == mr1->FreshData() && (!input.eof())){
                input.getline(buf, 1024, '\n');
                mr1->SetRstr(buf);
            }
            if(input.eof()){
                break;
            }
        }
        //store the begin time
        *tmprecord = *mr1;

        while(! input.eof()){
            // Get the next record
            input.getline(buf, 1024, '\n');
            //cout<<"deal with string: "<<buf<<endl;
            mr2->SetRstr(buf);
            if(-1 == mr2->FreshData()){
                cout<<"ERROR: mr2 fresh failed!"<<endl;
                continue;
            }
            if(mr2->dt < tmprecord->dt){
                continue;
            }
            if(mr1->dt > mr2->dt){
                continue;
            }
            if(mr1->dt == mr2->dt){
                flag_same_time = true;
                continue;
            }

            // Whether the time between two record is bigger than 1 hour?
            if(mr2->dt - mr1->dt > 1800){
                break;
            }
            upoint = new UPoint(true);
            if(! genUTrip3(*tmprecord, *mr1, *mr2, *upoint))
            {
                cout<<"failed to generate utrip!"<<endl;
                delete upoint;
                continue;
            }
            //upoint->Print(cout);
            if(! upoint->IsDefined()){
                cout<<"ERROR: upoint is undefined!"<<endl;
                delete upoint;
                continue;
            }

            mpoint->MergeAdd(*upoint);
            // turn to next record
            mrtmp = mr1;
            mr1 = mr2;
            mr2 = mrtmp;
        }
        mrtmp = mr1;
        mr1 = mr2;
        mr2 = mrtmp;
        mpoint->EndBulkLoad();
        
        if(mpoint->IsEmpty()){
            cout<<"ERROR: mpoint is empty!"<<endl;
            mpoint->DeleteIfAllowed();
            continue;
        }
        result.push_back(mpoint);
    }
    if(flag_same_time){
        cout<<"Warning: "<<filename<<" have the same time records!"<<endl;
    }
    cout<<filename<<": data process successed!"<<endl;
    input.close();
    delete mr1;
    delete mr2;
    delete tmprecord;
    delete temprecord2;
    return true;
}

//value mapping function of operator~loaddata~
//
int LoadDataValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
  const int BUFFER_SIZE=64;
  LD        *ldata;
  int       resofgenutrip, current_recid;
  int       mr_p = 0; //mr_p : pointer to current record;
  MyRecord  mr, *temp_mr[2];
  MPoint    *cartrip, *restrip;
  UPoint    *carupoint;

  //cout<<"program is here: value map~"<<endl;

  switch(message)
  {
    case OPEN:
      {
        ldata = new LD;
        char *filename = new char[30];
        string path_name  = ((CcString*)args[0].addr)->GetValue();
        ldata->resulttype = new TupleType(nl->Second(GetTupleResultType(s)));
        strcpy(filename,path_name.c_str());
        //open datafile
        ldata->fin = new ifstream(filename);
        
        delete [] filename;

        if(ldata->fin == NULL)
        {
          cout<<"open file fail~"<<endl;
          ldata->clear();
          return -1;
        }
        ldata->buf = new char[BUFFER_SIZE];
        if(ldata->buf == NULL)
        {
          cout<<"new buf error~\n"<<endl;
          ldata->clear();
          return -1;
        }
//
//        when ldata->mr was initial, we set the value null; when the program is running , we will set the ldata->mr as the record that 
//        has different id.
//        ldata->mr = new MyRecord();
//        

        local.setAddr(ldata);
        return 0;
      }
    case REQUEST:
      {
        if(local.addr == NULL)
            return CANCEL;
        ldata = (LD*)local.addr;
        if(ldata->fin == NULL || ldata->resulttype == NULL || ldata->fin->eof())
            return CANCEL;

        //mpoint : car trip
        //
        cartrip = new MPoint(0);
        
        while(1)
        {
        cartrip->StartBulkLoad();
        //
        if(NULL == ldata->mr)   //if ldata->mr is null
        {
            mr_p = 0;
            temp_mr[0] = new MyRecord();
            if(!ldata->fin->eof())
            {
                ldata->fin->getline(ldata->buf, BUFFER_SIZE);

               // cout<<ldata->buf<<endl;

                temp_mr[0]->SetRstr(ldata->buf);
                while(-1 == temp_mr[0]->FreshData())
                {
                    if(ldata->fin->eof())
                        break;

                    ldata->fin->getline(ldata->buf, BUFFER_SIZE);
                    
                    temp_mr[0]->SetRstr(ldata->buf);
                }
            }
        }
        else
        {
            temp_mr[0] = ldata->mr;    //give the address of ldata->mr to temp_mr[0]
        }
        
        //
        current_recid = temp_mr[0]->id;
        //cout<<"first mr id:"<<temp_mr[0]->id<<endl;
        
        temp_mr[1] = new MyRecord();
        
        
	//
	mr_p = 1;
        //code to create the trajectories of the car
        //using mpoint
      	while(!(ldata->fin->eof()))
	{
            //get a record

            ldata->fin->getline(ldata->buf, BUFFER_SIZE);
            
            temp_mr[mr_p]->SetRstr(ldata->buf);
            while(1 != temp_mr[mr_p]->FreshData()|| (*temp_mr[0]) == (*temp_mr[1]))
                //check the data whether is right
            {
                if(ldata->fin->eof())
                {
                    break;
                }

    		ldata->fin->getline(ldata->buf, BUFFER_SIZE);
                
                temp_mr[mr_p]->SetRstr(ldata->buf);
            }
			
	    //code to create a upoint
	    carupoint = new UPoint(true);
	    //cout<<"before genutrip function:"<<endl;
            //cout<<*temp_mr[mr_p];
	    resofgenutrip = genUTrip(*temp_mr[(mr_p+1)%2], *temp_mr[mr_p], *carupoint);
	    //
	    if(-3 == resofgenutrip)   // the time between two record is bigger than 1 hour
	    {
                //cout<<"in the -3"<<endl;
		mr_p = recordNext(mr_p);
	    }
	    else if(-2 == resofgenutrip)  // the id of record is different
	    {
                //cout<<"in the -2"<<endl;
		ldata->mr = temp_mr[mr_p];
		break;
	    }
	    else if(-1 == resofgenutrip)  //can not through the check
	    {
                //cout<<"in the -1"<<endl;
		continue;
	    }
	    else if(1 == resofgenutrip)
	    {
                //cout<<"in the 1"<<endl;
		cartrip->MergeAdd(*carupoint);
		mr_p = recordNext(mr_p);
	    }
	    else
	    {
                //cout<<"in the other"<<endl;
		cout<<"error is happening in LoadAlgebra.cpp when create mpoint"<<endl;
		return -1;
	    }
        }
        cartrip->EndBulkLoad();
        //check the restrip whether empty
        if(cartrip->IsEmpty())
        {
            
            continue;
        }
        
        restrip = new MPoint(0);
        cartrip->gk(39,*restrip);
        
        cartrip->DeleteIfAllowed();

        //create the tuple
        Tuple * tuple = new Tuple(ldata->resulttype);

        //put record to tuple
        tuple->PutAttribute(0, new CcInt(true, current_recid));
        tuple->PutAttribute(1, restrip);
        result.setAddr(tuple);
        
        return YIELD;
      }
      }
    case CLOSE:
      {
        ldata = (LD*)local.addr;
        delete ldata;
        local.setAddr(Address(0));
        return 0;
      }
      return 0;
  }
  return 0;
}//end loaddata_p

//Type mapping function for the operators -loaddata-
//
ListExpr LoadDataTypeMap(ListExpr args)
{
  cout<<"program is here: LoadDataTypeMap~"<<endl;
//  return args;
  if( nl->ListLength( args ) == 1)
  {
    ListExpr arg1 = nl->First(args);
    if(nl->SymbolValue(arg1) == "string")
    {
      ListExpr res = nl->TwoElemList(
          nl->SymbolAtom("stream"),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(
                nl->SymbolAtom("Id"),
                nl->SymbolAtom("int")),
              nl->TwoElemList(
                nl->SymbolAtom("Trip"),
                nl->SymbolAtom("mpoint"))
              )));

      cout<<"program is here: LoadDataTypeMap return res~"<<endl;
      return res;
    }
  }
/*  NList type(args);
  const string errMsg = "Expecting a string";
  if(type == NList(CcString::BasicType()))
  {
return
  }
*/
  return nl->SymbolAtom( Symbol::TYPEERROR());
}
/////////////////////////////////////////////////////////////////////////////

struct LoadTrajLocal
{
    list<char*>     *files;
    int             curid;
    TupleType       *resulttype;
    vector<MPoint*> mpoints;
};


//value mapping function of operator~loaddata~
//
int LoadTrajectoryValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    LoadTrajLocal *ldata;
    MPoint        *cartrip, *restrip;
    char          *fntmp = NULL;

    //cout<<"program is here: value map~"<<endl;
    switch(message)
    {
        case OPEN:
        {
            //cout<<"program is here: value map OPEN~"<<endl;
            ldata = new LoadTrajLocal;
            ldata->curid = 1;
            char *filename = new char[128];
            string path_name  = ((CcString*)args[0].addr)->GetValue();
            ldata->resulttype = new TupleType(nl->Second(GetTupleResultType(s)));
            strcpy(filename,path_name.c_str());
            //get all the files
            ldata->files = getAllFileList(filename);
            if(ldata->files == NULL){
                cout<<"Can not open the file or directory "<< filename <<endl;
        	return CANCEL;
    	    }
            delete [] filename;
            
            local.setAddr(ldata);
            return 0;
        }
        case REQUEST:
        {
            //cout<<"program is here: value map REQUEST~"<<endl;
            if(local.addr == NULL)
                return CANCEL;
            ldata = (LoadTrajLocal*)local.addr;
            if(ldata->resulttype == NULL || (ldata->mpoints.empty() && ldata->files->empty())){
                return CANCEL;
            }
            
            while(ldata->mpoints.empty()){
                //cout<<"Warning: mpoins is null!"<<endl;
                fntmp = ldata->files->front();
                ldata->files->pop_front();
                if(strContain(fntmp, ".plt")){
                    genMPoint2(fntmp, ldata->mpoints);
                }else if(strContain(fntmp, ".txt")){
                    genMPoint3(fntmp, ldata->mpoints);
                }else{
                    cout<<fntmp<<"\nERROR: the file may be not fit the data loading!"<<endl;
                }
            }
            if(ldata->mpoints.empty() && ldata->files->empty())
            {
                return CANCEL;
            }
            cartrip = ldata->mpoints.back();
            ldata->mpoints.pop_back();

            //restrip = new MPoint(0);
            //cartrip->gk(39,*restrip);
        
            //cartrip->DeleteIfAllowed();
            restrip = cartrip;

            //create the tuple
            Tuple * tuple = new Tuple(ldata->resulttype);

            //put record to tuple
            tuple->PutAttribute(0, new CcInt(true, ldata->curid));
            tuple->PutAttribute(1, restrip);
            result.setAddr(tuple);
            ldata->curid ++;
            return YIELD;
        }
        case CLOSE:
        {
            //cout<<"program is here: value map CLOSE~"<<endl;
            ldata = (LoadTrajLocal*)local.addr;
            freeFileList(ldata->files);
            delete ldata;
            local.setAddr(Address(0));
            return 0;
        }
        return 0;
    }
    return 0;
}//end loadtrajectory_p

//////////////////////////////////////////////////////////////////////////////
//Type mapping function for the operators -loaddata-
//
ListExpr LoadUploadUnitTypeMap(ListExpr args)
{
  cout<<"program is here: LoadUploadUnitTypeMap~"<<endl;
//  return args;
  if( nl->ListLength( args ) == 1)
  {
    ListExpr arg1 = nl->First(args);
    if(nl->SymbolValue(arg1) == "string")
    {
      ListExpr res = nl->TwoElemList(
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

      cout<<"program is here: LoadUploadUnitTypeMap return res~"<<endl;
      return res;
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR());
}

//
//
//
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
//
//
int LoadUploadUnitValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    LoadUploadUnitLocal *luul = NULL;
    GPSRecord           gr;
    DateTime            dt(instanttype);
    UploadUnit          *uu;
    UnitPos             up;
    Tuple               *tuple;

    //cout<<"program is here: begin of LoadUploadUnitValueMap!"<<endl;

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


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class BreakUpLocalInfo
{
public:
    BreakUpInfo *bui;
    TupleType   *resulttype;
};

//value mapping function of operator ~BreakUp~
//
//
int BreakUpValueMap(Word *args, Word &result, int message, Word &local, Supplier s)
{
    Word wTuple;
    BreakUpLocalInfo *buli;// = new BreakUpLocalInfo();
    //BreakUpInfo *breakupinfo;
    MPoint *tempmpoint;
    
    switch(message)
    {
        case OPEN:
            {
                buli = new BreakUpLocalInfo();
                buli->bui = new BreakUpInfo();
                ListExpr rt = GetTupleResultType(s);
                TupleType *tupleType = new TupleType(nl->Second(rt));
                buli->resulttype = tupleType;
                //breakupinfo = new BreakUpInfo();
                local.addr = buli;
                qp->Open(args[0].addr);
                return 0;
            }
        case REQUEST:
            {
                if(!local.addr)
                {
                    return CANCEL;
                }
                //cout<<"Program is here: value map!"<<endl;
                buli = (BreakUpLocalInfo*)local.addr;
                while(buli->bui->IsEmpty())
                {
                    qp->Request(args[0].addr, wTuple);
                    if(!qp->Received(args[0].addr))
                    {
                        return CANCEL;
                    }
                    buli->bui->Init((Tuple*)wTuple.addr);
                }
                //cout<<"Program is here: end while!"<<endl;
                Tuple *t = buli->bui->tuple->Clone();
                tempmpoint = buli->bui->GetNextMPoint();
                if(tempmpoint == NULL)
                {
                    //
                    cout<<"the result is null!"<<endl;
                    tempmpoint = new MPoint(0);
                }
                //cout<<"Program is here: before put attribute!"<<endl;
                //t->PutAttribute(3, (Attribute*)tempmpoint);
                //t->PutAttribute(4, (Attribute*)(new CcInt(buli->bui->GetSId())));
                Tuple *tupleresult = new Tuple(buli->resulttype);
                tupleresult->PutAttribute(0, t->GetAttribute(0));
                tupleresult->PutAttribute(1, t->GetAttribute(1));
                tupleresult->PutAttribute(2, t->GetAttribute(2));
                tupleresult->PutAttribute(3, (Attribute*)tempmpoint);
                tupleresult->PutAttribute(4, (Attribute*)(new CcInt(buli->bui->GetSId())));
                result.setAddr(tupleresult);
                return YIELD;
            }
        case CLOSE:
            {
                if(local.addr)
                {
                    buli = (BreakUpLocalInfo*)local.addr;
                    if(buli->bui)
                    {
                        delete buli->bui;
                    }
                }
                qp->Close(args[0].addr);
                return 0;
            }
    }
    return 0;
}


//type mapping function of operator ~BreakUp~
//
//
ListExpr BreakUpTypeMap(ListExpr args)
{
    //cout<<"program is here: BreakUpTypeMap"<<endl;

    ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Line"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Up"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Trip"),
                        nl->SymbolAtom("mpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Segid"),
                        nl->SymbolAtom("int"))
                    )));
    if(nl->ListLength(args) == 1)
    {
        
        ListExpr arg1 = nl->First(args);
        if(IsStreamDescription(arg1))
        {

            return res;
        }
    }
    return nl->SymbolAtom(Symbol::TYPEERROR());
}

//
//
const string BreakUP = 
  "((\"Signature\"\"Syntax\"\"Meaning\"\"Example\")"
  "(<text>stream(rel()) -> stream(rel())</text--->"
  "<text>_ breakup </text--->"
  "<text>break up the trajectories of objects</text--->"
  "<text>query Trains feed breakup consume</text--->"
  "))";

//
//
const string Loadfiledata = 
  "((\"Signature\"\"Syntax\"\"Meaning\"\"Example\")"
  "(<text>file -> stream(rel())</text--->"
  "<text>loaddata( _ )</text--->"
  "<text>load data from file</text--->"
  "<text>query loaddata( filename )</text--->"
  "))";


//
//
Operator loaddatafile(
    "loaddata",
    Loadfiledata,
    LoadDataValueMap,
    Operator::SimpleSelect,
    LoadDataTypeMap );


//
// type map for the ~trajectorysplit~
//     stream x real x int-> stream
//
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

    1.operator LoadTrajData

***************************************************************/

// type map function
//
ListExpr LoadTrajDataTM(ListExpr args)
{
    return nl->Empty();
}

//value map function
//
int LoadTrajDataVM(Word* args, Word& result, int message, Word& local, Supplier s)
{

    switch( message )
    {
        case OPEN:
            {
            }
        case REQUEST:
            {
            }
        case CLOSE:
            {
                return 0;
            }
    }
    return 0;
}

//
// operator info
struct LoadTrajDataInfo : OperatorInfo {
    LoadTrajDataInfo()
    {
        name      = "loadtrajdata";
        signature = "string -> stream (tuple (Upload uploadunit))";
        syntax    = "loadtrajdata ( _ )";
        meaning   = "load trajectory data from file to a stream tuple uploadunits";
    }
};

/****************************************************************

    2.operator ConvertUU2UP

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

    3.operator MeanFilter

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

    4.operator MedianFilter

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

    5.operator ConvertUP2MP

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
//
//
class LoadAlgebra : public Algebra
{
public:
  LoadAlgebra() : Algebra()
  {
    AddOperator(loaddataInfo(), LoadDataValueMap, LoadDataTypeMap);
    AddOperator(breakupInfo(), BreakUpValueMap, BreakUpTypeMap);
    AddOperator(trasplitInfo(), TrajectorySplitValueMap, TrajectorySplitTypeMap);
    AddOperator(sizetest1Info(), SizeTest1ValueMap, SizeTest1TypeMap);
    AddOperator(sizetest2Info(), SizeTest2ValueMap, SizeTest2TypeMap);
    AddOperator(loadtrajectoryInfo(), LoadTrajectoryValueMap, LoadDataTypeMap);
    AddOperator(loadUploadUnitInfo(), LoadUploadUnitValueMap, LoadUploadUnitTypeMap);
    AddOperator(ConvertUU2UPInfo(), ConvertUU2UPVM, ConvertUU2UPTM);
    AddOperator(ConvertUP2MPInfo(), ConvertUP2MPVM, ConvertUP2MPTM);
    AddOperator(MeanFilterInfo(), MeanFilterVM, MeanFilterTM);
    AddOperator(MedianFilterInfo(), MedianFilterVM, MeanFilterTM);
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
