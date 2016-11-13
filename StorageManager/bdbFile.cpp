/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Implementation of SmiFile using the Berkeley-DB

April 2002 Ulrich Telle

September 2002 Ulrich Telle, missing fileId in Open method fixed

February 2003 Ulrich Telle, adjusted for Berkeley DB version 4.1.25

April 2003 Ulrich Telle, implemented temporary SmiFiles

August 2004 M. Spiekermann. New private function ~CheckDbHandles~ introduced.
Since files are closed only when an enclosing transaction is finished, 
reallocation of DbHandles is only done when necessary, e.g. when an instance 
op SmiFile should be reused after ~close~ and ~create~ and ~open~ is called 
again.

January 2005 M.Spiekermann. Changes in the Implementation of the 
PrefetchingIterator.  Since Berkeley DB 4.2.52 does not support any longer 
the bulk retrieval macros with parameters of class ~Dbt~, a reference to
 the C-API struct ~DBT~ will be passed now.
This code also compiles with version 4.1.25 of Berkeley-DB.

*/

using namespace std;

#include <string.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <cassert>
#include <limits.h>

#undef TRACE_ON
#include "Trace.h"
#include "LogMsg.h"

#include <db_cxx.h>
#include "DbVersion.h"
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "CharTransform.h"
#include "Counter.h"
#include "WinUnix.h"

static int  BdbCompareInteger( Db* dbp, const Dbt* key1, const Dbt* key2 );
static int  BdbCompareLongint( Db* dbp, const Dbt* key1, const Dbt* key2 );
static int  BdbCompareFloat( Db* dbp, const Dbt* key1, const Dbt* key2 );
//static void BdbInitCatalogEntry( SmiCatalogEntry& entry );


/* --- Implementation of class SmiFile --- */

string lu_2_s(uint32_t value)
{
  ostringstream os;
  os << value;
  return os.str();
}
/*
Converts a u\_int32\_t value to string (somehow, << does not work directly)

*/

ostream& operator<<(ostream& os, const SmiFile& f)
{
  return f.Print(os);
}


SmiFile::Implementation::Implementation()
{
  bdbHandle =  0; // SmiEnvironment::Implementation::AllocateDbHandle();
  bdbFile   = 0; //SmiEnvironment::Implementation::GetDbHandle( bdbHandle );
  noHandle = true;
  bdbName = "undefined";
/*
The constructor cannot allocate a Berkeley DB handle by itself since handles
must stay open until the enclosing transaction has been terminated, that is
the handle must be capable to survive this SmiFile instance.

*/
  isSystemCatalogFile = false;
  isTemporaryFile = false;
}

SmiFile::Implementation::Implementation( bool isTemp )
{
  bdbHandle = 0;
  bdbFile   = new Db( SmiEnvironment::Implementation::GetTempEnvironment(),
                      DB_CXX_NO_EXCEPTIONS );
  noHandle = false;
  bdbName = "undefined";
/*
The constructor cannot allocate a Berkeley DB handle by itself since handles
must stay open until the enclosing transaction has been terminated, that is
the handle must be capable to survive this SmiFile instance.

*/
  isSystemCatalogFile = false;
  isTemporaryFile = true;
}

SmiFile::Implementation::~Implementation()
{
  if ( !isTemporaryFile )
  {
    SmiEnvironment::Implementation::FreeDbHandle( bdbHandle );
/*
The destructor flags the handle as not used any more. After the termination
of the enclosing transaction the handle will be closed.

*/
  }
  else
  {
    if (bdbFile) {
      int rc = bdbFile->close( 0 );
      SmiEnvironment::SetBDBError(rc);
      delete bdbFile;
      bdbFile = 0;
    }
  }
}



void
SmiFile::Implementation::CheckDbHandles() {

  static long& ctr = Counter::getRef("SmiFile:Realloc-DBHandles");
  if ( !isTemporaryFile && noHandle ) { // reallocate a DbHandle if necessary

    ctr++;
    bdbHandle = SmiEnvironment::Implementation::AllocateDbHandle();
    bdbFile   = SmiEnvironment::Implementation::GetDbHandle( bdbHandle );
    noHandle = false;
  }
}



SmiFile::SmiFile( const bool isTemporary /* = false */)
  : opened( false ),
    fileContext( "" ),
    fileName( "" ),
    fileId( 0 ),
    fixedRecordLength( 0 ),
    uniqueKeys( true ),
    keyDataType( SmiKey::Unknown )
{
  trace = RTFlag::isActive("SMI:traceHandles") ? true : false;
  useTxn = SmiEnvironment::useTransactions;
  if ( !isTemporary )
  {
    impl = new Implementation();
  }
  else
  {
    impl = new Implementation( true );
  }
}


SmiFile::SmiFile( const SmiFile& f)
  : opened( f.opened ),
    fileContext( f.fileContext ),
    fileName( f.fileName ),
    fileId( f.fileId ),
    fixedRecordLength( f.fixedRecordLength ),
    uniqueKeys( f.uniqueKeys ),
    keyDataType( f.keyDataType )
{
  trace = RTFlag::isActive("SMI:traceHandles") ? true : false;
  useTxn = SmiEnvironment::useTransactions;

  // SPM: What should we do here deep copy or copy by
  // reference?
  impl = f.impl;

  /*
  if ( !isTemporary )
  {
    impl = new Implementation();
  }
  else
  {
    impl = new Implementation( true );
  }
  */
}


SmiFile::~SmiFile()
{
  delete impl;
}


bool
SmiFile::IsTemp(){
  return impl->IsTemp();
}

bool
SmiFile::CheckName( const string& name )
{
  static string alpha( "abcdefghijklmnopqrstuvwxyz" );
  static string alnum( alpha + "0123456789_" );
  string temp = name;
  bool ok = false;

  if ( temp.length() > 0 )
  {
    transform( temp.begin(), temp.end(), temp.begin(), ToLowerProperFunction );
    string::size_type pos = temp.find_first_not_of( alnum );
    ok = pos == string::npos &&
         name[0] != '_' &&
         name.length() < SMI_MAX_NAMELEN;
  }

  return (ok);
}




