/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\include\mp_refcount.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_REFCOUNT_H
#define DONT_INCLUDE_MP_REFCOUNT_H

namespace maple {
   struct crefcount
   {
      crefcount ();

      long addref ();
      long release ();
      long refcount_;
   };
}

#define MP_SUPPORT_RECOUNT() \
   maple::crefcount m_rc; \
   ULONG AddRef () { return m_rc.addref(); } \
   ULONG Release () { ULONG ulRefCount = m_rc.release(); if ( !ulRefCount ) delete this; return ulRefCount; }

#endif // DONT_INCLUDE_MP_REFCOUNT_H