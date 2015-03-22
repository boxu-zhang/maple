/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\source\mp_refcount.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include <windows.h>
#include "mp_refcount.h"

namespace maple {
   crefcount::crefcount()
      : refcount_( 1 )
   {}

   long crefcount::addref () {
      return ::InterlockedIncrement( &refcount_ );
   }

   long crefcount::release () {
      return ::InterlockedDecrement( &refcount_ );
   }
}