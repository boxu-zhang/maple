/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ .\mbcs_tchar_bridge.h

-- Updates 2012.10.25 boxu.zhang
   
   Using the compiler predefined macro, '_NATIVE_WCHAR_T_DEFINED', to decide
   whether the wchar_t type is supported
~~ 
-- Updates 2014.04.03 boxu.zhang
   
   Fix some comment syntax error. In this hole file, no memory allocation fail
   is handled, because in my project, input strings aren't big enough to
   make it. But if you want to convert a long string which is not any small,
   please take care with memory problem. I suggest that you use a 'try catch'
   block around the converter.
~~
-- Updates 2014.10.27 boxu.zhang

   Move into MAPLE library. It's a new C/CPP library that supports many new features.
   Details:
      1. Namespace is changed from 'mbcs_tchar_bridge' to 'maple'
      2. Delete the header files: <tchar.h> && <windows.h>, and all required types are
   defined in this file itself.
      3. Rename the two converter: 'tszstr' to 'tszstr', 'aszstr' to 'aszstr'
      4. Rename filename to mp_string.h
~~

--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef _MBCS_TCHAR_BRIDGE_H_
#define _MBCS_TCHAR_BRIDGE_H_

#include <string>

#ifndef MTB_CODE_PAGE
#define MTB_CODE_PAGE CP_ACP
#endif // MTB_CODE_PAGE

// define text
#ifndef MP_TEXT
   #if defined( UNICODE )
      #define MP_TEXT( x ) L##x
   #else
      #define MP_TEXT( x ) x
   #endif
#endif

namespace maple {

enum 
{
   noneCharactor,
   noneWideChar,
   wideChar,
};

template <class charactors>
struct char_type
{
   enum { type = noneCharactor };
};

template <>
struct char_type<char>
{
   enum { type = noneWideChar };
};

template <>
struct char_type<unsigned short>
{
   enum { type = wideChar };
};

// note: fix /Zc:wchar_t option
#if defined(_NATIVE_WCHAR_T_DEFINED)
template <>
struct char_type<wchar_t>
{
   enum { type = wideChar };
};
#endif // _NATIVE_WCHAR_T_DEFINED


template <
   class charactors,
      bool is_charactor = char_type<charactors>::type != noneCharactor,
      bool tchar_is_mbcs = char_type<TCHAR>::type == noneWideChar,
      bool input_is_mbcs = char_type<charactors>::type == noneWideChar >
class tszstr;

// TCHAR is mbcs and the input is wide char
template <class charactors>
class tszstr<charactors, true, true, false >
{
public:
   tszstr ( const charactors * pstr ) 
   {
      // call the API to convert wide char to mbcs
      int nCount = WideCharToMultiByte(
         MTB_CODE_PAGE, 0, pstr, -1,
         NULL, 0, NULL, NULL );
      m_pstr = new unsigned short[ nCount ];
      memset( m_pstr, 0, sizeof( unsigned short ) * nCount );
      // make it
      WideCharToMultiByte(
         MTB_CODE_PAGE, 0, pstr, -1,
         m_pstr, nCount, NULL, NULL );
   }
   ~tszstr () 
   {
      if ( m_pstr )
         delete [] m_pstr;
   }

   operator const unsigned short * () {
      return m_pstr;
   }

#if defined(_NATIVE_WCHAR_T_DEFINED)
   operator const wchar_t * () {
      return (wchar_t *)m_pstr;
   }
#endif // _NATIVE_WCHAR_T_DEFINED

private:
   operator unsigned short * ();

private:
   unsigned short * m_pstr;
};

// unsigned short is unicode and input is mbcs
template <class charactors>
class tszstr<charactors, true, false, true >
{
public:
   tszstr ( const charactors * pstr ) 
   {
      // call the API to convert mbcs to wide char 
      int nCount = MultiByteToWideChar(
         MTB_CODE_PAGE, 0, pstr, -1,
         NULL, 0 );
      m_pstr = new unsigned short[ nCount ];
      memset( m_pstr, 0, sizeof( unsigned short ) * nCount );
      MultiByteToWideChar(
         MTB_CODE_PAGE, 0, pstr, -1,
         (wchar_t *)m_pstr, nCount );
   }
   ~tszstr () 
   {
      if ( m_pstr )
         delete [] m_pstr;
   }

   operator const unsigned short * () {
      return m_pstr;
   }

#if defined(_NATIVE_WCHAR_T_DEFINED)
   operator const wchar_t * () {
      return (wchar_t *)m_pstr;
   }
#endif // _NATIVE_WCHAR_T_DEFINED

   // disable converter to 'unsigned short *', because it's not safe.
private:
   operator unsigned short * () { return m_pstr; }

#if defined ( _NATIVE_WCHAR_T_DEFINED )
   operator wchar_t * () {
      return (wchar_t *)m_pstr;
   }
#endif // _NATIVE_WCHAR_T_DEFINED

private:
   unsigned short * m_pstr;
};

/**
 * unsigned short and 'charactors' are same types, or, unsigned short is wchar_t and 'charactors'
 * is unsigned short, or, vice versa.
 */
template <class charactors, bool b>
class tszstr<charactors, true, b, b>
{
public:
   tszstr ( const charactors * pstr ) 
      : m_pstr( pstr )
   {}
   ~tszstr () 
   {}

   operator const charactors * () {
      return m_pstr;
   }

   // disable converter to type 'unsigned short *', because it's not safe.
private:
   operator charactors * ();
   
private:
   const charactors * m_pstr;
};

template <
   class charactors,
      const int input_type = char_type<charactors>::type >
class aszstr;

template <class charactors>
class aszstr<charactors, noneWideChar>
{
public:
   aszstr( const charactors * pstr ) : m_pstr( pstr ) {}

   operator const charactors * () { return m_pstr; }

private:
   operator char * ();

private:
   const charactors * m_pstr;
};

template <class charactors>
class aszstr<charactors, wideChar>
{
public:
   aszstr ( const charactors * pstr )
   {
      int nCount = WideCharToMultiByte(
         MTB_CODE_PAGE, 0, pstr, -1,
         NULL, 0, NULL, NULL );
      m_pstr = new char[ nCount ];
      memset( m_pstr, 0, sizeof( char ) * nCount );
      WideCharToMultiByte(
         MTB_CODE_PAGE, 0, pstr, -1,
         m_pstr, nCount, NULL, NULL );
   }
   ~aszstr ()
   {
      if ( m_pstr )
         delete [] m_pstr;
   }

   operator const char * () { return m_pstr; }

private:
   operator char * ();

private:
   char * m_pstr;
};

}
#endif // _MBCS_TCHAR_BRIDGE_H_