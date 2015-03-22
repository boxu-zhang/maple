/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\source\mp_component_object_model.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include "mp_component_object_model.h"

namespace maple {
namespace component_object_model {

   cunknown::cunknown ()
      : m_refcount( 1 )
   {}

   cunknown::~cunknown ()
   {
      assert( m_refcount == 0 );
   }

   ULONG cunknown::mpAddRef () {
      return ::InterlockedIncrement( (LONG*)&m_refcount );
   }

   ULONG cunknown::mpRelease () {
      unsigned long refcount = ::InterlockedDecrement( (LONG*)&m_refcount );
      if ( !refcount )
         delete this;
      return refcount;
   }

   HRESULT cunknown::mpQueryInterface ( REFIID riid, void ** ppvObj ) {
      if ( !ppvObj )
         return E_INVALIDARG;
      for ( interface_map * m = mpGetInterfaceMap(); m; m = m->base_map )
      {
         for ( int i = 0; m->entries[ i ].iid; ++i )
         {
            if ( riid == *m->entries[ i ].iid )
            {
               *ppvObj = ((char* )this + m->entries[ i ].offset);
               reinterpret_cast<IUnknown *>(*ppvObj)->AddRef();
               return S_OK;
            }
         }
      }
      return E_NOINTERFACE;
   }

   // interface map framework
   interface_map cunknown::_interfaceMap = {
      NULL, NULL
   };
}}