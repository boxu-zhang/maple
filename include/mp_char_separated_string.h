/*******************************************************************************
Copyright (c) 1983-2009 Advantech Co., Ltd.
********************************************************************************
THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY INFORMATION
WHICH IS THE PROPERTY OF ADVANTECH CORP., ANY DISCLOSURE, USE, OR REPRODUCTION,
WITHOUT WRITTEN AUTHORIZATION FROM ADVANTECH CORP., IS STRICTLY PROHIBITED. 

================================================================================
REVISION HISTORY
--------------------------------------------------------------------------------
$Log:$ e:\works\maple\project\static\mp_char_separated_string.h
 
--------------------------------------------------------------------------------
$NoKeywords:$
*/

#ifndef DONT_INCLUDE_MP_CHAR_SEPARATED_STRING_H
#define DONT_INCLUDE_MP_CHAR_SEPARATED_STRING_H

namespace maple {

   struct char_separated_string
   {
      char_separated_string ( const char * str, char c = ',' )
         : str_( _strdup( str ) )
         , max_index_( 1 )
      {
         for ( int i = 0; str_[ i ]; ++i )
         {
            if ( str_[ i ] == c )
            {
               str_[ i ] = '\0';
               max_index_ ++;
            }
         }
      }

      ~char_separated_string ()
      {
         if ( str_ )
            free( str_ );
      }

      const char * get_sub_string ( int idx )
      {
         if ( idx < max_index_ )
         {
            if ( !idx )
               return str_;

            char * p = str_;
            for ( ; idx != 0; --idx )
            {
               p += strlen( p );
               p++;
            }
            return p;
         }
         return NULL;
      }

      int sub_string_count () {
         return max_index_;
      }

   private:
      int max_index_;
      char * str_;
   };
}

#endif // DONT_INCLUDE_MP_CHAR_SEPARATED_STRING_H