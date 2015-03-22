/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ E:\works\maple\include\mp_formated_liner_buffer.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_FORMATED_LINER_BUFFER_H
#define DONT_INCLUDE_MP_FORMATED_LINER_BUFFER_H

namespace maple {

   inline int roundupto( int val, int ceiling ) {
      return ( val + ceiling - 1 ) & ( ~(ceiling - 1));
   }

   enum {
      fmt_fourcc_int      = ' tni',
      fmt_fourcc_string   = ' rts',
      fmt_fourcc_blob     = 'bolb',
   };

   struct formated_field
   {
      int fourcc;
      size_t size;
      char data[1];
   };

   inline size_t get_formated_field_size ( formated_field * field )
   {
      return roundupto( field->size, 4 ) + sizeof( int ) + sizeof( size_t );
   }

   /*
   ** class formated_liner_buffer
   */
   class formated_liner_buffer
   {
   public:
      formated_liner_buffer ()
         : m_size( 4096 ), m_ptr( 0 ), m_buffer( NULL )
      {}

      ~formated_liner_buffer ()
      {
         if ( m_buffer )
            free( m_buffer );
      }
 
      bool put_int ( int val )
      {
         return put_formated_value( fmt_fourcc_int, &val, sizeof( val ) );
      }

      bool put_string ( const char * val )
      {
         return put_formated_value( fmt_fourcc_string, ( void * )val, strlen( val ) + 1 );
      }

      bool put_blob ( const char * val, size_t len )
      {
         return put_formated_value( fmt_fourcc_blob, ( void * )val, len );
      }

      const char * get_buffer ()
      {
         return m_buffer;
      }

      size_t used_size () 
      {
         return m_ptr;
      }

   protected:
      bool extend ()
      {
         if ( !m_buffer || !m_size )
            return false;
         size_t size = m_size * 2;
         char * buf = ( char * )malloc( size );
         if ( !buf )
            return false;
         // copy buffer
         memcpy( buf, m_buffer, m_ptr );
         // free old buffer
         free( m_buffer );
         // change buffer ptr and size value
         m_buffer = buf;
         m_size = size;
         return true;
      }

      bool put_formated_value ( int fourcc, void * buf, size_t len )
      {
         if ( !m_size )
            return false;
         if ( !m_buffer )
            m_buffer = ( char * )malloc( m_size );
         if ( !m_buffer )
            return false;
         
         // calculate required buffer size
         size_t placeholder = roundupto( len, 4 ) - len;
         size_t required = sizeof( fourcc ) + sizeof( len ) + len + placeholder;
         
         // extend buffer if necessary
         while ( m_size < m_ptr + required )
         {
            if ( !extend() )
               return false;
         }
         
         // set field value
         formated_field * field = ( formated_field * )&m_buffer[ m_ptr ];
         field->fourcc = fourcc;
         field->size = len;
         memcpy( field->data, buf, len );
         
         // move pointer
         m_ptr += required;

         return true;
      }

   private:
      size_t m_size;
      size_t m_ptr;
      char * m_buffer;
   };

   class formated_liner_buffer_reader
   {
   public:
      formated_liner_buffer_reader ( formated_liner_buffer & buf )
         : m_buffer( buf.get_buffer() ), m_size( buf.used_size() )
      {}

      ~formated_liner_buffer_reader ()
      {}

      bool get_field ( int idx, int * fourcc, int * len, const char ** buf )
      {
         if ( !m_buffer || !m_size )
            return false;

         size_t ptr = 0;
         for ( int i = 0; ptr < m_size; ++i )
         {
            int tfourcc = *( const int * )&m_buffer[ ptr ];
            ptr += sizeof( fourcc );
            int tlen = *( const int * )&m_buffer[ ptr ];
            ptr += sizeof( len );
            if ( i == idx )
            {
               if ( fourcc )
                  *fourcc = tfourcc;
               if ( len )
                  *len = tlen;
               if ( buf )
                  *buf = &m_buffer[ ptr ];
               return true;
            }
            ptr += roundupto( tlen, 4 );
         }

         return false;
      }

   private:
      const char *   m_buffer;
      size_t         m_size;
   };
}

#endif // DONT_INCLUDE_MP_FORMATED_LINER_BUFFER_H