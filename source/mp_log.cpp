/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\project\shared\mp_shared_log.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "mp_shared_log.h"
#include "mp_shared_base.h"
#include "mp_shared_auxiliary.h"

struct mpLog {
   mpLog ( const char * fname = NULL )
      : fpLogFile( NULL )
   {
      char fpath[256] = { 0 };
      sprintf_s( fpath, _countof( fpath ),
         "%s_%s.txt", maple::elect( fname, "maple" ), mpGetCurrentTimeString() );
      fopen_s( &fpLogFile, fpath, "w" );
   }
   ~mpLog () {
      fclose( fpLogFile );
   }
   FILE * fpLogFile;
};

mpLog _MP_LOG(mpGetEnvironmentString(MP_ES_PROCESS_NAME));

MP_SHARED_API(void) mpLogA ( const char * msg )
{
   fprintf_s( _MP_LOG.fpLogFile, "%s:%s", mpGetCurrentTimeString(), msg );
}

MP_SHARED_API(void) mpLogW ( const unsigned short * msg )
{
   fwprintf_s( _MP_LOG.fpLogFile, L"%s:%s", mpGetCurrentTimeString(), msg );
}

MP_SHARED_API(void) mpLogW ( const wchar_t * msg )
{
   mpLogW( ( const unsigned short * )msg );
}
