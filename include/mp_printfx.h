/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ E:\works\maple\include\mp_printfx.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_PRINTFX_H
#define DONT_INCLUDE_MP_PRINTFX_H

#include <stdio.h>
#include <stdlib.h>

namespace maple {
   struct printfx
   {
      struct spec
      {
         char flag;
         int width;
         int precision;
         char prefix[ 4 ];
         char type;
      };

      static int get_bytes ( __int64 n, int scale = 10 )
      {
         if ( !n )
            return 1;
         int i = 0;
         while ( n )
         {
            n /= scale;
            ++i;
         }
         return i;
      }

      static int get_scaler ( char c )
      {
         switch ( c )
         {
         case 'x': case 'X':
            return 16;
         case 'd': case 'i':
            return 10;
         case 'u': case 'o':
            return 8;
         }
         return 10;
      }

      static int get_spec_bytes ( spec & s, int arg, int scale = 10 )
      {
         return get_spec_bytes( s, (__int64)arg, scale );
      }

      static int get_spec_bytes ( spec & s, __int64 arg, int scale = 10 )
      {
         int nvalbytes = get_bytes( arg, scale );
         int nbytes = max(nvalbytes, s.width);
         switch ( s.flag )
         {
         case '+': case '-':
            if ( s.width <= nvalbytes )
               nbytes += 1;
            break;
         case '#':
            if ( s.width <= nvalbytes )
            {
               if ( s.type == 'o' )
                  nbytes += 1;
               if ( s.type == 'x' || s.type == 'X' )
                  nbytes += 2;
            }
            break;
         }
         return nbytes;
      }

      static int get_spec_bytes ( spec & s, const char * arg )
      {
         return max( strlen( arg ), (size_t)s.width );
      }

      static int get_spec_bytes ( spec & s, const wchar_t * arg )
      {
         return max( wcslen( arg ), (size_t)s.width );
      }

      static int get_spec_bytes ( spec & s, double arg )
      {
         return 32;
      }

      static bool is_type_character ( char c )
      {
         if ( c == 'C' || c == 'c' || c == 'd' || c == 'i'
            || c == 'o' || c == 'u' || c == 'x' || c == 'X'
            || c == 'e' || c == 'E' || c == 'f' || c == 'g' || c == 'G'
            || c == 'a' || c == 'A' || c == 'n' || c == 'p'
            || c == 's' || c == 'S' )
            return true;
         return false;
      }

      static void get_next_spec ( const char * fmt, spec & s, int & i )
      {
         // skip %
         if ( fmt[ i ] == '%' )
            ++i;
         // get flag
         if ( fmt[ i ] == '+'
            || fmt[ i ] == '-'
            || fmt[ i ] == ' '
            || fmt[ i ] == '0'
            || fmt[ i ] == '#' )
            s.flag = fmt[ i++ ];
         // get width
         while ( '0' < fmt[ i ] && fmt[ i ] < '9' )
         {
            s.width += s.width * 10 + fmt[ i ] - '0';
            ++i;
         }
         // get precision
         if ( fmt[ i ] == '.' )
         {
            ++i;
            while ( '0' < fmt[ i ] && fmt[ i ] < '9' )
            {
               s.precision += s.precision * 10 + fmt[ i ] - '0';
               ++i;
            }
         }
         // get prefix
         for ( int j = i; !is_type_character( fmt[ i ] ); ++i )
            s.prefix[ i - j ] = fmt[ i ];
         // get type
         if ( is_type_character( fmt[ i ] ) )
            s.type = fmt[ i++ ];
      }

      static void debug_output_spec ( spec & s )
      {
         printf( "(flag: %c, width: %d, precision: %d, prefix: %s)\n",
            s.flag, s.width, s.precision, s.prefix );
      }

      static int get_printf_bytes ( const char * fmt, va_list args )
      {
         int nbytes = 0;
         int nParamIdx = 0;
         for ( int i = 0; fmt[ i ]; )
         {
            if ( i > 0 && ( ( fmt[ i ] == '%' && fmt[ i - 1 ] == '%' )
               || ( fmt[ i ] == '%' && fmt[ i - 1 ] == '\\' ) ) )
            {
               nbytes++;
               i++;
               continue;
            }
            if ( fmt[ i ] == '%' && fmt[ i + 1 ] == '%' )
            {
               nbytes++;
               i++;
               continue;
            }
            if ( fmt[ i ] == '\n' )
            {
               nbytes += 2;
               i++;
               continue;
            }
            if ( fmt[ i ] == '%' )
            {
               spec s = { 0 };
               get_next_spec( fmt, s, i );
               if ( !s.type )
                  return 0;
               switch ( s.type )
               {
               case 'c':
                  nbytes += 1;
                  break;
               case 'C':
                  nbytes += 2;
                  break;
               case 'd': case 'i': case 'x': case 'X': case 'o': case 'u':
                  if ( s.prefix[ 0 ] == 'l' ) // lower case L
                     nbytes += get_spec_bytes( s, va_arg( args, __int64 ), get_scaler( s.type ) );
                  else if ( s.prefix[ 0 ] == 'h' )
                     nbytes += get_spec_bytes( s, va_arg( args, short int ), get_scaler( s.type ) );
                  else if ( s.prefix[ 0 ] == 'I' )
                  {
                     if ( s.prefix[ 1 ] == '3' && s.prefix[ 2 ] == '2' )
                        nbytes += get_spec_bytes( s, va_arg( args, int ), get_scaler( s.type ) );
                     if ( s.prefix[ 1 ] == '6' && s.prefix[ 2 ] == '4' )
                        nbytes += get_spec_bytes( s, va_arg( args, __int64 ), get_scaler( s.type ) );
                  }
                  else
                     nbytes += get_spec_bytes( s, va_arg( args, int ), get_scaler( s.type ) );
                  break;
               case 's':
                  nbytes += get_spec_bytes( s, va_arg( args, const char * ) );
                  break;
               case 'S':
                  nbytes += get_spec_bytes( s, va_arg( args, const wchar_t * ) );
                  break;
               case 'e': case 'E': case 'f': case 'g': case 'G': case 'a': case 'A':
                  nbytes += get_spec_bytes( s, va_arg( args, double ) );
                  break;
               }
            }
            else
            {
               ++nbytes;
               ++i;
            }
         }
         return nbytes;
      }
   };

