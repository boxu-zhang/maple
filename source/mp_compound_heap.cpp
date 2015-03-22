/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\source\mp_compound_heap.cpp
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#include <windows.h>
#include "mp_compound_heap.h"

// global variables for this cpp, please attention that new and delete
// cannot stay within any namespace. They must be global.

// All new/delete operations in this file operate the self management heap
// Using global variables is not safe, we need to make it singleton.
// Why I don't make a global variables here but instead of this piece of
// complex code?
// Because the sequence of destruction of global OBJECT are hard to control.
// For example, we make a global compound_heap object, because of the dependency
// , we must make sure that compound_heap destructing before this class, which
// we cannot make it.
class reserved_tiny_heap
   : maple::tiny_heap
{
public:
   static maple::tiny_heap * instance ()
   {
      maple::auto_lock<maple::critical_section> lock( _cs );
      if ( !_instance )
      {
         _instance = new reserved_tiny_heap();
      }
      return _instance;
   }

   // the reference count are necessary, because the only class that depend this
   // class can create more than 1 instance. And all of the instances share
   // same instance of this class.
   static void addref ()
   {
      InterlockedIncrement( (LONG*)&_refcount );
   }

   static void release ()
   {
      if ( !InterlockedDecrement( (LONG*)&_refcount ) )
      {
         maple::auto_lock<maple::critical_section> lock( _cs );
         if ( _instance )
            delete _instance;
      }
   }

protected:
   reserved_tiny_heap ()
      : maple::tiny_heap( maple::constant::max_slot_size_of_tiny_heap )
   {}
   ~reserved_tiny_heap ()
   {}

   void * _cdecl operator new ( size_t size )
   {
      return &_buf[ 0 ];
   }

   // do nothing in delete
   void _cdecl operator delete ( void * ptr )
   {}

   static char _buf[ sizeof( maple::tiny_heap ) ];
   static maple::critical_section _cs;
   static reserved_tiny_heap * _instance;
   static unsigned int _refcount;
};

char reserved_tiny_heap::_buf[ sizeof(maple::tiny_heap) ] = { 0 };
maple::critical_section reserved_tiny_heap::_cs;
reserved_tiny_heap * reserved_tiny_heap::_instance = NULL;
unsigned int reserved_tiny_heap::_refcount = 0;

static void * _cdecl operator new ( size_t size )
{
   return reserved_tiny_heap::instance()->alloc( size );
}

static void _cdecl operator delete ( void * ptr )
{
   reserved_tiny_heap::instance()->dealloc( ptr );
}

namespace maple {

   // implement tiny heap
   tiny_heap::tiny_heap ( size_t slot_size /* = sizeof( int ) */ )
      : m_base( VirtualAlloc( NULL, constant::page_size, MEM_COMMIT, PAGE_READWRITE ) )
      , m_max_size( constant::page_size )
      , m_slot_size( clamp( roundupto( slot_size, sizeof( int ) ),
         constant::min_slot_size_of_tiny_heap, constant::max_slot_size_of_tiny_heap ) )
   {
      if ( validate() )
      {
         int * base_addr = (int*)m_base;
         for ( size_t i = 0; i < m_max_size / sizeof( int ); ++i )
            base_addr[ i ] = tiny_heap_block_flag_free;
      }
   }

   tiny_heap::~tiny_heap ()
   {
      if ( validate() )
         VirtualFree( m_base, constant::page_size, MEM_DECOMMIT | MEM_RELEASE );
   }

   // overwrites of base class heap
   bool tiny_heap::validate ()
   {
      // check whether the heap is validate, navigate all nodes and check
      // either the flag is correct or not
      return m_base != NULL ? true : false;
   }

   bool tiny_heap::is_empty ()
   {
      for ( size_t i = 0; i < m_max_size / sizeof( int ); ++i )
      {
         if ( ((int*)m_base)[i] == tiny_heap_block_flag_busy )
            return false;
      }
      return true;
   }

