/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ E:\works\maple\include\mp_componenet_object_model.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_COMPONENT_OBJECT_MODEL_H
#define DONT_INCLUDE_MP_COMPONENT_OBJECT_MODEL_H

#include <Unknwn.h>
#include <assert.h>

namespace maple {
namespace component_object_model {

   struct interface_entry
   {
      const GUID * iid;
      size_t offset;
   };

   struct interface_map
   {
      interface_map * base_map;
      interface_entry * entries;
   };

   // implement component object model base object
   class cunknown
   {
   protected:
      cunknown ();
      virtual ~cunknown ();

      virtual interface_map * mpGetInterfaceMap () = 0;
   public:
      // interface map framework
      static interface_map _interfaceMap;
      
      // IUnknown
      ULONG mpAddRef ();
      ULONG mpRelease ();
      HRESULT mpQueryInterface ( REFIID riid, void ** ppvObj );

   private:
      unsigned long m_refcount;
   };
}}

// type alias
typedef maple::component_object_model::cunknown CMpUnknown;

// Macros that benefits development
#define MP_OFFSETOF(s, m) (size_t)&(((s *)0)->m)

#define MP_DECLARE_INTERFACE_MAP() \
   public: \
   static maple::component_object_model::interface_map _interfaceMap; \
   static maple::component_object_model::interface_entry _interfaceEntries []; \
   protected: \
   maple::component_object_model::interface_map * mpGetInterfaceMap() { \
      return &_interfaceMap; \
   } \

#define MP_BEGIN_INTERFACE_PART( i ) \
   class X##i : public I##i { \
   public: \
      STDMETHOD_( ULONG, AddRef )(); \
      STDMETHOD_( ULONG, Release )(); \
      STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObj ); \

#define MP_END_INTERFACE_PART( i ) \
   } m_x##i;

#define MP_BEGIN_INTERFACE_MAP( t, b ) \
   maple::component_object_model::interface_map t::_interfaceMap = { \
      &b::_interfaceMap, &t::_interfaceEntries[0] \
   }; \
   maple::component_object_model::interface_entry t::_interfaceEntries [] = { \

#define MP_INTERFACE_ENTRY( t, iid, i ) \
      { &iid, MP_OFFSETOF(t, m_x##i) }, \

#define MP_END_INTERFACE_MAP() \
      { NULL, size_t( -1 ) }, \
   }; \

#define MP_METHOD_PROLOGUE( t, i ) \
   t * pThis = ( t * )( \
      (char*)this - MP_OFFSETOF( t, m_x##i ) )

#define MP_IMPLEMENT_IUNKNOWN( t, i ) \
   STDMETHODIMP_( ULONG ) t::X##i::AddRef () { \
      MP_METHOD_PROLOGUE( t, i ); \
      return pThis->mpAddRef(); \
   } \
   STDMETHODIMP_( ULONG ) t::X##i::Release () { \
      MP_METHOD_PROLOGUE( t, i ); \
      return pThis->mpRelease(); \
   } \
   STDMETHODIMP t::X##i::QueryInterface ( REFIID riid, void ** ppvObj ) { \
      MP_METHOD_PROLOGUE( t, i ); \
      return pThis->mpQueryInterface( riid, ppvObj ); \
   } \

#define MP_IMPLEMENT_METHOD_PROXY( t, i, m, p, ... ) \
   STDMETHODIMP t::X##i::m p { \
      MP_METHOD_PROLOGUE( t, i ); \
      return pThis->Ex_##i##_##m __VA_ARGS__; \
   } \

#if defined( MP_AUTO_LINK )
   #if defined( MP_USE_STATIC_LIB )

      #pragma comment ( lib, "mp_static" )
   
   #else
      
      #pragma comment( lib, "mp_shared" )
   
   #endif
#endif // Automatically link to maple library

#endif // DONT_INCLUDE_MP_COMPONENT_OBJECT_MODEL_H