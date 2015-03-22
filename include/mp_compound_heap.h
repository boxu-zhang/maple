/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\include\mp_compound_heap.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef MP_COMPOUND_HEAP_H
#define MP_COMPOUND_HEAP_H

// support thread lock
#include "mp_locks.h"

/*
** Compile configurations
*/
#ifndef MP_COMPOUND_HEAP_DEFAULT_PAGE_HEAP_MAX_SIZE
#define MP_COMPOUND_HEAP_DEFAULT_PAGE_HEAP_MAX_SIZE ( 10 * 1024 * 1024 )
#endif // MP_COUMPOUND_HEAP_DEFAULT_PAGE_HEAP_MAX_SIZE

#ifndef MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD
#define MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD 1
#endif // MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD

namespace maple {

   struct constant {
      enum {
         page_size = 4096,
         def_reserve_page_count_of_regular_heap = 100,
         def_commit_page_count_of_regular_heap = 1,
         min_slot_size_of_tiny_heap = 4,
         max_slot_size_of_tiny_heap = 32,
         min_size_of_regular_heap = 32,
         max_size_of_regular_heap = page_size,
         min_size_of_page_heap = page_size,
         max_size_of_page_heap = MP_COMPOUND_HEAP_DEFAULT_PAGE_HEAP_MAX_SIZE,
         is_multithread_enabled = MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD,
      };
   };

   // inline functions
   inline int roundupto( int val, int ceiling ) {
      return ( val + ceiling - 1 ) & ( ~(ceiling - 1));
   }

   inline int clamp( int val, int min, int max ) {
      if ( min < max ) {
         if ( val < min )
            return min;
         if ( max < val )
            return max;
         return val;
      }
      else if ( min == max ) {
         return min;
      }
      return clamp( val, max, min );
   }

   struct heap_protrait
   {
      size_t nodes;
      size_t total_size;
      size_t overhead_size;
   };

   class heap
   {
   public:
      // default constructor
      heap () {}
      // virtual destructor for delete
      virtual ~heap () {}

      // to be overwritten
      virtual bool validate () = 0;
      virtual bool is_empty () = 0;
      virtual bool get_heap_protrait ( heap_protrait & protrait ) = 0;
      virtual bool is_allowed_size ( size_t size ) = 0;
      virtual bool is_valid_pointer ( void * ptr ) = 0;
      virtual void * alloc ( size_t size ) = 0;
      virtual bool dealloc ( void * ptr ) = 0;
   };

   /*
   ** This heap supports allocation less or equal than 4 bytes
   */
   class tiny_heap
      : public heap
   {
      enum {
         tiny_heap_block_flag_free = (int)-1,
         tiny_heap_block_flag_busy = (int)-2,
      };
   public:
      tiny_heap ( size_t slot_size = sizeof( int ) );
      ~tiny_heap ();

      // overwrites of base class heap
      virtual bool validate ();
      virtual bool is_empty ();
      virtual bool get_heap_protrait ( heap_protrait & protrait );
      virtual bool is_allowed_size ( size_t size );
      virtual bool is_valid_pointer ( void * ptr );
      virtual void * alloc ( size_t size );
      virtual bool dealloc ( void * ptr );

   private:
      void * m_base;
      size_t m_max_size;
      size_t m_slot_size;
   };

   /*
   ** This heap supports allocation less than 1 page but greater than 4 bytes
   */
   class regular_heap
      : public heap
   {
      // nested class that describe heap blocks
      struct block
      {
         // we used impossible value of memory block size as a flag to 
         // indicate whether this block is free or not. see inline 
         // function is block free
         size_t size;
         size_t offset_to_next;
         size_t offset_to_previous;
      };
   public:
      regular_heap ();
      ~regular_heap ();

      // overwrites of base class heap
      virtual bool validate ();
      virtual bool is_empty ();
      virtual bool get_heap_protrait ( heap_protrait & protrait );
      virtual bool is_allowed_size ( size_t size );
      virtual bool is_valid_pointer ( void * ptr );
      virtual void * alloc ( size_t size );
      virtual bool dealloc ( void * ptr );

   protected:
      void * alloc_in_block ( block * this_block, size_t size );
      bool dealloc_block ( block * this_block );
      // make auxiliary inline function for navigating blocks
      inline bool is_block_free ( block * this_block )
      {
         return this_block->size == -1 ? true : false;
      }

      inline size_t get_available_block_bytes ( block * this_block )
      {
         return this_block->offset_to_next ?
            this_block->offset_to_next - sizeof( block ) :
            m_reserved * constant::page_size - absolute_offset_of_block( this_block ) - sizeof( block );
      }

      inline block * get_next_block ( block * this_block )
      {
         return this_block != NULL && this_block->offset_to_next ?
            (block*)((char*)this_block + this_block->offset_to_next) : NULL;
      }

      // please attention that we use a subtraction but not addition here.
      inline block * get_previous_block ( block * this_block )
      {
         return this_block != NULL && this_block->offset_to_previous ?
            (block*)((char*)this_block - this_block->offset_to_previous) : NULL;
      }

      inline size_t absolute_offset_of_block ( block * this_block )
      {
         return this_block >= m_base ? (size_t)((char*)this_block - (char*)m_base) : -1;
      }

   private:
      // necessary heap properties
      void * m_base;     // base address of this heap
      size_t m_reserved; // in pages
      size_t m_commited; // in pages

      // for increasing heap performance
      block * m_latest_alloc;
      block * m_latest_dealloc;
   };

   /*
   ** This heap supports allocation less than 10MB but greater than 1 page.
   ** Note: Why it's 10MB but not any other value? Because in common scene,
   ** the most biggest data objects such as pictures, audio clips, part of
   ** video clips, and etc, are not exceed this value. It's an empirical
   ** value, and you can change it by adding a macro at compile command
   ** line. For detail of it, see MP_DEFAULT_PAGE_HEAP_MAX_SIZE at head
   ** of this file
   */
   class page_heap
      : public heap
   {
      struct page_heap_node
      {
         void * base_addr;
         size_t size;
         page_heap_node * next;
      };
   public:
      page_heap ();
      ~page_heap ();

      // overwrites of base class heap
      virtual bool validate ();
      virtual bool is_empty ();
      virtual bool get_heap_protrait ( heap_protrait & protrait );
      virtual bool is_allowed_size ( size_t size );
      virtual bool is_valid_pointer ( void * ptr );
      virtual void * alloc ( size_t size );
      virtual bool dealloc ( void * ptr );

   private:
      page_heap_node * m_allocated_head;
   };

   /*
   ** The compound heap receives all of the incoming allocations and 
   ** dispatches them to different heap implementation according to 
   ** the input size.
   */
   class compound_heap
   {
      struct compound_heap_node 
      {
         heap * ptr;
         compound_heap_node * previous;
         compound_heap_node * next;
      };

   public:
      compound_heap ();
      ~compound_heap ();

      void * alloc ( size_t size );
      void dealloc ( void * ptr );
      void dump ();

   protected:
      void * alloc_from_heap_list ( size_t size );
      void dealloc_from_heap_list ( void * ptr );

      // inline functions
      compound_heap_node * insert_heap_node ( compound_heap_node * node );
      compound_heap_node * remove_heap_node ( compound_heap_node * node );

   private:
      compound_heap_node * m_heaps;
      
      // for multiple threads
#if MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD
      critical_section m_cs;
#endif
   };
}

#endif // MP_COMPOUND_HEAP_H