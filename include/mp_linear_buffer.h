/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\include\mp_linear_buffer.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_LINEAR_BUFFER_H
#define DONT_INCLUDE_MP_LINEAR_BUFFER_H

namespace maple {

   struct linear_buffer
   {
      // constants
      enum {
         defaultSize = 4096
      };

      // methods
      linear_buffer ( size_t default_size = defaultSize )
         : buf_( 0 )
         , size_( default_size )
         , ptr_( 0 )
      {}

      ~linear_buffer ()
      {
         if ( buf_ )
            free( buf_ );
      }

      bool write ( void * obj, unsigned int size )
      {
         if ( !obj || !size )
            return false;
         if ( size > left_size() && !extend() )
            return false;
         memcpy( &buf_[ ptr_ ], obj, size );
         ptr_ += size;
         return true;
      }

      const char * get_buf ( unsigned int offset = 0 ) {
         if ( offset < size_ )
            return &buf_[ offset ];
         return NULL;
      }

      unsigned int used_size () {
         return ptr_;
      }

      unsigned int left_size () {
         return size_ - ptr_;
      }

      void clear () {
         ptr_ = 0;
      }

   protected:
      bool extend ()
      {
         char * buf = ( char * )malloc( size_ * 2 );
         if ( !buf )
            return false;
         memcpy( buf, buf_, size_ );
         free( buf_ );
         buf_ = buf;
         size_ *= 2;
         return true;
      }

   private:
      unsigned int   size_;
      unsigned int   ptr_;
      char *         buf_;
   };
}

typedef maple::linear_buffer mpLinearBuffer;

#endif // DONT_INCLUDE_MP_LINEAR_BUFFER_H