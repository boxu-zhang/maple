/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\work_maple\maple\common\mp_common_thread_pool.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_COMMON_THREAD_POOL_H
#define DONT_INCLUDE_MP_COMMON_THREAD_POOL_H

#include "private/mp_exports.h"

/* @mpThreadPool
** The following API are used to manipulate a thread pool.
*/
MP_API(HRESULT) mpCreateThreadPool ( void ** ppPool );
MP_API(HRESULT) mpDestroyThreadPool ( void * pPool );
MP_API(HRESULT) mpThreadPoolInsertTask (
   void * pPool,
   void ( _stdcall * taskProc )( void * ),
   void * taskCtx );


#endif // DONT_INCLUDE_MP_COMMON_THREAD_POOL_H