bool
SmiFile::Create( const string& name,
		 const string& context /* = "Default" */,
		 const uint16_t ps /* = 0 */,
     const bool keepId /*=false*/ )
{
  static long& ctrCreate = Counter::getRef("SmiFile::Create");
  static long& ctrOpen = Counter::getRef("SmiFile::Open");
  int rc = 0;
  impl->CheckDbHandles();

  if ( CheckName( context ) )
  {
    if(!keepId){
      fileId = SmiEnvironment::Implementation::GetFileId(
                                                    impl->isTemporaryFile );
    }
    if ( fileId != 0 )
    {
      string bdbName = name;
      if (name=="") {
        bdbName = SmiEnvironment::
	             Implementation::ConstructFileName( fileId,
                                                        impl->isTemporaryFile );
      }

      impl->bdbName = bdbName;
      fileName = bdbName;
      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary

      DBTYPE bdbType;
      switch ( fileType )
      {
        case KeyedBtree:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError(rc);
          }
          if ( keyDataType == SmiKey::Integer )
          {
            rc = impl->bdbFile->set_bt_compare((bt_compare_fcn_type) 
                                                BdbCompareInteger );
            SmiEnvironment::SetBDBError(rc);
          }else  if ( keyDataType == SmiKey::Longint )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type)
                                                 BdbCompareLongint );
            SmiEnvironment::SetBDBError(rc);
          }
          else if ( keyDataType == SmiKey::Float )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type)
                                                 BdbCompareFloat );
            SmiEnvironment::SetBDBError(rc);
          }
          break;
        case KeyedHash:
          bdbType = DB_HASH;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError(rc);
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          rc = impl->bdbFile->set_re_len( fixedRecordLength );
          SmiEnvironment::SetBDBError(rc);
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize = 0;
      if (ps > 0) {
        pagesize = ps;
      } else {
        pagesize = SmiProfile::GetParameter( context, "PageSize", 0,
                                             SmiEnvironment::configFile );
      }

      if ( pagesize > 0 )
      {
          rc = impl->bdbFile->set_pagesize( pagesize );
          SmiEnvironment::SetBDBError(rc);

//           cout << "Setting page size for SmiFile to "
//                << pagesize << " !" << endl;
      }


      // --- Open Berkeley DB file

      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      u_int32_t dirtyFlag = useTxn ? DB_DIRTY_READ : 0;
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      if(tid){
        commitFlag=0;
      }
      u_int32_t flags = (!impl->isTemporaryFile) ?
                           DB_CREATE | dirtyFlag | commitFlag : DB_CREATE;
      rc = impl->bdbFile->open( tid, bdbName.c_str(), 0, bdbType, flags, 0 );
      if (trace)
        cerr << "Creating " << *this << endl;
      if ( rc == 0 )
      {
        ctrCreate++;
        ctrOpen++;
        SmiDropFilesEntry entry(fileId, false);
        SmiEnvironment::instance.impl->bdbFilesToDrop.push( entry );
        opened      = true;
        fileName    = bdbName;
        fileContext = context;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetBDBError( rc );
      }
    }
    else
    {
      rc = E_SMI_FILE_NOFILEID;
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
    }
  }
  else
  {
    rc = E_SMI_FILE_INVALIDNAME;
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }

  return (rc == 0);
}


bool
SmiFile::Create( const string& context /* = "Default" */,
		 const uint16_t ps /* = 0 */ )
{
  return Create("", context, ps);
}

bool SmiFile::ReCreate(){
   uint16_t ps = GetPageSize();

   DbEnv* env(0);
   uint16_t closeFlags;
   if(impl->isTemporaryFile){
       env = SmiEnvironment::instance.impl->tmpEnv;
       closeFlags = DB_NOSYNC;
   } else {
       env = SmiEnvironment::instance.impl->bdbEnv;
       closeFlags = 0;
   }

   if(opened){
     int rc = impl->bdbFile->close(closeFlags);
     if(rc){
        SmiEnvironment::SetBDBError( rc );
        return false;
     }
     opened = false;
     // after closing, the old db handle is invalid, create a new one
     delete impl->bdbFile;
     impl->bdbFile = new Db(env, DB_CXX_NO_EXCEPTIONS );
   }



   int rc = impl->bdbFile->remove(impl->bdbName.c_str(), 0, 0 );
   if(rc){
      SmiEnvironment::SetBDBError( rc );
      return false;
   }

   // after removing, the old db handle is invalid, create a new one
   delete impl->bdbFile;
   impl->bdbFile = new Db(env, DB_CXX_NO_EXCEPTIONS );

   return  Create(fileName,fileContext, ps, true);
}


/*
Opens a file by name. This should be used only by catalog files!

*/
bool
SmiFile::Open( const string& name, const string& context /* = "Default" */ )
{
  if(opened){
    cerr << "try to open an open  file: " << (*this) << endl;
    return true;
  }
  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;
  bool existing = false;
  //impl->CheckDbHandles(); // allocating dbhandles moved

  if ( impl->isTemporaryFile )
  {
    rc = E_SMI_FILE_ISTEMP;
    SmiEnvironment::SetError( E_SMI_FILE_ISTEMP );
  }
  else if ( CheckName( context ) && CheckName( name ) )
  {
    SmiCatalogEntry entry;
    string newName = context + '.' + name;

    if ( SmiEnvironment::Implementation::LookUpCatalog( newName, entry ) )
    {
      // --- File found in permanent file catalog
      fileId = entry.fileId;
      existing = true;
    }
    else
    {
      // --- Check whether a file with the given name was created
      // --- earlier within the enclosing transaction
      map<string,SmiCatalogFilesEntry>::iterator it =
        SmiEnvironment::instance.impl->bdbFilesToCatalog.find( newName );
      if ( it != SmiEnvironment::instance.impl->bdbFilesToCatalog.end() &&
           it->second.updateOnCommit )
      {
        fileId = it->second.entry.fileId;
        existing = true;
      }
      else
      {
        fileId = SmiEnvironment::Implementation::GetFileId();
      }
    }

    if ( fileId != 0 )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( fileId );

      impl->bdbName = bdbName;
      fileName = bdbName;
      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary
      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      u_int32_t dirtyFlag = useTxn ? DB_DIRTY_READ : 0;
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      if(tid){
        commitFlag=0;
      }
      u_int32_t openFlags = DB_CREATE | dirtyFlag | commitFlag;

      int alreadyExisting =
          SmiEnvironment::Implementation::FindOpen(fileName,openFlags);
      if(alreadyExisting>=0){
         DbHandleIndex old = impl->bdbHandle;
         if(!impl->noHandle){
             SmiEnvironment::Implementation::SetUnUsed(old);
         }
         impl->bdbHandle = alreadyExisting;
         impl->bdbFile =
               SmiEnvironment::Implementation::GetDbHandle(impl->bdbHandle);
         SmiEnvironment::Implementation::SetUsed(impl->bdbHandle);
         opened = true;
         impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
         return true;
      }

      impl->CheckDbHandles();

      DBTYPE bdbType;
      switch ( fileType )
      {
        case KeyedBtree:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError( rc );
          }
          if ( keyDataType == SmiKey::Integer )
          {
            rc = impl->bdbFile->set_bt_compare((bt_compare_fcn_type) 
                                                BdbCompareInteger );
            SmiEnvironment::SetBDBError( rc );
          }else if ( keyDataType == SmiKey::Longint )
          {
            rc = impl->bdbFile->set_bt_compare((bt_compare_fcn_type) 
                                                BdbCompareLongint );
            SmiEnvironment::SetBDBError( rc );
          }
          else if ( keyDataType == SmiKey::Float )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type) 
                                                 BdbCompareFloat );
            SmiEnvironment::SetBDBError( rc );
          }
          break;
        case KeyedHash:
          bdbType = DB_HASH;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError(rc);
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          rc = impl->bdbFile->set_re_len( fixedRecordLength );
          SmiEnvironment::SetBDBError( rc );
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize =
        SmiProfile::GetParameter( context, "PageSize", 0,
                                  SmiEnvironment::configFile );
      if ( pagesize > 0 )
      {
        rc = impl->bdbFile->set_pagesize( pagesize );
        SmiEnvironment::SetBDBError( rc );
      }

      // --- Open Berkeley DB file

      rc = impl->bdbFile->open( tid, bdbName.c_str(),
                                0, bdbType,
                                openFlags, 0 );

      if (trace)
        cerr << "opening by name = " << name << ", " << *this << endl;

      if ( rc == 0 )
      {
        if ( !existing )
        {
          // --- Register newly created file, since it has to be dropped
          // --- when the enclosing transaction is aborted
          SmiDropFilesEntry dropEntry(fileId, false);
          SmiEnvironment::instance.impl->bdbFilesToDrop.push( dropEntry );
          SmiCatalogFilesEntry catalogEntry;
          //BdbInitCatalogEntry( catalogEntry.entry );
          catalogEntry.entry.fileId = fileId;
          newName.copy( catalogEntry.entry.fileName, 2*SMI_MAX_NAMELEN+1 );
          catalogEntry.entry.isKeyed = fileType == KeyedBtree ||
                                       fileType == KeyedHash;
          catalogEntry.entry.isFixed = fileType == FixedLength;
          catalogEntry.updateOnCommit = true;
          SmiEnvironment::instance.impl->bdbFilesToCatalog[newName]
                                                           = catalogEntry;
        }
        ctr++;
        opened      = true;
        fileName    = name;
        fileContext = context;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetBDBError( rc );
      }
    }
    else
    {
      rc = E_SMI_FILE_NOFILEID;
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
    }
  }
  else
  {
    rc = E_SMI_FILE_INVALIDNAME;
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }

  return (rc == 0);
}

