/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\project\shared\mp_shared_auxiliary.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_SHARED_AUXILIARY_H
#define DONT_INCLUDE_MP_SHARED_AUXILIARY_H

namespace maple
{
   template <class T>
   T elect ( T vala, T valb ) {
      return vala ? vala : valb;
   };
}

#endif // DONT_INCLUDE_MP_SHARED_AUXILIARY_H