/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\include\mp_async_task_scheduler.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_ASYNC_TASK_SCHEDULER_H
#define DONT_INCLUDE_MP_ASYNC_TASK_SCHEDULER_H

#include <queue>
#include "mp_thread.h"
#include "mp_locks.h"

namespace maple {
   
   struct task {
      virtual bool execute () = 0;
      virtual void destroy () = 0;
   };

   class async_task_scheduler
   {
      typedef looping_thread<async_task_scheduler, event_policy<false> > engine;
   public:
      async_task_scheduler ();
      ~async_task_scheduler ();

      bool start ();
      void stop ();
      bool append ( task * tsk );

   protected:
      void task_schedule_proc ();

   private:
      critical_section m_cs;
      std::queue<task *> m_queue;
      engine m_engine;
   };
}

#endif // DONT_INCLUDE_MP_ASYNC_TASK_SCHEDULER_H