bool
SmiFile::Open( const SmiFileId fileid, const string& context /* = "Default" */ )
{
  if(opened){
    cerr << "try to open an open file" << (*this) << endl;
    return true;
  }

  static long& ctr = Counter::getRef("SmiFile::Open");
  TRACE("SmiFile::Open")
  int rc = 0;
  // impl->CheckDbHandles(); memory allocated later

  if ( CheckName( context ) )
  {
    SmiCatalogEntry entry;
    if ( impl->isTemporaryFile )
    {
      fileContext = context;
      fileName    = "";
    }
    else if ( SmiEnvironment::Implementation::LookUpCatalog( fileid, entry ) )
    {
      // SPM: should never work!
      assert(false);
      // --- File found in permanent file catalog
      char* point = strchr( entry.fileName, '.' );
      *point = '\0';
      fileContext = entry.fileName;
      fileName = ++point;
    }
    else
    {
      //cerr << "INFO: fileId not found in fileCatalog" << endl;
      fileContext = context;
      fileName    = "";
    }

    if ( fileid != 0 && fileContext == context )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName(
                                                      fileid,
                                                      impl->isTemporaryFile );

      impl->bdbName = bdbName;
      fileName = bdbName;
      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      u_int32_t dirtyFlag = useTxn ? DB_DIRTY_READ : 0;
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      if(tid){
        commitFlag=0;
      }
      u_int32_t flags = (!impl->isTemporaryFile) ?
                            dirtyFlag | commitFlag : 0;

      int alreadyExist =
          SmiEnvironment::Implementation::FindOpen(bdbName,flags);
      if(alreadyExist>=0){

         if (trace) {
           cerr << "File id =" << fileid << "already exists." << endl;
          }

	 DbHandleIndex old = impl->bdbHandle;
         if(!impl->noHandle){
             SmiEnvironment::Implementation::SetUnUsed(old);
         }
         impl->bdbHandle = alreadyExist;
         impl->bdbFile =
               SmiEnvironment::Implementation::GetDbHandle(impl->bdbHandle);
         SmiEnvironment::Implementation::SetUsed(impl->bdbHandle);
         opened = true;
         impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
	 fileId = fileid;
         return true;
      }
      impl->CheckDbHandles();
      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary

      DBTYPE bdbType;
      switch ( fileType )
      {
        case KeyedBtree:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError( rc );
          }
          if ( keyDataType == SmiKey::Integer )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type) 
                                                 BdbCompareInteger );
            SmiEnvironment::SetBDBError( rc );
          } else  if ( keyDataType == SmiKey::Longint )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type) 
                                                 BdbCompareLongint );
            SmiEnvironment::SetBDBError( rc );
          }
          else if ( keyDataType == SmiKey::Float )
          {
            rc = impl->bdbFile->set_bt_compare( (bt_compare_fcn_type) 
                                                 BdbCompareFloat );
            SmiEnvironment::SetBDBError( rc );
          }
          break;
        case KeyedHash:
          bdbType = DB_HASH;
          if ( !uniqueKeys )
          {
            rc = impl->bdbFile->set_flags( DB_DUP );
            SmiEnvironment::SetBDBError(rc);
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          rc = impl->bdbFile->set_re_len( fixedRecordLength );
          SmiEnvironment::SetBDBError( rc );
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize =
        SmiProfile::GetParameter( context, "PageSize", 0,
                                  SmiEnvironment::configFile );
      if ( pagesize > 0 )
      {
        rc = impl->bdbFile->set_pagesize( pagesize );
        SmiEnvironment::SetBDBError( rc );
      }

      // --- Open Berkeley DB file


      rc = impl->bdbFile->open( tid, bdbName.c_str(), 0, bdbType, flags, 0 );
      fileId = fileid;
      if (trace) {
        cerr << "opening by id =" << fileid << ", "<< *this << endl;
      }
      SmiEnvironment::SetBDBError( rc );
      if ( rc == 0 )
      {
        opened = true;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }

    }
    else
    {
      rc = (fileid == 0) ? E_SMI_FILE_NOFILEID : E_SMI_FILE_BADCONTEXT;

      SmiEnvironment::SetError( rc );
    }
  }
  else
  {
    rc = E_SMI_FILE_INVALIDNAME;
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }

  if( rc == 0 )
  {
    ctr++;
    return true;
  }
  else {
    fileId = ULONG_MAX;
    return false;
  }
}

