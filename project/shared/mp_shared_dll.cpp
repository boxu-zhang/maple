/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\project\shared\mp_shared_dll.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "stdafx.h"
#include "mp_shared_dll.h"
#include "mp_compound_heap.h"

maple::compound_heap _compound_heap;

void * _stdcall mpAllocate ( unsigned int size )
{
   return _compound_heap.alloc( size );
}

void _stdcall mpDeallocate ( void * ptr )
{
   _compound_heap.dealloc( ptr );
}

void _stdcall mpDiagnoseHeap ()
{
   _compound_heap.dump();
}

#include <winerror.h>
#include <stdio.h>
#include <string.h>

HRESULT _stdcall mpConsoleShowDescription (
   int argc, char * argv[], int count, mpCommand * pCmds )
{
   if ( !count || !pCmds )
      return E_INVALIDARG;

   printf(
      "Command syntax(in ECMA regular expression):\n\t%s ([-|/]{options} {parameters})*\n",
      argv[ 0 ] );

   printf( "{options} list:\n" );

   for ( int i = 0; i < count; ++i )
      printf( "   %s\t%s\n", pCmds[i].name, pCmds[i].description ); 
   return S_OK;
}

HRESULT _stdcall mpConsoleValidateParameters (
   int argc, char * argv[], int count, mpCommand * pCmds )
{
   if ( !count || !pCmds )
      return E_INVALIDARG;
   if ( argc == 1 )
      return S_OK;
   for ( int i = 0; i < count; ++i )
   {
      char * cmd = argv[ 1 ];
      if ( cmd[ 0 ] == '-' || cmd[ 0 ] == '/' )
      {
         if ( !strcmp( &cmd[ 1 ], pCmds[i].name ) )
         {
            if ( pCmds[ i ].validate )
               return 0 == pCmds[ i ].validate( argc - 2, &argv[ 2 ] ) ? S_OK : E_FAIL;
            else
               return S_OK;
         }
      }
   }
   return S_OK;
}

HRESULT _stdcall mpConsoleExecute (
   int argc, char * argv[], int count, mpCommand * pCmds )
{
   if ( !count || !pCmds )
      return E_INVALIDARG;
   if ( argc == 1 )
      return S_OK;
   for ( int i = 0; i < count; ++i )
   {
      char * cmd = argv[ 1 ];
      if ( cmd[ 0 ] == '-' || cmd[ 0 ] == '/' )
      {
         if ( !strcmp( &cmd[ 1 ], pCmds[i].name ) )
         {
            if ( pCmds[ i ].execute )
               return 0 == pCmds[ i ].execute( argc - 2, &argv[ 2 ] ) ? S_OK : E_FAIL;
            else
               return E_FAIL;
         }
      }
   }
   return S_OK;
}