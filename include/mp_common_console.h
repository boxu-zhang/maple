/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\include\mp_common_console.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_COMMON_CONSOLE_H
#define DONT_INCLUDE_MP_COMMON_CONSOLE_H

#include <wtypes.h>
#include "private/mp_exports.h"

// structures for command
typedef struct _mpCommand {
   const char * name;
   const char * description;
   int ( * execute )( int argc, char * argv[] );
   int ( * validate )( int argc, char * argv[] );
} mpCommand;

#define MP_COMMAND_ENTRY( name, desc, exeproc, validproc ) \
   { name, desc, exeproc, validproc }

// APIs
MP_API(HRESULT) mpConsoleShowDescription ( int argc, char * argv[], int count, mpCommand * pCmds );
MP_API(HRESULT) mpConsoleValidateParameters ( int argc, char * argv[], int count, mpCommand * pCmds );
MP_API(HRESULT) mpConsoleExecute ( int argc, char * argv[], int count, mpCommand * pCmds );


#endif // DONT_INCLUDE_MP_COMMON_CONSOLE_H