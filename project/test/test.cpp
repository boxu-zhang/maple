/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\project\test\test.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "stdafx.h"
#include <list>
#include "../shared/mp_shared_dll.h"

// global variables
const char * test_help_doc = "\
Here we make a round test. Each round we generate following N randomized parameters:\n\
   1. count of serial allocations.( less than 10000 )\n\
   2. count of serial deallocations. (must smaller than the previous count)\n\
   3. size of each allocation in bytes. \n\
Once we get a piece of memory, we try to access every bytes of it.\n";

// implement command handler for test heap
int mp_test_heap ( int argc, char * argv[] )
{
   printf( "mp test heap begin...\n" );
   printf( test_help_doc );

   std::list<char*> ptr_cache;

   for ( int round = rand() % 100; round; round-- )
   {
      printf( "round %d start\n", round );
      int allocations = rand() % 100;
      while ( allocations )
      {
         int size = rand() % ( 8 * 1024 ) ;
         char * p = ( char * )mpAllocate( size );
         if ( p )
         {
            for ( int i = 0; i < size; ++i )
            {
               p[ i ] = 'c';
            }
         }
         ptr_cache.push_back( p );
         allocations--;
      }

      if ( allocations > 0 )
      {
         for ( int deallocations = rand() % allocations; deallocations; deallocations-- )
         {
            mpDeallocate( ptr_cache.back() );
            ptr_cache.pop_back();
         }
      }

      mpDiagnoseHeap();
      printf( "round %d end\n", round );
   }


   printf( "~test end~\n\n" );
   return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
   // define command table
   mpCommand cmds [] = {
      MP_COMMAND_ENTRY(
         "test_heap", "test mpAllocate & mpDeallocate API in mp_shared.dll",
         mp_test_heap, NULL ),
   };

   if ( FAILED( mpConsoleValidateParameters( argc, argv, _countof(cmds), cmds ) ) )
      return SUCCEEDED( mpConsoleShowDescription( argc, argv, _countof(cmds), cmds ) );
   
	return SUCCEEDED( mpConsoleExecute( argc, argv, _countof(cmds), cmds ) );
}