   bool tiny_heap::get_heap_protrait ( heap_protrait & protrait )
   {
      if ( !validate() )
         return false;

      for ( size_t i = 0; i < m_max_size / sizeof( int ); i += m_slot_size / sizeof( int ) )
      {
         if ( ((int*)m_base)[i] != tiny_heap_block_flag_free )
         {
            protrait.nodes ++;
            protrait.total_size += m_slot_size;

            // please not that here we won't get an precise overhead value
            // all busy but not initialized slot are considered as overhead
            char * p = (char*)&((int*)m_base)[i];
            for ( size_t j = 1; j < m_slot_size; j++ )
            {
               if ( p[ j ] == tiny_heap_block_flag_busy )
               {
                  protrait.overhead_size ++;
               }
            }
         }
      }

      return true;
   }

   bool tiny_heap::is_allowed_size ( size_t size )
   {
      return 0 < size && size <= m_slot_size ? true : false;
   }

   bool tiny_heap::is_valid_pointer ( void * ptr )
   {
      if ( ptr < m_base )
         return false;
      return (size_t)( (char*)ptr - (char*)m_base ) < m_max_size ? true : false;
   }

   void * tiny_heap::alloc ( size_t size )
   {
      // check size 
      if ( !is_allowed_size( size ) )
         return NULL;

      // go find a serial of slots that can held required size
      size_t required_size = m_slot_size;

      // here we use a trick, using memcmp to find a suitable segment.
      // First, we setup a fixed piece of memory which size is exactly
      // max_slot_size_of_tiny_heap. And when we compare it with 
      // existing slot, we use the m_slot_size as the size argument in
      // memcpy.
      struct {
         int v0, v1, v2, v3,
            v4, v5, v6, v7;
      } node_to_be_compared = { 0 };
      memset( &node_to_be_compared, tiny_heap_block_flag_free, sizeof( node_to_be_compared ) );

      for ( size_t offset = 0; offset < m_max_size; offset += required_size )
      {
         if ( !memcmp( (char*)m_base + offset, &node_to_be_compared, required_size ) )
         {
            // we found it, and mark it as busy and return it
            memset( (char*)m_base + offset, tiny_heap_block_flag_busy, required_size );
            return (char*)m_base + offset;
         }
      }

      // just failed with NULL
      return NULL;
   }

   bool tiny_heap::dealloc ( void * ptr )
   {
      // validate parameter
      if ( !is_valid_pointer( ptr ) )
         return false;

      // just change flag to dealloc it
      memset( (char*)ptr, tiny_heap_block_flag_free, m_slot_size );
      return true;
   }

   // implementation of regular_heap
   regular_heap::regular_heap ()
      : m_base( NULL )
      , m_reserved( constant::def_reserve_page_count_of_regular_heap )
      , m_commited( constant::def_commit_page_count_of_regular_heap )
      , m_latest_alloc( NULL )
      , m_latest_dealloc( NULL )
   {
      m_base = VirtualAlloc( NULL,
         constant::page_size * constant::def_reserve_page_count_of_regular_heap,
         MEM_RESERVE, PAGE_READWRITE );
      if ( m_base )
      {
         // commit one page
         void * p = VirtualAlloc( m_base,
            constant::page_size * constant::def_commit_page_count_of_regular_heap,
            MEM_COMMIT, PAGE_READWRITE );
         if ( !p )
         {
            VirtualFree( m_base,
               constant::page_size * constant::def_reserve_page_count_of_regular_heap,
               MEM_RELEASE );
            m_base = NULL;
         }
         else
         {
            // initialize the head node
            block * head = ( block * )m_base;
            // using -1 to indicate the block is free
            head->size = -1;
            // using 0 to indicate there is no next block
            head->offset_to_next = 0;
            // using 0 to indicate there is no previous block
            head->offset_to_previous = 0;
         }
      }
   }

   regular_heap::~regular_heap ()
   {
      if ( m_base )
      {
         VirtualFree( m_base,
            constant::page_size * constant::def_reserve_page_count_of_regular_heap,
            MEM_RELEASE );
         m_base = NULL;
      }
   }

   // overwrites of base class heap
   bool regular_heap::validate ()
   {
      return m_base != NULL ? true : false;
   }

   bool regular_heap::is_empty ()
   {
      return ((regular_heap::block*)m_base)->size == -1 ? true : false;
   }

