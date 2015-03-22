/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\shared\mp_shared_base.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_SHARED_BASE_H
#define DONT_INCLUDE_MP_SHARED_BASE_H

#include "mp_shared_exports.h"
#include <stdio.h>

// MACROs {
#ifndef _countof
#define _countof( ary ) (sizeof(ary)/sizeof(ary[0]))
#endif // _countof

// }

// APIs {

/*
** Initialize shared library
*/
MP_SHARED_API(HRESULT) mpInitialize();

/*
** De-initialize shared library
*/
MP_SHARED_API(HRESULT) mpDeinitialize();

/*
**
*/
enum {
   MP_ES_PROCESS_NAME,
   MP_ES_MAX,
};

/*
** Get predefined environment string, please notice that this string is only valid after
** mpInitialize is called.
*/
MP_SHARED_API(const char *) mpGetEnvironmentString( int idx );
// } APIs


/*
** Get current time in string format, which looks like "2014-04-02_15-00-00"
*/
MP_SHARED_API(const char *) mpGetCurrentTimeString();

#endif // MP_SHARED_BASE_H