bool
SmiFile::Close( const bool sync /* = true */)
{
  static long& ctr = Counter::getRef("SmiFile::Close");
  TRACE("SmiFile::Close")
  int rc = 0;

  if ( opened )
  {
    ctr++;
    // --- The current Berkeley DB handle is freed, but a new one is
    // --- allocated for possible reuse of this SmiFile instance
    opened = false;
    if ( !impl->isTemporaryFile )
    {
      SmiEnvironment::Implementation::FreeDbHandle( impl->bdbHandle );
      impl->noHandle = true;
    //  if(impl->bdbFile){
    //    impl->bdbFile->close(0);
    //    delete impl->bdbFile;
    //    impl->bdbFile=0;
    //    impl->noHandle=true;
    //    SmiEnvironment::Implementation::DeleteDbHandle( impl->bdbHandle );
    //  }
    }
    else
    {
      uint32_t flags = sync?0:DB_NOSYNC;
      rc = impl->bdbFile->close( flags );
      if (trace)
        cerr << "closing " << *this << endl;
      SmiEnvironment::SetBDBError( rc );
      delete impl->bdbFile;

      impl->bdbFile = new Db( SmiEnvironment::instance.impl->tmpEnv,
                              DB_CXX_NO_EXCEPTIONS );
    }
    impl->isSystemCatalogFile = false;
  }

  return (rc == 0);
}

bool
SmiFile::Drop()
{
  bool ok = Close();
  if ( ok && !impl->isTemporaryFile )
  {
    // --- Register SmiFile for real dropping after
    // --- successfully committing the enclosing transaction
    SmiDropFilesEntry dropEntry(fileId, true);
    SmiEnvironment::instance.impl->bdbFilesToDrop.push( dropEntry );

    /* FIXME : commented out because the persistent version
               does not work otherwise
    if ( fileName.length() > 0 )
    {
      SmiCatalogFilesEntry catalogEntry;
      string newName = fileContext + '.' + fileName;
      BdbInitCatalogEntry( catalogEntry.entry );
      catalogEntry.entry.fileId = fileId;
      newName.copy( catalogEntry.entry.fileName, 2*SMI_MAX_NAMELEN+1 );
      catalogEntry.entry.isKeyed = fileType == KeyedBtree ||
                                   fileType == KeyedHash;
      catalogEntry.entry.isFixed = fileType == FixedLength;
      catalogEntry.updateOnCommit = false;
      SmiEnvironment::instance.impl->bdbFilesToCatalog[newName] = catalogEntry;
    }*/
  }
  return (ok);
}

bool
SmiFile::Truncate()
{
  DbTxn* tid = !impl->isTemporaryFile ?
                  SmiEnvironment::instance.impl->usrTxn : 0;

  u_int32_t countp = 0;
  int rc = impl->bdbFile->truncate( tid, &countp, 0 );
  SmiEnvironment::SetBDBError(rc);
  return rc == 0;
}

bool
SmiFile::Remove()
{
  //cerr << endl << "removing " <<  impl->bdbName << endl;

  int rc = 0;
  if(opened){
    if(! Close(false)){
      return false;
    }
  }
  rc = impl->bdbFile->remove( impl->bdbName.c_str(), 0, 0 );
  SmiEnvironment::SetBDBError(rc);
  if ( rc == 0 ) {
    if(impl->bdbFile){
       delete impl->bdbFile;
       impl->bdbFile = 0;
    }
  }
  //cerr << endl << "End removing " <<  impl->bdbName << endl;
  return rc == 0;
}

string
SmiFile::GetContext()
{
  return (fileContext);
}

string
SmiFile::GetName()
{
  return (fileName);
}

SmiFileId
SmiFile::GetFileId()
{
  return (fileId);
}

uint16_t
SmiFile::GetPageSize() const
{
  u_int32_t pageSize;
  int rc = impl->bdbFile->get_pagesize( &pageSize );
  SmiEnvironment::SetError(rc);
  return pageSize;
}


bool
SmiFile::IsOpen()
{
  return (opened);
}