   bool regular_heap::get_heap_protrait ( heap_protrait & protrait )
   {
      if ( !validate() )
         return false;

      block * head = ( block * )m_base;
      for ( block * p_block = head; p_block; p_block = get_next_block( p_block ) )
      {
         if ( p_block->size != -1 )
         {
            protrait.nodes ++;
            protrait.total_size += p_block->size;
            if ( p_block->offset_to_next )
               protrait.overhead_size += p_block->offset_to_next - p_block->size;
         }
      }

      return true;
   }

   bool regular_heap::is_allowed_size ( size_t size )
   {
      return constant::min_size_of_regular_heap < size
         && size <= constant::max_size_of_regular_heap ? true : false;
   }

   bool regular_heap::is_valid_pointer ( void * ptr )
   {
      if ( ptr < m_base )
         return false;

      return ( size_t )((char*)ptr - (char*)m_base) < m_reserved * constant::page_size
         ? true : false;
   }

   void * regular_heap::alloc ( size_t size )
   {
      // check size
      if ( !is_allowed_size( size ) )
         return NULL;

      // validate heap
      if ( !validate() )
         return NULL;

      // find a free block that can held this required size of memory
      void * p = NULL;
      do 
      {
         // try recent allocated 
         p = alloc_in_block( m_latest_alloc, size );
         if ( p )
            break;

         // try recent released
         p = alloc_in_block( m_latest_dealloc, size );
         if ( p )
            break;

         // try from the head
         block * head = ( block * )m_base;
         for ( block * p_block = head; p_block; p_block = get_next_block( p_block ) )
         {
            if ( is_block_free( p_block ) && size <= get_available_block_bytes( p_block ) )
            {
               p = alloc_in_block( p_block, size );
               if ( p )
                  break;
            }
         }

         if ( p )
            break;

      } while ( false );

      return p;
   }

   bool regular_heap::dealloc ( void * ptr )
   {
      if ( !is_valid_pointer( ptr ) )
         return false;

      return dealloc_block( (regular_heap::block*)ptr - 1 );
   }

   void * regular_heap::alloc_in_block ( regular_heap::block * this_block, size_t size )
   {
      if ( !this_block )
         return NULL;
      block backup_this_block = *this_block;
      // check block size whether it is enough to held this new block
      if ( is_block_free( this_block ) && size <= get_available_block_bytes( this_block ) )
      {
         this_block->size = size;
         this_block->offset_to_next = sizeof( *this_block ) + size;
         // Do we need to submit more pages?
         block * next_block = get_next_block( this_block );

         // The next_block must be mapped with physical pages, we need it.
         if ( m_commited * constant::page_size
            < absolute_offset_of_block( next_block ) + sizeof( *next_block ) )
         {
            // we need it
            size_t required_page_bytes = roundupto(
               absolute_offset_of_block( next_block ) + sizeof( *next_block ),
               constant::page_size );
            if ( !VirtualAlloc( (char*)m_base + m_commited * constant::page_size,
               required_page_bytes, MEM_COMMIT, PAGE_READWRITE ) )
            {
               // it is failed, that no more physical page available
               // we need to reset this_block
               *this_block = backup_this_block;
               return NULL;
            }

            // update committed page count
            m_commited += required_page_bytes / constant::page_size;

            // update next_block 
            next_block->size = -1;
            next_block->offset_to_next = 0;
            next_block->offset_to_previous =
               absolute_offset_of_block( next_block ) - absolute_offset_of_block( this_block );
         }

         // change latest allocated block to next
         m_latest_alloc = get_next_block( this_block );

         // step over the block structure which is used internally for heap management
         // and shouldn't be exposed.
         return (char*)(this_block + 1);
      }
      return NULL;
   }

