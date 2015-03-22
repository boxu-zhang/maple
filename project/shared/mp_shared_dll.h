/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\project\shared\mp_shared_dll.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_SHARED_DLL_H
#define DONT_INCLUDE_MP_SHARED_DLL_H

#if defined( MP_SHARED_EXPORTS )
#define MP_SHARED_API __declspec(dllexport)
#else
#define MP_SHARED_API __declspec(dllimport)
#endif

// structures for command
typedef struct _mpCommand {
   const char * name;
   const char * description;
   int ( * execute )( int argc, char * argv[] );
   int ( * validate )( int argc, char * argv[] );
} mpCommand;

#define MP_COMMAND_ENTRY( name, desc, exeproc, validproc ) \
{ name, desc, exeproc, validproc }


#if defined( __cplusplus )
extern "C" {
#endif 

// APIs for heap
MP_SHARED_API void * _stdcall mpAllocate ( unsigned int size );
MP_SHARED_API void _stdcall mpDeallocate ( void * ptr );
MP_SHARED_API void _stdcall mpDiagnoseHeap ();

// APIs for console command process
MP_SHARED_API long _stdcall mpConsoleShowDescription (
   int argc, char * argv[], int count, mpCommand * pCmds );
MP_SHARED_API long _stdcall mpConsoleValidateParameters (
   int argc, char * argv[], int count, mpCommand * pCmds );
MP_SHARED_API long _stdcall mpConsoleExecute (
   int argc, char * argv[], int count, mpCommand * pCmds );


#if defined( __cplusplus )
}
#endif

// replace C++ new and delete operator
#if defined( ENABLE_MAPLE_NEW_DELETE_OPERATOR )

// only can be used with C++
#if defined( __cplusplus )

void * operator new ( unsigned int size ) {
   return mpAllocate( size );
}

void operator delete ( void * ptr ) {
   mpDeallocate( ptr );
}

#endif // __cplusplus

#endif // ENABLE_MAPLE_NEW_DELETE_OPERATOR

// auto link code
#if !defined( MP_SHARED_EXPORTS )
#pragma comment( lib, "mp_shared.lib" )
#endif

#endif // DONT_INCLUDE_MP_SHARED_DLL_H