SmiStatResultType
  SmiFile::GetFileStatistics(const SMI_STATS_MODE mode)
{
  int getTypeReturnValue = 0;
  int getStatReturnValue = 0;
  u_int32_t flags = 0;
  DBTYPE dbtype;
  SmiStatResultType result;
  result.push_back(pair<string,string>("FileName",fileName));

  switch(mode){
    case SMI_STATS_LAZY: {
        flags = DB_FAST_STAT;
        break;
      }
    case SMI_STATS_EAGER: {
        flags = 0;
        break;
      }
    default: {
        cout << "Error in SmiBtreeFile::GetFileStatistics: Unknown "
             << "SMI_STATS_MODE" << mode << endl;
        // assert( false );
        result.push_back(pair<string,string>("ExitStatus","ERROR"));
        return result;
      }
  }
  result.push_back(pair<string,string>("StatisticsMode",
                  (mode == SMI_STATS_LAZY) ? "Lazy" : "Eager" ));

  getTypeReturnValue = impl->bdbFile->get_type(&dbtype);
  if(getTypeReturnValue != 0){
    cout << "Error in SmiFile::GetFileStatistics: get_type(...) returned != 0"
         << getTypeReturnValue << endl;
    string error;
    SmiEnvironment::GetLastErrorCode( error );
    cout << error << endl;
    // assert( false );
    result.push_back(pair<string,string>("ExitStatus","ERROR"));
    return result;
  }
  switch(dbtype){
    case DB_QUEUE:
    {
      DB_QUEUE_STAT *sRS = 0;
      // set flags according to ~mode~
      // call bdb stats method
#if DB_VERSION_REQUIRED(4,3)
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      getStatReturnValue = impl->bdbFile->stat(tid, &sRS, flags);
#else
      getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
      // check for errors
      if(getStatReturnValue != 0){
        cout << "Error in SmiFile::GetFileStatistics: stat(...) returned != 0"
            << getStatReturnValue << endl;
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
      //     assert( false );
        result.push_back(pair<string,string>("ExitStatus","ERROR"));
        return result;
      }
      // translate result structure to vector<pair<string,string> >
      result.push_back(pair<string,string>("FileType","QueueFile"));
      result.push_back(pair<string,string>("FileTypeVersion",
                lu_2_s(sRS->qs_version)));
      result.push_back(pair<string,string>("NoRecords",
                lu_2_s(sRS->qs_nkeys)));
      result.push_back(pair<string,string>("NoEntries",
                lu_2_s(sRS->qs_ndata)));
      result.push_back(pair<string,string>("PageSize",
                lu_2_s(sRS->qs_pagesize)));
      result.push_back(pair<string,string>("ExtentSize",
                lu_2_s(sRS->qs_extentsize)));
      result.push_back(pair<string,string>("NoPages",
                lu_2_s(sRS->qs_pages)));
      result.push_back(pair<string,string>("RecordLength",
                lu_2_s(sRS->qs_re_len)));
      result.push_back(pair<string,string>("PaddingByte",
                lu_2_s(sRS->qs_re_pad)));
      result.push_back(pair<string,string>("NoBytesFree",
                lu_2_s(sRS->qs_pgfree)));
      result.push_back(pair<string,string>("FirstUndeletedRecord",
                lu_2_s(sRS->qs_first_recno)));
      result.push_back(pair<string,string>("NextAvailRecordNo",
                lu_2_s(sRS->qs_cur_recno)));
      result.push_back(pair<string,string>("ExitStatus","OK"));
      free(sRS); // free result structure
      break;
    }
    case DB_RECNO:
    {
      DB_BTREE_STAT *sRS = 0;
      // set flags according to ~mode~
      // call bdb stats method
#if DB_VERSION_REQUIRED(4, 3)
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      getStatReturnValue = impl->bdbFile->stat(tid, &sRS, flags);
#else
      getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
      // check for errors
      if(getStatReturnValue != 0){
        cout << "Error in SmiFile::GetFileStatistics: stat(...) returned != 0"
            << getStatReturnValue << endl;
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
      //     assert( false );
        result.push_back(pair<string,string>("ExitStatus","ERROR"));
        return result;
      }
      // translate result structure to vector<pair<string,string> >
      result.push_back(pair<string,string>("FileType","RecNoFile"));
      result.push_back(pair<string,string>("FileTypeVersion",
                lu_2_s(sRS->bt_version)));
      result.push_back(pair<string,string>("NoUniqueKeys",
                lu_2_s(sRS->bt_nkeys)));
      result.push_back(pair<string,string>("NoEntries",
                lu_2_s(sRS->bt_ndata)));
      result.push_back(pair<string,string>("PageSize",
                lu_2_s(sRS->bt_pagesize)));
      result.push_back(pair<string,string>("MinKeyPerPage",
                lu_2_s(sRS->bt_minkey)));
      result.push_back(pair<string,string>("RecordLength",
                lu_2_s(sRS->bt_re_len)));
      result.push_back(pair<string,string>("PaddingByte",
                lu_2_s(sRS->bt_re_pad)));
      result.push_back(pair<string,string>("NoLevels",
                lu_2_s(sRS->bt_levels)));
      result.push_back(pair<string,string>("NoInternalPages",
                lu_2_s(sRS->bt_int_pg)));
      result.push_back(pair<string,string>("NoLeafPages",
                lu_2_s(sRS->bt_leaf_pg)));
      result.push_back(pair<string,string>("NoDuplicatePages",
                lu_2_s(sRS->bt_dup_pg)));
      result.push_back(pair<string,string>("NoOverflowPages",
                lu_2_s(sRS->bt_over_pg)));
      result.push_back(pair<string,string>("NoFreeListPages",
                lu_2_s(sRS->bt_free)));
      result.push_back(pair<string,string>("NoBytesFreeInternalPages",
                lu_2_s(sRS->bt_int_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeLeafPages",
                lu_2_s(sRS->bt_leaf_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeDuplicatePages",
                lu_2_s(sRS->bt_dup_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeOverflowPages",
                lu_2_s(sRS->bt_over_pgfree)));
      result.push_back(pair<string,string>("ExitStatus","OK"));
      free(sRS); // free result structure
      break;
    }
    case DB_HASH:
    {
      DB_HASH_STAT *sRS = 0;
      SmiStatResultType result;
      // call bdb stats method
#if DB_VERSION_REQUIRED(4, 3)
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      getStatReturnValue = impl->bdbFile->stat(tid, &sRS, flags);
#else
      getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
      // check for errors
      if(getStatReturnValue != 0){
        cout << "Error in SmiFile::GetFileStatistics: stat(...) returned != 0"
            << getStatReturnValue << endl;
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
      //  assert( false );
        result.push_back(pair<string,string>("ExitStatus","ERROR"));
        return result;
      }
      // translate result structure to vector<pair<string,string> >
      result.push_back(pair<string,string>("FileType","HashFile"));
      result.push_back(pair<string,string>("FileTypeVersion",
          lu_2_s(sRS->hash_version)));
      result.push_back(pair<string,string>("NoUniqueKeys",
          lu_2_s(sRS->hash_nkeys)));
      result.push_back(pair<string,string>("NoEntries",
          lu_2_s(sRS->hash_ndata)));
      result.push_back(pair<string,string>("PageSize",
          lu_2_s(sRS->hash_pagesize)));
      result.push_back(pair<string,string>("NoDesiredItemsPerBucket",
          lu_2_s(sRS->hash_ffactor)));
      result.push_back(pair<string,string>("NoBuckets",
          lu_2_s(sRS->hash_buckets)));
      result.push_back(pair<string,string>("NoFreeListPages",
          lu_2_s(sRS->hash_free)));
      result.push_back(pair<string,string>("NoBigItemPages",
          lu_2_s(sRS->hash_bigpages)));
      result.push_back(pair<string,string>("NoOverflowPages",
          lu_2_s(sRS->hash_overflows)));
      result.push_back(pair<string,string>("NoDuplicatePages",
          lu_2_s(sRS->hash_dup)));
      result.push_back(pair<string,string>("NoBytesFreeBucketPages",
          lu_2_s(sRS->hash_bfree)));
      result.push_back(pair<string,string>("NoBytesFreeBigItemPages",
          lu_2_s(sRS->hash_big_bfree)));
      result.push_back(pair<string,string>("NoBytesFreeOverflowPages",
          lu_2_s(sRS->hash_ovfl_free)));
      result.push_back(pair<string,string>("NoBytesFreeDuplicatePages",
          lu_2_s(sRS->hash_dup_free)));
      result.push_back(pair<string,string>("ExitStatus","OK"));
      free(sRS); // free result structure
      break;
    }
    case DB_BTREE: {
      DB_BTREE_STAT *sRS = 0;
      // set flags according to ~mode~
      // call bdb stats method
#if DB_VERSION_REQUIRED(4, 3)
      DbTxn* tid = !impl->isTemporaryFile ?
                    SmiEnvironment::instance.impl->usrTxn : 0;
      getStatReturnValue = impl->bdbFile->stat(tid, &sRS, flags);
#else
      getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
      // check for errors
      if(getStatReturnValue != 0){
        cout << "Error in SmiFile::GetFileStatistics: stat(...) returned != 0"
            << getStatReturnValue << endl;
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
    //     assert( false );
        result.push_back(pair<string,string>("ExitStatus","ERROR"));
        return result;
      }
      // translate result structure to vector<pair<string,string> >
      result.push_back(pair<string,string>("FileType","BtreeFile"));
      result.push_back(pair<string,string>("FileTypeVersion",
                lu_2_s(sRS->bt_version)));
      result.push_back(pair<string,string>("NoUniqueKeys",
                lu_2_s(sRS->bt_nkeys)));
      result.push_back(pair<string,string>("NoEntries",
                lu_2_s(sRS->bt_ndata)));
      result.push_back(pair<string,string>("PageSize",
                lu_2_s(sRS->bt_pagesize)));
      result.push_back(pair<string,string>("MinKeyPerPage",
                lu_2_s(sRS->bt_minkey)));
      result.push_back(pair<string,string>("RecordLength",
                lu_2_s(sRS->bt_re_len)));
      result.push_back(pair<string,string>("PaddingByte",
                lu_2_s(sRS->bt_re_pad)));
      result.push_back(pair<string,string>("NoLevels",
                lu_2_s(sRS->bt_levels)));
      result.push_back(pair<string,string>("NoInternalPages",
                lu_2_s(sRS->bt_int_pg)));
      result.push_back(pair<string,string>("NoLeafPages",
                lu_2_s(sRS->bt_leaf_pg)));
      result.push_back(pair<string,string>("NoDuplicatePages",
                lu_2_s(sRS->bt_dup_pg)));
      result.push_back(pair<string,string>("NoOverflowPages",
                lu_2_s(sRS->bt_over_pg)));
      result.push_back(pair<string,string>("NoFreeListPages",
                lu_2_s(sRS->bt_free)));
      result.push_back(pair<string,string>("NoBytesFreeInternalPages",
                lu_2_s(sRS->bt_int_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeLeafPages",
                lu_2_s(sRS->bt_leaf_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeDuplicatePages",
                lu_2_s(sRS->bt_dup_pgfree)));
      result.push_back(pair<string,string>("NoBytesFreeOverflowPages",
                lu_2_s(sRS->bt_over_pgfree)));
      result.push_back(pair<string,string>("ExitStatus","OK"));
      free(sRS); // free result structure
      break;
    } // End FileType = btree
    default: {
      result.push_back(pair<string,string>("FileType","UNKNOWN"));
      result.push_back(pair<string,string>("ExitStatus","OK"));
    }
  } // end switch(dbtype)
  return result;
}

ostream&
SmiFile::Print(ostream& os) const
{
  os << "Smifile = (" << fileId << ", " << impl->bdbName << ")";
  return os;
}

// --- Key comparison function for integer keys

static int BdbCompareInteger( Db* dbp, const Dbt* key1, const Dbt* key2 )
{
  int ret;
  int32_t d1, d2;
  memcpy( &d1, key1->get_data(), sizeof(int32_t) );
  memcpy( &d2, key2->get_data(), sizeof(int32_t) );

  if ( d1 < d2 )
  {
    ret = -1;
  }
  else if ( d1 > d2 )
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}

static int BdbCompareLongint( Db* dbp, const Dbt* key1, const Dbt* key2 )
{
  int ret;
  int64_t d1, d2;
  memcpy( &d1, key1->get_data(), sizeof(int64_t) );
  memcpy( &d2, key2->get_data(), sizeof(int64_t) );

  if ( d1 < d2 )
  {
    ret = -1;
  }
  else if ( d1 > d2 )
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}


// --- Key comparison function for floating point keys

static int BdbCompareFloat( Db* dbp, const Dbt* key1, const Dbt* key2 )
{
  int ret;
  double d1, d2;
  memcpy( &d1, key1->get_data(), sizeof(double) );
  memcpy( &d2, key2->get_data(), sizeof(double) );

  if ( d1 < d2 )
  {
    ret = -1;
  }
  else if ( d1 > d2 )
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}

// --- Initialize Catalog Entry ---

 /*
static void BdbInitCatalogEntry( SmiCatalogEntry& entry )
{
  memset( entry.fileName, 0 , (2*SMI_MAX_NAMELEN+2) );
}
  */

// --- Implementation of class SmiFileIterator ---

SmiFileIterator::Implementation::Implementation()
  : bdbCursor( 0 )
{
}

SmiFileIterator::Implementation::~Implementation()
{
}

SmiFileIterator::SmiFileIterator()
  : solelyDuplicates( false ), ignoreDuplicates( false ),
    smiFile( 0 ), endOfScan( true ), opened( false), writable( false ),
    restart( true ), rangeSearch( false ), searchKey( 0 )
{
  impl = new Implementation();
}

SmiFileIterator::~SmiFileIterator()
{
  TRACE_ENTER
  SHOW(opened)
  if (opened) // close cursor if necessary
    Finish();
  delete impl;
  impl = 0;
  TRACE_LEAVE
}

bool
SmiFileIterator::Next( SmiRecord& record )
{
  static long& ctr = Counter::getRef("SmiFileIterator::Next");
  ctr++;

  static char keyData[SMI_MAX_KEYLEN];
  bool ok = false;

  SHOW(opened)
  if ( opened )
  {
    Dbt key( keyData, SMI_MAX_KEYLEN );
    key.set_data( keyData );

    // --- Initialize key if required

    if ( restart && (rangeSearch || solelyDuplicates) )
    {
      memcpy( keyData, searchKey->GetAddr(), searchKey->keyLength );
      key.set_size( searchKey->keyLength );
    }
    key.set_ulen( SMI_MAX_KEYLEN );
    key.set_flags( DB_DBT_USERMEM );

    // --- Initialize data buffer

    char buffer;
    Dbt data( &buffer, 0 );
    data.set_ulen( 0 );
    data.set_flags( DB_DBT_USERMEM | DB_DBT_PARTIAL );

    // --- Find out the appropriate cursor position

    u_int32_t flags;
    if ( restart && solelyDuplicates ) flags = DB_SET;
    else if ( restart && rangeSearch ) flags = DB_SET_RANGE;
    else if ( restart )                flags = DB_FIRST;
    else if ( solelyDuplicates )       flags = DB_NEXT_DUP;
    else if ( ignoreDuplicates )       flags = DB_NEXT_NODUP;
    else                               flags = DB_NEXT;

    // --- Position the cursor to the requested record
    // --- without reading any data, then find out the
    // --- size of the record

    restart   = false;
    endOfScan = false;
    int rc = impl->bdbCursor->get( &key, &data, flags );
    if ( rc == 0 )
    {
      flags = DB_CURRENT;
      data.set_ulen( 0 );
      data.set_flags( DB_DBT_USERMEM );
      rc = impl->bdbCursor->get( &key, &data, flags );
    }

    // --- Initialize record handle if record is available
// VTA - 15.11.2005 - to compile with the new version of Berkeley DB
#if (DB_VERSION_REQUIRED(4, 3))
    if ( rc == DB_BUFFER_SMALL )
#else
    if ( rc == ENOMEM )
#endif
    {
      // the size
      assert( key.get_size() <= SMI_MAX_KEYLEN);

      if ( record.initialized )
      {
        record.Finish();
      }
      record.recordKey.SetKey( smiFile->keyDataType,
                               keyData, key.get_size() );
      record.recordSize        = data.get_size();
      record.writable          = writable;
      record.smiFile           = smiFile;
      record.impl->bdbFile     = smiFile->impl->bdbFile;
      record.impl->useCursor   = true;
      record.impl->closeCursor = false;
      record.impl->bdbCursor   = impl->bdbCursor;
      record.initialized       = true;
      ok = true;
    }
    else if ( rc == DB_NOTFOUND )
    {
      endOfScan = true;
    }
    else
    {
      SmiEnvironment::SetBDBError( rc );
    }
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::DeleteCurrent()
{
  bool ok = false;
  if ( opened )
  {
    int rc = impl->bdbCursor->del( 0 );
    SmiEnvironment::SetBDBError( rc );
    ok = (rc == 0);
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::EndOfScan()
{
  return (endOfScan);
}

bool
SmiFileIterator::Finish()
{
  bool ok = false;
  if ( opened )
  {
    opened    = false;
    writable  = false;
    endOfScan = true;
    restart   = true;
    smiFile   = 0;
    solelyDuplicates = false;
    ignoreDuplicates = false;
    int rc = impl->bdbCursor->close();
    SmiEnvironment::SetBDBError( rc );
    ok = (rc == 0);
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::Restart()
{
  bool ok = false;
  if ( opened )
  {
    restart   = true;
    endOfScan = false;
    ok        = true;
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

PrefetchingIterator::~PrefetchingIterator()
{
}

void PrefetchingIterator::CurrentKey(SmiKey& smiKey)
{
  void* addr = 0;
  SmiSize length = 0;

  GetKeyAddressAndLength(&addr, length);
  smiKey.SetKey(keyType, addr, length);
}
/*
Get a new bulk of tuples. If that is not possible due to
too little memory, the state changes to partial retrieval.

*/
bool PrefetchingIteratorImpl::NewPrefetch()
{
  //cerr << "PrefIter = " << (void*)this
  //     << ": NewPrefetch(), state = " << state << endl;
  if(state == INITIAL && (searchType == RANGE || searchType == RIGHTRANGE))
  {
    memcpy(keyBuffer, leftBoundary, leftBoundaryLength);
    keyDbt.set_size(leftBoundaryLength);
    errorCode = dbc->get(&keyDbt, &buffer, DB_SET_RANGE | DB_MULTIPLE_KEY);
  }
  else
  {
    errorCode = dbc->get(&keyDbt, &buffer, DB_NEXT | DB_MULTIPLE_KEY);
  }

  if(errorCode != 0)
  {
// VTA - 15.11.2005 - to compile with the new version of Berkeley DB
#if (DB_VERSION_REQUIRED(4, 3))
    if ( errorCode == DB_BUFFER_SMALL )
#else
    if ( errorCode == ENOMEM )
#endif
    {
      Dbt buf;
      const size_t cBufLength = 10;
      char cBuf[cBufLength];

      buf.set_data(cBuf);
      buf.set_ulen(cBufLength);
      buf.set_dlen(cBufLength);
      buf.set_doff(0);
      buf.set_flags(DB_DBT_PARTIAL | DB_DBT_USERMEM);

      if(state == INITIAL && (searchType == RANGE || searchType == RIGHTRANGE))
      {
        memcpy(keyBuffer, leftBoundary, leftBoundaryLength);
        keyDbt.set_size(leftBoundaryLength);
        errorCode = dbc->get(&keyDbt, &buf, DB_SET_RANGE);
      }
      else
      {
        errorCode = dbc->get(&keyDbt, &buf, DB_NEXT);
      }

      if(errorCode == 0)
      {
        state = PARTIAL_RETRIEVAL;
        cerr << "PrefetchingIterator - Warning: state==PARTIAL_RETRIEVAL"
             << endl;

        if(!isBTreeIterator)
          recordNumber = *((db_recno_t*)keyBuffer);

        return true;
      }
    }

    // return code DB_NOTFOUND indicates an end of scan!
    if(errorCode != DB_NOTFOUND)
    {
      SmiEnvironment::SetBDBError(errorCode);
    }

    state = BROKEN;
    //cerr << "PrefetchingIterator - Warning: state==BROKEN" << endl;
    return false;
  }


  DB_MULTIPLE_INIT( p, buffer.get_DBT() );
  state = BULK_RETRIEVAL;

  return true;
}

bool PrefetchingIteratorImpl::RightBoundaryExceeded()
{
  size_t cmpLength = 0;
  int rc = 0;
  void* key = 0;
  size_t keyLength = 0;

  int32_t keyint = 0;
  int32_t boundaryint = 0;

  int64_t keylong = 0;
  int64_t boundarylong = 0;


  double keyDouble = 0;
  double boundaryDouble = 0;

  if(searchType == ALL || searchType == RIGHTRANGE)
  {
    return false;
  }

  assert(rightBoundary != 0);

  switch(state)
  {
    case BULK_RETRIEVAL:
      key = retKey;
      keyLength = retKeyLength;
      break;

    case PARTIAL_RETRIEVAL:
      key = keyDbt.get_data();
      keyLength = keyDbt.get_size();
      break;

    case BROKEN:
      return true;

    default:
      assert(false);
  }

  /* This is analogous to SmiKey::operator> */
  switch(keyType)
  {
    case SmiKey::Integer:
      assert(keyLength == sizeof(int32_t));
      assert(rightBoundaryLength == sizeof(int32_t));
      memcpy(&keyint, key, keyLength);
      memcpy(&boundaryint, rightBoundary, keyLength);
      if(keyint > boundaryint)
      {
        return true;
      }
      else
      {
        return false;
      };
    
    case SmiKey::Longint:
      assert(keyLength == sizeof(int64_t));
      assert(rightBoundaryLength == sizeof(int64_t));
      memcpy(&keylong, key, keyLength);
      memcpy(&boundarylong, rightBoundary, keyLength);
      if(keylong > boundarylong)
      {
        return true;
      }
      else
      {
        return false;
      };

    case SmiKey::Float:
      assert(keyLength == sizeof(double));
      assert(rightBoundaryLength == sizeof(double));
      memcpy(&keyDouble, key, keyLength);
      memcpy(&boundaryDouble, rightBoundary, keyLength);
      if(keyDouble > boundaryDouble)
      {
        return true;
      }
      else
      {
        return false;
      };

    case SmiKey::String:
    case SmiKey::Composite:
    case SmiKey::Unknown:
      cmpLength =
        keyLength > rightBoundaryLength ?
          rightBoundaryLength:
          keyLength;
      rc = memcmp(key, rightBoundary, cmpLength);
      if(rc > 0 || (rc == 0 && keyLength > rightBoundaryLength))
      {
        // the first cmpLength bytes of key are greater than rightBoundary
        // or they are equal but the keyLength is greater
        return true;
      }
      else
      {
        return false;
      };

    default:
      assert(false);
      return false;
  }
}

void PrefetchingIteratorImpl::Init
  (Dbc* dbc, const size_t bufferLength, bool isBTreeIterator)
{
  bufferPtr = new char[bufferLength];
  assert(bufferPtr != 0);

  searchType = ALL;

  buffer.set_data(bufferPtr);
  buffer.set_ulen(bufferLength);
  buffer.set_flags(DB_DBT_USERMEM);

  keyDbt.set_data(keyBuffer);
  keyDbt.set_ulen(SMI_MAX_KEYLEN);
  keyDbt.set_flags(DB_DBT_USERMEM);

  this->dbc = dbc;
  this->isBTreeIterator = isBTreeIterator;
  state = INITIAL;
}

void
PrefetchingIteratorImpl::GetKeyAddressAndLength
  (void** addr, SmiSize& length)
{
  if(isBTreeIterator)
  {
    switch(state)
    {
      case BULK_RETRIEVAL:
        *addr = retKey;
        length = retKeyLength;
        break;

      case PARTIAL_RETRIEVAL:
        *addr = keyDbt.get_data();
        length = keyDbt.get_size();
        break;

      case INITIAL:
      case BROKEN:
        assert(false);
    }
  }
  else
  {
    *addr = &recordNumber;
    length = sizeof(SmiRecordId);
  }
}

PrefetchingIteratorImpl::PrefetchingIteratorImpl
  (Dbc* dbc, SmiKey::KeyDataType keyType,
  const size_t bufferLength, bool isBTreeIterator)
{
  Init(dbc, bufferLength, isBTreeIterator);

  this->keyType = keyType;
}

PrefetchingIteratorImpl::PrefetchingIteratorImpl
  (Dbc* dbc, SmiKey::KeyDataType keyType, const char* leftBoundary,
  size_t leftBoundaryLength, const char* rightBoundary,
  size_t rightBoundaryLength, const size_t bufferLength)
{
  assert(leftBoundaryLength >= 0);
  assert(leftBoundaryLength <= SMI_MAX_KEYLEN);
  assert(rightBoundaryLength >= 0);
  assert(rightBoundaryLength <= SMI_MAX_KEYLEN);

  Init(dbc, bufferLength, true);

  if(leftBoundary == 0)
  {
    if(rightBoundary == 0)
    {
      searchType = ALL;
    }
    else
    {
      searchType = LEFTRANGE;
    }
  }
  else
  {
    if(rightBoundary == 0)
    {
      searchType = RIGHTRANGE;
    }
    else
    {
      searchType = RANGE;
    }
  }

  if(leftBoundary != 0)
  {
    memcpy(this->leftBoundary, leftBoundary, leftBoundaryLength);
  }

  if(rightBoundary != 0)
  {
    memcpy(this->rightBoundary, rightBoundary, rightBoundaryLength);
  }

  this->leftBoundaryLength = leftBoundaryLength;
  this->rightBoundaryLength = rightBoundaryLength;

  this->keyType = keyType;
}

PrefetchingIteratorImpl::~PrefetchingIteratorImpl()
{
  char* bufferPtr = (char*)buffer.get_data();
  delete[] bufferPtr;
  int rc = dbc->close();
  SmiEnvironment::SetBDBError(rc);
  assert(rc != DB_LOCK_DEADLOCK);
}

bool PrefetchingIteratorImpl::Next()
{
  if(state == INITIAL || state == PARTIAL_RETRIEVAL)
  {
    if(!NewPrefetch())
    {
      return false;
    }
  }

  if(state == PARTIAL_RETRIEVAL || state == BROKEN)
  {
    return state == PARTIAL_RETRIEVAL && !RightBoundaryExceeded();
  }

  if(isBTreeIterator)
  {
    DB_MULTIPLE_KEY_NEXT(p, buffer.get_DBT(), retKey,
      retKeyLength, retData, retDataLength);
  }
  else
  {
    DB_MULTIPLE_RECNO_NEXT(p, buffer.get_DBT(),
                           recordNumber, retData, retDataLength);
  }

  if(p == 0)
  {
    /* The pointer p is managed by Berkeley DB. ~p == 0~ implies that
       no tuples could be bulk-retrieved. */
    if(!NewPrefetch())
    {
      return false;
    }

    if(state == PARTIAL_RETRIEVAL || state == BROKEN)
    {
      return state == PARTIAL_RETRIEVAL  && !RightBoundaryExceeded();
    }

    if(isBTreeIterator)
    {
      DB_MULTIPLE_KEY_NEXT(p, buffer.get_DBT(), retKey,
        retKeyLength, retData, retDataLength);
    }
    else
    {
      DB_MULTIPLE_RECNO_NEXT(p, buffer.get_DBT(),
                             recordNumber, retData, retDataLength);
    }

    if(p != 0  && !RightBoundaryExceeded())
    {
      return true;
    }
    else
    {
      // end of scan
      return false;
    }
  }
  else
  {
    if(RightBoundaryExceeded())
    {
      //end of scan
      return false;
    }
    else
    {
      return true;
    }
  }
}

SmiSize PrefetchingIteratorImpl::BulkCopy(void* data, size_t dataLength,
  void* userBuffer, SmiSize nBytes, SmiSize offset)
{
  char* src = (char*)data;
  SmiSize nBytesCopied;

  if(offset >= dataLength)
  {
    return 0;
  }
  else
  {
    nBytesCopied =
     offset + nBytes > dataLength ?
       dataLength - offset :
       nBytes;

    memcpy(userBuffer, src + offset, nBytesCopied);
  }
  return nBytesCopied;
}

SmiSize PrefetchingIteratorImpl::ReadCurrentData
  (void* userBuffer, SmiSize nBytes, SmiSize offset)
{
  static long& ctr = Counter::getRef("SmiPrefetch::Calls");
  static long& byteCtr = Counter::getRef("SmiPrefetch::Read:Bytes");
  static long& pageCtr = Counter::getRef("SmiPrefetch::Read:Pages");
  static const int pageSize = WinUnix::getPageSize();
  SmiSize bytes = 0;
  ctr++;

  Dbt buf;

  switch(state)
  {
    case BULK_RETRIEVAL:
      bytes = BulkCopy(retData, retDataLength, userBuffer, nBytes, offset);
      byteCtr += bytes;
      pageCtr = byteCtr / pageSize;
      return bytes;

    case PARTIAL_RETRIEVAL:
      buf.set_data(userBuffer);
      buf.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
      buf.set_dlen(nBytes);
      buf.set_doff(offset);
      buf.set_ulen(nBytes);
      errorCode = dbc->get(&keyDbt, &buf, DB_CURRENT);
      bytes = buf.get_size();
      byteCtr += bytes;
      return bytes;

    case INITIAL:
      assert(false);
      return 0;

    case BROKEN:
      return 0;
  }
  assert(false);
  return 0;
}

SmiSize PrefetchingIteratorImpl::ReadCurrentKey
  (void* userBuffer, SmiSize nBytes, SmiSize offset)
{
  assert(isBTreeIterator);

  switch(state)
  {
    case BULK_RETRIEVAL:
      return BulkCopy(retKey, retKeyLength, userBuffer, nBytes, offset);

    case PARTIAL_RETRIEVAL:
      assert(keyDbt.get_size() <= keyDbt.get_ulen());
      return BulkCopy(keyDbt.get_data(), keyDbt.get_size(),
        userBuffer, nBytes, offset);

    case INITIAL:
      assert(false);
      return 0;

    case BROKEN:
      return 0;
  }
  assert(false);
  return 0;
}

void
PrefetchingIteratorImpl::ReadCurrentRecordNumber(SmiRecordId& recordNumber)
{
  assert(!isBTreeIterator);
  recordNumber = (SmiRecordId)this->recordNumber;
}

int PrefetchingIteratorImpl::ErrorCode()
{
  return errorCode;
}

/* --- bdbFile.cpp --- */
