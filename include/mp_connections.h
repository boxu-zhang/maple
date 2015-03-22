/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\advantech_trek\svn_xian\MRM_Video\Include\pcl\pcl_connections.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef PCL_CONNECTIONS_H
#define PCL_CONNECTIONS_H

#include <map>
#include "pcl_locks.h"

#if defined _X86_
typedef unsigned int cp_key;
#elif defined _AMD64_
typedef unsigned __int64 cp_key;
#elif defined _IA64_
typedef unsigned __int64 cp_key;
#endif // processor architecture


// declare key maker
template <class T, class C>
struct cp_key_maker;
/* 
key maker interface
{
   static cp_key make ( typename T, typename C ) {
      // do make here
   }
};
*/

// predefined key makers
template <>
struct cp_key_maker<void *, void *> {
   static cp_key make ( void * pfn, void * ctx ) {
      return (cp_key)pfn ^ (cp_key)ctx; 
   }
};

namespace pcl
{
   template <class T, class C>
   struct is_member_traits {
      enum { val = 0 };
   };

   // member function with default calling convention
   template <class T, class P1>
   struct is_member_traits<T, void (T::*)(P1)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2>
   struct is_member_traits<T, void (T::*)(P1, P2)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2, class P3>
   struct is_member_traits<T, void (T::*)(P1, P2, P3)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2, class P3, class P4>
   struct is_member_traits<T, void (T::*)(P1, P2, P3, P4)> {
      enum { val = 1 };
   };

   // member function with standard calling convention
   template <class T, class P1>
   struct is_member_traits<T, void (_stdcall T::*)(P1)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2>
   struct is_member_traits<T, void (_stdcall T::*)(P1, P2)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2, class P3>
   struct is_member_traits<T, void (_stdcall T::*)(P1, P2, P3)> {
      enum { val = 1 };
   };

   template <class T, class P1, class P2, class P3, class P4>
   struct is_member_traits<T, void (_stdcall T::*)(P1, P2, P3, P4)> {
      enum { val = 1 };
   };

   // invoke helper
   template <class T, class C, const int is_member=is_member_traits<C, T>::val >
   struct invoke_function;

   template <class T, class C>
   struct invoke_function<T, C, 0>
   {
      template <class P1>
      static void invoke ( T pfn, C ctx, P1 p1 ) {
         pfn( p1, ctx );
      }
      template <class P1, class P2>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2 ) {
         pfn( p1, p2, ctx );
      }
      template <class P1, class P2, class P3>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2, P3 p3 ) {
         pfn( p1, p2, p3, ctx );
      }
      template <class P1, class P2, class P3, class P4>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2, P3 p3, P4 p4 ) {
         pfn( p1, p2, p3, p4, ctx );
      }
   };

   template <class T, class C>
   struct invoke_function<T, C, 1>
   {
      template <class P1>
      static void invoke ( T pfn, C ctx, P1 p1 ) {
         ((*ctx).*pfn)( p1 );
      }
      template <class P1, class P2>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2 ) {
         ((*ctx).*pfn)( p1, p2 );
      }
      template <class P1, class P2, class P3>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2, P3 p3 ) {
         ((*ctx).*pfn)( p1, p2, p3 );
      }
      template <class P1, class P2, class P3, class P4>
      static void invoke ( T pfn, C ctx, P1 p1, P2 p2, P3 p3, P4 p4 ) {
         ((*ctx).*pfn)( p1, p2, p3, p4 );
      }
   };

   template <class T, class C, class K>
   class connection_pt
   {
      typedef typename T pfn_type;
      typedef typename C ctx_type;
      typedef typename K key_maker_type;
      struct sink
      {
         pfn_type pfn;
         ctx_type ctx;
      };
   public:
      connection_pt ()
      {}
      ~connection_pt ()
      {}

      bool connect ( pfn_type pfn, ctx_type ctx ) {
         auto_lock<critical_section> lock( cs_ );
         if ( sinks_.find( key_maker_type::make( pfn, ctx ) ) == sinks_.end() )
         {
            // not found
            sink s = { pfn, ctx };
            sinks_[ key_maker_type::make( pfn, ctx ) ] = s;
            return true;
         }
         return false;
      }

      bool disconnect ( pfn_type pfn, ctx_type ctx ) {
         auto_lock<critical_section> lock( cs_ );
         std::map<cp_key, sink>::iterator it =
            sinks_.find( key_maker_type::make( pfn, ctx ) );
         if ( it != sinks_.end() )
         {
            sinks_.erase( it );
            return true;
         }
         return false;
      }
		int getsize( pfn_type pfn, ctx_type ctx )
		{
			return sinks_.size();
		}
      template <class P1>
      void fire ( P1 p1 ) {
         auto_lock<critical_section> lock( cs_ );
         for ( std::map<cp_key, sink>::iterator it = sinks_.begin();
            it != sinks_.end(); it++ )
         {
            if ( it->second.pfn )
               invoke_function<T, C>::invoke( it->second.pfn, it->second.ctx, p1 );
         }
      }

      template <class P1, class P2>
      void fire ( P1 p1, P2 p2 ) {
         auto_lock<critical_section> lock( cs_ );
         for ( std::map<cp_key, sink>::iterator it = sinks_.begin();
            it != sinks_.end(); it++ )
         {
            if ( it->second.pfn )
               invoke_function<T, C>::invoke( it->second.pfn, it->second.ctx, p1, p2 );
         }
      }

      template <class P1, class P2, class P3>
      void fire ( P1 p1, P2 p2, P3 p3 ) {
         auto_lock<critical_section> lock( cs_ );
         for ( std::map<cp_key, sink>::iterator it = sinks_.begin();
            it != sinks_.end(); it++ )
         {
            if ( it->second.pfn )
               invoke_function<T, C>::invoke( it->second.pfn, it->second.ctx, p1, p2, p3 );
         }
      }

      template <class P1, class P2, class P3, class P4>
      void fire ( P1 p1, P2 p2, P3 p3, P4 p4 ) {
         auto_lock<critical_section> lock( cs_ );
         for ( std::map<cp_key, sink>::iterator it = sinks_.begin();
            it != sinks_.end(); it++ )
         {
            if ( it->second.pfn )
               invoke_function<T, C>::invoke( it->second.pfn, it->second.ctx, p1, p2, p3, p4 );
         }
      }

   private:
      critical_section cs_;
      std::map<cp_key, sink> sinks_;
   };
}

#define PCL_DECLARE_CONNECTION_PT_TYPE( pfn_t, ctx_t, connection_pt_t ) \
   template <> \
   struct cp_key_maker<pfn_t, ctx_t> : public cp_key_maker<void*, void*> { \
      static cp_key make ( pfn_t pfn, ctx_t ctx ) { \
         return cp_key_maker<void*, void*>::make( (void *)pfn, (void*)ctx ); \
      } \
   }; \
   typedef pcl::connection_pt<pfn_t, ctx_t, cp_key_maker<pfn_t, ctx_t> > connection_pt_t;


#endif // PCL_CONNECTIONS_H