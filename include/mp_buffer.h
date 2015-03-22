/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\advantech_trek\svn_xian\MRM_Video\ExpNetwork\pcl_xmemory.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef PCL_XMEMORY_H
#define PCL_XMEMORY_H

#define PACK_SIGNATURE 'KERT'

namespace maple {
namespace xmemory {

   // constant
   enum {
      winPageSize = 4096,
      maxRecvBufferSize = 0x00100000, // every sock has a recv buffer up to 128k bytes
   };

   /*
   ** Define some base memory type
   */

   // a memory pool
   struct dynamic_memory_pool
   {
      dynamic_memory_pool ()
         : buf_( (char*)VirtualAlloc( NULL, maxRecvBufferSize, MEM_RESERVE, PAGE_READWRITE ) )
         , size_( 0 )
         , used_( 0 )
      {
         increase( winPageSize ); // initial 1 page
      }

      ~dynamic_memory_pool ()
      {
         destroy();
      }

      bool is_valid () {
         return buf_ != NULL && size_ != 0;
      }

      char * get_buf_ptr () {
         return buf_;
      }

      size_t get_size () {
         return size_;
      }

      char * get_used_ptr () {
         return buf_;
      }

      size_t get_used_size () {
         return used_;
      }

      char * get_unused_ptr () {
         return buf_ + used_;
      }

      size_t get_unused_size () {
         return size_ - used_;
      }

      void set_used_size ( size_t size ) {
         used_ += size;
      }

      void reset_used_size () {
         used_ = 0;
      }

      bool increase ( size_t size )
      {
         size = roundupto( size, winPageSize );
         if ( !size )
            return false;
         if ( maxRecvBufferSize < size_ + size )
            return false;
         if ( ::VirtualAlloc( buf_ + size_, size, MEM_COMMIT, PAGE_READWRITE ) )
         {
            size_ += size;
            return true;
         }
         return false;
      }

   protected:
      void destroy ()
      {
         if ( buf_ )
         {
            ::VirtualFree( buf_, -1, MEM_RELEASE );
            buf_ = NULL;
            size_ = NULL;
         }
      }

   private:
      char * buf_;
      size_t size_;
      size_t used_;
   };

   struct fixed_memory_pool
   {
      fixed_memory_pool ( size_t size )
         : size_( roundupto( size, 4 ) )
         , buf_( NULL )
         , offset_( sizeof( size_t ) * 2 ) // reserved two int for signature and package size 
      {}

      ~fixed_memory_pool ()
      {
         if ( buf_ )
            free( buf_ );
      }

      bool put_int ( int v ) {
         return put_buf( (char *)&v, sizeof( int ) );
      }

      bool put_string ( const char * v ) {
         return put_buf( v, strlen( v ) + 1 );
      }

      bool put_blob ( const char * v, size_t size ) {
         return put_buf( v, size );
      }

      bool lock_for_send ( const char ** p_buf, size_t * p_size ) {
         if ( buf_ != NULL && offset_ > 0 ) {
            // reset the package size and signature
            *p_size = roundupto( offset_, 4 );
            if ( (int)*p_size < 0 )
               return false;
            size_t * p = ( size_t * )buf_;
            // set signature
            *p = PACK_SIGNATURE;
            // set the package length, please take care with following sentence,
            // the package length is 8 bytes less than the sending buffer size.
            *(p+1) = *p_size - sizeof( size_t ) * 2;
            *p_buf = buf_;
            return true;
         }
         return false;
      }

   protected:
      bool put_buf ( const char * v, size_t size ) {
         if ( !buf_ )
            buf_ = size_ ? ( char * )malloc( size_ ) : NULL;
         if ( !buf_ )
            return false;
         if ( offset_ + size <= size_ ) {
            // do copy
            memcpy( buf_ + offset_, v, size );
            offset_ += size;
            return true;
         }
         else {
            // extend buffer
            size_t new_size = size_ * 2;
            while ( new_size - size < offset_ )
               new_size *= 2;
            char * new_buf = ( char * )malloc( new_size );
            memcpy( new_buf, buf_, offset_ );
            free( buf_ );
            buf_ = new_buf;
            size_ = new_size;
            // offset doesn't change
            return put_buf( v, size );
         }
         return false;
      }
   private:
      char * buf_;
      size_t offset_;
      size_t size_;
   };

   /*
   ** Define some classes to access memory
   */
   
   // A memory iterator, the iterator doesn't change the content of memory,
   // but only read value form it.
   struct liner_memory_iter
   {
      liner_memory_iter ( const char * ptr, size_t max_len )
         : buf_( ptr )
         , max_len_( max_len )
         , offset_( 0 )
      {}

      bool is_valid ()
      {
         return offset_ < max_len_;
      }

      // query function doesn't affect offset_, call multiple times will get a same value
      int query_int ()
      {
         return *reinterpret_cast<const int *>( buf_ + offset_ );
      }

      // get will change the offset_, every got value cannot be got again.
      int get_int ()
      {
         int v = *reinterpret_cast<const int *>( buf_ + offset_ );
         offset_ += sizeof( int );
         return v;
      }

      const char * get_string ()
      {
         const char * v = reinterpret_cast<const char *>( buf_ + offset_ );
         offset_ += strlen( v ) + 1;
         return v;
      }

      const char * get_blob ( size_t size )
      {
         const char * v = buf_ + offset_;
         offset_ += size;
         return v;
      }

      const char * get_left () {
         return buf_ + offset_;
      }

      size_t get_unused_size () {
         if ( max_len_ > offset_ )
            return max_len_ - offset_;
         return 0;
      }

      size_t get_used_size () {
         return offset_;
      }

      void move_backward ( size_t val ) {
         offset_ -= val;
      }

   private:
      const char * buf_;
      size_t max_len_;
      size_t offset_;
   };
}}

#endif // PCL_XMEMORY_H