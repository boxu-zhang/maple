/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\source\mp_async_task_scheduler.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "mp_async_task_scheduler.h"

namespace maple {
   async_task_scheduler::async_task_scheduler ()
   {}

   async_task_scheduler::~async_task_scheduler ()
   {
      if ( m_engine.is_running() )
         m_engine.stop();
   }

   bool async_task_scheduler::start ()
   {
      if ( m_engine.is_running() )
         return false;
      return m_engine.start( this, &async_task_scheduler::task_schedule_proc );
   }

   void async_task_scheduler::stop ()
   {
      if ( m_engine.is_running() )
         m_engine.stop();
   }

   bool async_task_scheduler::append ( task * tsk )
   {
      if ( !tsk )
         return false;

      do 
      {
         auto_lock<critical_section> lock( m_cs );
         while ( m_queue.size() > 12 )
         {
            m_queue.front()->destroy();
            m_queue.pop();
         }
         m_queue.push( tsk );

         if ( !m_engine.is_running() )
            start();
         // resume if needed
         if ( m_engine.is_suspended() )
            m_engine.resume();
      } while ( false );
      
      return true;
   }

   void async_task_scheduler::task_schedule_proc ()
   {
      task * tsk = NULL;
      do 
      {
         auto_lock<critical_section> lock( m_cs );
         if ( !m_queue.size() )
            break;
         tsk = m_queue.front();
         m_queue.pop();
      } while ( false );

      if ( !tsk )
      {
         m_engine.suspend();
         return;
      }

      // execute it
      tsk->execute();
      tsk->destroy();
   }
}