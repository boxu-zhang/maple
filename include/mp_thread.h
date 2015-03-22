/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\maple\include\mp_thread.h

-- 2014.11.12 boxu.zhang

   Move into maple library and rename to mp_thread.h. Change mutual including
processor macro.
~~
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_THREAD_H
#define DONT_INCLUDE_MP_THREAD_H

#include <process.h>
#include <windows.h>

namespace maple {
   /*
   ** simple thread class
   */
   template <class thread_proc_class>
   class thread
   {
      typedef unsigned int ( thread_proc_class::*pfn_thread_routine )();
   public:
      thread ()
         : handle_( NULL )
      {}
      ~thread ()
      {
         stop();
      }

      bool start ( thread_proc_class * obj, pfn_thread_routine routine ) {
         if ( is_running() )
            return false;
         if ( handle_ )
            stop( true );
         obj_ = obj;
         routine_ = routine;
         handle_ = _beginthreadex( NULL, 0, thread_proc, this, 0, NULL );
         return handle_ != NULL ? true : false;
      }

      void stop ( bool wait = false ) {
         if ( !is_running() )
            return;
         if ( wait )
            WaitForSingleObject( ( HANDLE )handle_, INFINITE );
         CloseHandle( ( HANDLE )handle_ );
         handle_ = NULL;
      }

      bool is_running () {
         return (handle_ != NULL) && (WAIT_TIMEOUT == WaitForSingleObject( (HANDLE)handle_, 0 ));
      }

      void set_priority ( int priority ) {
         ::SetThreadPriority( (HANDLE)handle_, priority );
      }

   protected:
      static unsigned int _stdcall thread_proc ( void * context ) {
         return reinterpret_cast<thread *>( context )->thread_proc_ex();
      }
      unsigned int thread_proc_ex () {
         return (obj_->*routine_)();
      }

   private:
      uintptr_t handle_;
      thread_proc_class * obj_;
      pfn_thread_routine  routine_;
   };

   template <const bool autoreset = true>
   struct event_policy
   {
      static HANDLE create () {
         return ::CreateEvent( NULL, autoreset ? FALSE : TRUE, FALSE, NULL );
      }
      static void destroy ( HANDLE handle ) {
         ::CloseHandle( handle );
      }
      static void signal ( HANDLE handle ) {
         ::SetEvent( handle );
      }
      static void nonsignal ( HANDLE handle ) {
         ::ResetEvent( handle );
      }
   };

   template <const unsigned int period = 1000>
   struct timer_policy {
      static HANDLE create () {
         return ::CreateWaitableTimer( NULL, FALSE, NULL );
      }
      static void destroy ( HANDLE handle ) {
         ::CloseHandle( handle );
      }
      static void signal ( HANDLE handle ) {
         LARGE_INTEGER li;
         li.QuadPart = -1000;
         ::SetWaitableTimer( handle, &li, period, NULL, NULL, FALSE );
      }
      static void nonsignal ( HANDLE handle ) {
         ::CancelWaitableTimer( handle );
      }
   };

   template <
      class looping_proc_class,
      class looping_policy = event_policy<true>,
      class exit_policy = event_policy<false>,
      const bool alertable = false
      >
   class looping_thread
   {
      typedef void ( looping_proc_class::*pfn_looping_routine )();
   public:
      looping_thread ( bool exit_priority = true )
         : exit_event_( exit_policy::create() )
         , looping_event_( looping_policy::create() )
      {}

      ~looping_thread ()
      {}

      bool start ( looping_proc_class * obj, pfn_looping_routine routine ) {
         if ( thread_.is_running() )
            return true;

         // every time before we start a new thread, make sure the exit event
         // is none signaled
         exit_policy::nonsignal( exit_event_ );

         obj_ = obj;
         routine_ = routine;
         if ( !thread_.start( this, &looping_thread::thread_proc ) ) {
            return false;
         }

         // signal the looping event
         looping_policy::signal( looping_event_ );
         return true;
      }

      void stop () {
         if ( thread_.is_running() )
         {
            exit_policy::signal( exit_event_ );
            thread_.stop( true );
         }
      }

      bool is_running () {
         return thread_.is_running();
      }

      void suspend () {
         if ( looping_event_ )
            looping_policy::nonsignal( looping_event_ );
      }
      
      bool is_suspended () {
         if ( looping_event_ )
            return WAIT_OBJECT_0 == WaitForSingleObject( looping_event_, 0 ) ? false : true;
         return false;
      }

      void resume () {
         if ( looping_event_ )
            looping_policy::signal( looping_event_ );
      }

      void set_priority ( int priority ) {
         thread_.set_priority( priority );
      }

      HANDLE looping_event () {
         return looping_event_;
      }

      HANDLE exit_event () {
         return exit_event_;
      }

   protected:
      unsigned int thread_proc () {
         HANDLE waits [] = {
            exit_event_,
            looping_event_
         };
         do 
         {
            DWORD waited = WaitForMultipleObjectsEx(
               sizeof( waits ) / sizeof( waits[ 0 ] ),
               waits,
               FALSE,
               INFINITE,
               alertable ? TRUE : FALSE );
            if ( WAIT_OBJECT_0 == waited )
               break;
            if ( WAIT_OBJECT_0 + 1 == waited )
               (obj_->*routine_)();
         } while ( TRUE );
         return 0;
      }

   private:
      HANDLE exit_event_;
      HANDLE looping_event_;
      looping_proc_class * obj_;
      pfn_looping_routine routine_;
      thread<looping_thread/*<looping_proc_class> */> thread_;
   };
}

#endif // DONT_INCLUDE_MP_THREAD_H