   /*
   ** Deallocate is a little complicated, we need to consider merge free memory blocks here
   */
   bool regular_heap::dealloc_block ( regular_heap::block * this_block )
   {
      // The merge order are inversed from allocation order, which means it
      // is from high to low of memory address.

      // find the farthest free block from this block in direction of next block.
      regular_heap::block * merge_start_block = get_previous_block( this_block );
      while ( merge_start_block )
      {
         if ( merge_start_block->size != -1 )
         {
            merge_start_block = get_previous_block( merge_start_block );
            break;
         }
         merge_start_block = get_next_block( merge_start_block ) ;
      }

      if ( !merge_start_block )
         merge_start_block = this_block;

      // start merge until encounter a allocated node in direction of previous block
      while ( merge_start_block )
      {
         // set size of start block to -1
         merge_start_block->size = -1;

         regular_heap::block * previous = get_previous_block( merge_start_block );
         if ( previous )
         {
            // if it is also free, merge them
            if ( previous->size == -1 )
            {
               // with this technic, we step over the merge_start_block in next allocation,
               // which exactly is the merging operation want.
               previous->offset_to_next = merge_start_block->offset_to_next;
            }
            else
               break;
         }

         // use previous node as next start block
         merge_start_block = previous;
      }

      // change latest deallocate block
      m_latest_dealloc = merge_start_block;

      // after the merging operation, the merge_start_block is a big block that replace
      // all coherent free blocks. Check shall we need to decommit some pages
      if ( !merge_start_block->offset_to_next )
      {
         // calculate parameter for decommit
         void * decommit_start_address = (void*)roundupto(
            (int)merge_start_block, constant::page_size );
         size_t decommit_bytes = m_commited * constant::page_size - 
            (size_t)( (char*)decommit_start_address - (char*)m_base );

         // calculate how many pages that we can decommit
         VirtualFree( decommit_start_address, decommit_bytes, MEM_DECOMMIT );
      }

      return true;
   }

   // implementation of page heap
   page_heap::page_heap ()
      : m_allocated_head( NULL )
   {}

   page_heap::~page_heap ()
   {
      // free list
      page_heap_node * p = m_allocated_head;
      while ( p )
      {
         page_heap_node * next = p->next;
         delete p;
         p = next;
      }
   }

   bool page_heap::validate ()
   {
      return true;
   }

   bool page_heap::is_empty ()
   {
      return m_allocated_head == NULL ? true : false;
   }

   bool page_heap::get_heap_protrait ( heap_protrait & protrait )
   {
      if ( !validate() )
         return false;

      for ( page_heap_node * node = m_allocated_head; node; node = node->next )
      {
         protrait.nodes++;
         protrait.total_size += node->size;
         protrait.overhead_size += ( roundupto( node->size, constant::page_size ) - node->size );
      }

      return true;
   }

   bool page_heap::is_allowed_size ( size_t size )
   {
      return constant::min_size_of_page_heap < size &&
         size <= constant::max_size_of_page_heap ? true : false;
   }

   bool page_heap::is_valid_pointer ( void * ptr )
   {
      if ( ( (size_t)ptr % constant::page_size ) != (size_t)ptr )
         return false;

      // navigate allocated list
      for ( page_heap_node * node = m_allocated_head; node; node = node->next )
      {
         if ( node->base_addr == ptr )
            return true;
      }

      return false;
   }

   void * page_heap::alloc ( size_t size )
   {
      size_t required_size = roundupto( size, constant::page_size );
      if ( !is_allowed_size( required_size ) )
         return NULL;

      void * p = VirtualAlloc( NULL, required_size, MEM_COMMIT, PAGE_READWRITE );
      if ( !p )
         return NULL;

      page_heap_node * node = new page_heap_node;
      if ( node )
      {
         node->base_addr = p;
         node->size = size;
         node->next = m_allocated_head;
         m_allocated_head = node;
      }

      return p;
   }

   bool page_heap::dealloc ( void * ptr )
   {
      if ( !ptr )
         return false;

      for ( page_heap_node * node = m_allocated_head; node; )
      {
         if ( node->base_addr == ptr )
         {
            // remove node
            page_heap_node * node_to_be_free = node;
            node = node->next;
            // do virtual free
            VirtualFree( ptr,
               roundupto( node_to_be_free->size, constant::page_size ), MEM_RELEASE );
            // delete node
            delete node_to_be_free;
         }
         else
            node = node->next;
      }
      return false;
   }

   // implementation of compound heap
   compound_heap::compound_heap ()
   {
      // must initialize the reserved tiny heap
      reserved_tiny_heap::addref();
   }

   compound_heap::~compound_heap ()
   {
      // clean up
      compound_heap_node * node = m_heaps;
      while ( node != NULL )
      {
         delete node->ptr;
         node = remove_heap_node( node );
      }
      reserved_tiny_heap::release();
   }

