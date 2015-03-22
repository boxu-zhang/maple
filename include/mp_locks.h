/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ ./maple/include/mp_locks.h

-- 2014.11.18 boxu.zhang
   
   Move into maple library, and rename the namespace and mutual including macro.
~~
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_LOCKS_H
#define DONT_INCLUDE_MP_LOCKS_H

namespace maple {

   template <class lock_type>
   class lock_op_traits {
   public:
      static void lock ( lock_type & lock_obj ) {
         lock_obj.lock();
      }
      static void unlock ( lock_type & lock_obj ) {
         lock_obj.unlock();
      }
   };

   template <>
   class lock_op_traits<CRITICAL_SECTION> {
   public:
      static void lock ( CRITICAL_SECTION & lock_obj ) {
         ::EnterCriticalSection( &lock_obj );
      }
      static void unlock ( CRITICAL_SECTION & lock_obj ) {
         ::LeaveCriticalSection( &lock_obj );
      }
   };

   class critical_section {
   public:
      critical_section () {
         ::InitializeCriticalSection( &cs_ );
      }
      ~critical_section () {
         ::DeleteCriticalSection( &cs_ );
      }
      void lock () {
         ::EnterCriticalSection( &cs_ );
      }
      void unlock () {
         ::LeaveCriticalSection( &cs_ );
      }
   private:
      CRITICAL_SECTION cs_;
   };

   template <class lock_type>
   class auto_lock {
   public:
      auto_lock ( lock_type & lock_obj )
         : lock_obj_( lock_obj ) {
         lock_op_traits<lock_type>::lock( lock_obj_ );
      }
      ~auto_lock () {
         lock_op_traits<lock_type>::unlock( lock_obj_ );
      }
   private:
      lock_type & lock_obj_;
   };
}

#endif // DONT_INCLUDE_MP_LOCKS_H