/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\source\mp_common_console.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include <winerror.h>
#include <stdio.h>
#include <string.h>
#include "mp_common_console.h"

MP_API(HRESULT) mpConsoleShowDescription (
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

MP_API(HRESULT) mpConsoleValidateParameters (
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

MP_API(HRESULT) mpConsoleExecute ( int argc, char * argv[], int count, mpCommand * pCmds )
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