   void * compound_heap::alloc ( size_t size )
   {
#if MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD
      auto_lock<critical_section> lock( m_cs );
#endif
      // validate parameter
      if ( !size )
         return NULL;

      // try alloc from heap list
      void * p = alloc_from_heap_list( size );
      if ( p )
         return p;
      
      // none of heap have enough space, we need a new heap
      heap * p_heap = NULL;

      do 
      {
         if ( size < constant::max_slot_size_of_tiny_heap )
         {
            p_heap = new tiny_heap( size );
            break;
         }

         if ( size < constant::max_size_of_regular_heap )
         {
            p_heap = new regular_heap();
            break;
         }

         if ( size < constant::max_size_of_page_heap )
         {
            p_heap = new page_heap();
            break;
         }
      } while ( false );

      // none of heap can be created.
      if ( !p_heap )
         return NULL;

      // insert heap node
      compound_heap_node * node = new compound_heap_node;
      if ( !node )
      {
         delete p_heap;
         return NULL;
      }

      // initialize node member
      node->ptr = p_heap;
      node->previous = NULL;
      node->next = NULL;

      // insertion
      insert_heap_node( node );

      // allocate at new heap
      if ( p_heap->validate() )
      {
         // do allocation
         p = p_heap->alloc( size );
      }

      return p;
   }

   void compound_heap::dealloc ( void * ptr )
   {
#if MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD
      auto_lock<critical_section> lock( m_cs );
#endif
      dealloc_from_heap_list( ptr );
   }

   void * compound_heap::alloc_from_heap_list ( size_t size )
   {
      for ( compound_heap_node * node = m_heaps;
         node != NULL; node = node->next )
      {
         if ( node->ptr->validate() )
         {
            if ( node->ptr->is_allowed_size( size ) )
            {
               // try alloc 
               void * p = node->ptr->alloc( size );
               if ( p )
                  return p;
            }
         }
      }
      return NULL;
   }

   void compound_heap::dealloc_from_heap_list ( void * ptr )
   {
      compound_heap_node * node = m_heaps;
      while ( node != NULL )
      {
         if ( node->ptr->validate() && node->ptr->is_valid_pointer( ptr ) )
         {
            node->ptr->dealloc( ptr );
            if ( node->ptr->is_empty() )
            {
               delete node->ptr;
               node = remove_heap_node( node );
            }

            // please take care of this return, if it is missing, the linked list
            // of heap node will cause an exception, because after remove_heap_node
            // the node is already deleted.
            return;
         }
         else
         {
            node = node->next;
         }
      }
   }

   compound_heap::compound_heap_node * compound_heap::insert_heap_node (
      compound_heap::compound_heap_node * node )
   {
      if ( !node )
         return NULL;
      if ( !m_heaps )
      {
         m_heaps = node;
      }
      else
      {
         node->next = m_heaps;
         m_heaps->previous = node;
         m_heaps = node;
      }

      return node;
   }

   compound_heap::compound_heap_node * compound_heap::remove_heap_node (
      compound_heap::compound_heap_node * node )
   {
      if ( !node )
         return NULL;
      compound_heap_node * next_node = node->next;
      if ( node->previous )
      {
         node->previous->next = node->next;
         if ( node->next )
            node->next->previous = node->previous;
         delete node;
      }
      return next_node;
   }

#include <stdio.h>
   // dump heap information into stderr
   void compound_heap::dump ()
   {
      printf( "maple: begin dump heap informations\n" );

#if MP_COMPOUND_HEAP_ENABLE_MUTLI_THREAD
      auto_lock<critical_section> lock( m_cs );
#endif

      for ( compound_heap_node * node = m_heaps;
         node != NULL; node = node->next )
      {
         heap_protrait protrait = { 0 };
         if ( node->ptr->get_heap_protrait( protrait ) )
         {
            printf(
               "protrait of heap 0x%08x:\n\tnodes = %d, total_size = %d, overhead_size = %d, overhead_rate = %f\n",
               node->ptr, protrait.nodes, protrait.total_size, protrait.overhead_size, (float)protrait.overhead_size / protrait.total_size );
         }
      }

      printf( "maple: heap dump end\n" );
   }
}