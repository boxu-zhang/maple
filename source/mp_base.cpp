/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\shared\mp_shared_impl.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "mp_shared_base.h"
#include <time.h>

BOOL WINAPI DllMain (void * _HDllHandle, unsigned _Reason, void * _Reserved)
{
   switch ( _Reason )
   {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
   }
   return TRUE;
}

MP_SHARED_API(HRESULT) mpInitialize ()
{
   return 0;
}

MP_SHARED_API(HRESULT) mpDeinitialize ()
{
   return 0;
}

const char * _MP_ES_TABLE [ MP_ES_MAX ] = { 0 };

MP_SHARED_API(const char *) mpGetEnvironmentString ( int idx )
{
   if ( idx <= _countof( _MP_ES_TABLE ) )
      return _MP_ES_TABLE[ idx ];
   return NULL;
}

char _MP_CURRENT_TIME [ 64 ] = { 0 };

MP_SHARED_API(const char *) mpGetCurrentTimeString ()
{
   time_t cur = 0;
   time( &cur );
   ctime_s( _MP_CURRENT_TIME, _countof( _MP_CURRENT_TIME ), &cur );
   return _MP_CURRENT_TIME;
}