   class extensible_printf_buffer
   {
      enum {
         minBufferSize = 4096,
         maxBufferSize = 8192 * 4,
      };
   public:
      extensible_printf_buffer ( size_t minSize = minBufferSize )
         : m_bufferSize( minSize )
         , m_bufferPtr( 0 )
         , m_buffer( NULL )
      {}
      ~extensible_printf_buffer ()
      {
         if ( m_buffer )
            free( m_buffer );
      }

      BOOL _cdecl printf ( const char * fmt, ... )
      {
         va_list args;
         va_start(args, fmt);

         if ( !m_buffer )
            m_buffer = ( char * )malloc( m_bufferSize );
         if ( !m_buffer )
            return FALSE;

         int n = maple::printfx::get_printf_bytes( fmt, args ) + 1;
         while ( left_size() < n && m_bufferSize < maxBufferSize )
            extend();
         if ( left_size() < n )
            return FALSE;
         n = vsprintf_s( &m_buffer[ m_bufferPtr ], left_size(), fmt, args );
         if ( n != -1 )
         {
            // printf succeeded
            m_bufferPtr += n;
         }
         else
            return FALSE;
         va_end( args );
         return n != -1 ? TRUE : FALSE;
      }

      BOOL vprintf( const char * fmt, va_list args )
      {
         if ( !m_buffer )
            m_buffer = ( char * )malloc( m_bufferSize );
         if ( !m_buffer )
            return FALSE;

         int n = maple::printfx::get_printf_bytes( fmt, args ) + 1;
         while ( left_size() < n && m_bufferSize < maxBufferSize )
            extend();
         if ( left_size() < n )
            return FALSE;
         n = vsprintf_s( &m_buffer[ m_bufferPtr ], left_size(), fmt, args );
         if ( n != -1 )
         {
            // printf succeeded
            m_bufferPtr += n;
         }
         else
            return FALSE;
         return n != -1 ? TRUE : FALSE;
      }

      int left_size ()
      {
         return m_bufferSize - m_bufferPtr;
      }

      int used_size ()
      {
         return m_bufferPtr;
      }

      const char * get_string ()
      {
         return m_buffer;
      }

      char * lock_buffer ()
      {
         return m_buffer;
      }

      void unlock_buffer ( char * p )
      {
         // nothing
      }

      void clear ()
      {
         memset( m_buffer, 0, m_bufferSize );
      }

      BOOL extend ()
      {
         char * buffer = ( char * )malloc( m_bufferSize * 2 );
         if ( !buffer )
            return FALSE;
         memset( buffer, 0, m_bufferSize * 2 );
         memcpy( buffer, m_buffer, m_bufferSize );
         free( m_buffer );
         m_buffer = buffer;
         m_bufferSize *= 2;
         return TRUE;
      }

   private:
      char * m_buffer;
      size_t m_bufferSize;
      size_t m_bufferPtr;
   };

   struct cache_buffer
   {
      cache_buffer ( char * buf, int size, bool auto_free = true )
         : m_buf( buf ), m_size( size ), m_auto_free( auto_free ), m_offset( 0 )
      {
         if ( !m_buf && m_size > 0 )
            m_buf = ( char * )malloc( m_size );
      }

      ~cache_buffer ()
      {
         if ( m_auto_free )
            free( m_buf );
      }

      int get_left_size ()
      {
         return m_size - m_offset;
      }

      int get_size ()
      {
         return m_size;
      }
      
      size_t read ( char * buf, int size )
      {
         if ( !buf || !size )
            return 0;
         size_t bytes = min( m_size - m_offset, size );
         if ( !bytes )
            return 0;
         memcpy( buf, &m_buf[ m_offset ], bytes );
         m_offset += bytes;
         return bytes;
      }

      size_t write ( char * buf, int size )
      {
         if ( !buf || !size )
            return 0;
         if ( !m_buf || get_left_size() < size )
            return 0;
         memcpy( m_buf, buf, size );
         return size;
      }

      size_t seek ( int offset, int origin )
      {
         switch ( origin )
         {
         case SEEK_SET:
            {
               if ( m_offset + offset < 0 )
                  m_offset = 0;
               else
                  m_offset = offset;
            }
            break;
         case SEEK_CUR:
         case SEEK_END:
            {
               if ( m_offset + offset < 0 )
                  m_offset = 0;
               else if ( m_offset + offset > m_size )
                  m_offset = m_size;
               else
                  m_offset += offset;
            }
            break;
         }
      }

   private:
      char *   m_buf;
      int      m_size;
      int      m_offset;
      bool     m_auto_free;
   };
}

#endif // DONT_INCLUDE_MP_PRINTFX_H