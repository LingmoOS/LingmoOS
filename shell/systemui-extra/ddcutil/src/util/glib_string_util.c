/** @file glib_string_util.c
 *
 *  Functions that depend on both glib_util.c and string_util.c.
 *
 *  glib_string_util.c/h exists to avoid circular dependencies within directory util.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

/** \cond */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** \endcond */

#include "string_util.h"

#include "glib_string_util.h"


/** Joins a GPtrArray containing pointers to character strings
 *  into a single string,
 *
 *  @param strings   GPtrArray of strings
 *  @param sepstr   if non-null, separator to insert between joined strings
 *
 *  @return joined string, caller is responsible for freeing
 */
char * join_string_g_ptr_array(GPtrArray* strings, char * sepstr) {
   bool debug = false;

   char * catenated = NULL;
   if (strings) {
      int ct = strings->len;
      if (debug)
         fprintf(stdout, "(%s) ct = %d\n", __func__, ct);
      char ** pieces = calloc(ct, sizeof(char*));
      int ndx;
      for (ndx=0; ndx < ct; ndx++) {
         pieces[ndx] = g_ptr_array_index(strings,ndx);
         if (debug)
            fprintf(stdout, "(%s) pieces[%d] = %s\n", __func__, ndx, pieces[ndx]);
      }
      catenated = strjoin((const char**) pieces, ct, sepstr);
      if (debug)
         fprintf(stdout, "(%s) strlen(catenated)=%zd, catenated=%p, catenated=|%s|\n",
                         __func__, strlen(catenated), catenated, catenated);

   #ifdef GLIB_VARIANT
      // GLIB variant failing when used with file.  why?
      Null_Terminated_String_Array ntsa_pieces = g_ptr_array_to_ntsa(strings);
      if (debug) {
         DBGMSG("ntsa_pieces before call to g_strjoinv():");
         null_terminated_string_array_show(ntsa_pieces);
      }
      // n. our Null_Terminated_String_Array is identical to glib's GStrv
      gchar sepchar = ';';
      gchar * catenated2 = g_strjoinv(&sepchar, ntsa_pieces);
      DBGMSF(debug, "catenated2=%p", catenated2);
      *pstring = catenated2;
      assert(strcmp(catenated, catenated2) == 0);
   #endif

      free(pieces);
   }
   else
      catenated = strdup("");
   return catenated;
}


/** Joins a GPtrArray containing pointers to character strings
 *  into a single string,
 *
 *  The result is returned in a thread-specific private buffer that is
 *  valid until the next call of this function in the current thread.
 *
 *  @param strings   GPtrArray of strings
 *  @param sepstr   if non-null, separator to insert between joined strings
 *
 *  @return joined string, do not free
 */

char * join_string_g_ptr_array_t(GPtrArray* strings, char * sepstr) {
   static GPrivate  buffer_key = G_PRIVATE_INIT(g_free);
   static GPrivate  buffer_len_key = G_PRIVATE_INIT(g_free);

   char * catenated = join_string_g_ptr_array(strings, sepstr);
   int required_size = strlen(catenated) + 1;
   char * buf = get_thread_dynamic_buffer(&buffer_key, &buffer_len_key, required_size);
   strncpy(buf, catenated, required_size);
   free(catenated);
   return buf;
}


/** Looks for a string in a **GPtrArray** of strings.
 *
 * @param haystack **GPtrArray** to search
 * @param needle   string to search for (case sensitive)
 * @return index of string if found, -1 if not found
 *
 * @remark
 * glib function **g_ptr_array_find_with_equal_funct()** is an obvious alternative,
 * but it requires glib version >= 2.54
 */
int gaux_string_ptr_array_find(GPtrArray * haystack, const char * needle) {
   int result = -1;
   for (int ndx = 0; ndx < haystack->len; ndx++) {
      if (streq(needle, g_ptr_array_index(haystack, ndx))) {
         result = ndx;
         break;
      }
   }
   return result;
}


/** Compares two instances of #GPtrArray of pointers to unique strings.
 *
 *  @param  first  first array
 *  @param  second second array
 *  @return true if they contain the same strings, false if not
 *
 *  @remark
 *  The string values are compared, not the pointers.
 *  @remark
 *  Order of the strings does not matter.
 */
bool gaux_unique_string_ptr_arrays_equal(GPtrArray *first, GPtrArray* second) {
   assert(first);
   assert(second);
   bool result = false;
   // assumes each entry in first and second is unique
   if (first->len == second->len) {
      result = true;
      for (int ndx = 0; ndx < first->len; ndx++) {
         guint found_index;
         gpointer cur = g_ptr_array_index(first, ndx);
         bool found = gaux_ptr_array_find_with_equal_func(second, cur, g_str_equal, &found_index);
         if (!found) {
            result = false;
            break;
         }
      }
   }
   return result;
}


/** Creates a new #GPtrArray of unique strings containing only those in the
 *  first array that are not in the second.
 *
 *  @param  first  first array
 *  @param  second second array
 *  @result new array containing the difference
 *
 *  @remark
 *  The returned array may have length 0.
 */
GPtrArray * gaux_unique_string_ptr_arrays_minus(GPtrArray *first, GPtrArray* second) {
   // returns first - second
      assert(first);
      assert(second);
      // to consider: only allocate result if there's actually a difference
      GPtrArray * result = g_ptr_array_new_with_free_func(g_free);
      guint found_index;
      for (int ndx = 0; ndx < first->len; ndx++) {
         gpointer cur = g_ptr_array_index(first, ndx);
         // g_ptr_array_find_with_equal_func() requires glib 2.54
         // instead use our own implementation
         bool found = gaux_ptr_array_find_with_equal_func(second, cur, g_str_equal, &found_index);
         if (!found) {
            g_ptr_array_add(result, strdup(cur));
         }
      }
      return result;
   }


/** Appends a copy of a string to a #GPtrArray of unique strings.
 *  If the new value already exists in the array, does nothing.
 *
 *  \param arry      array of unique strings
 *  \param new_value string to include, must be non-null
 */
void gaux_unique_string_ptr_array_include(GPtrArray * arry, char * new_value) {
   bool debug = false;
   if (debug)
      printf("(%s) new_value=|%s|\n", __func__, new_value);
   assert(new_value);
   assert(arry);
   if (new_value) {   // ignore bad argument if asserts disabled
      int ndx = 0;
      for (; ndx < arry->len; ndx++) {
         char * old_value = g_ptr_array_index(arry, ndx);
         if (streq(new_value, old_value) ) {
            if (debug) printf("(%s) Found. ndx=%d\n", __func__, ndx);
            break;
         }
      }
      if (ndx == arry->len) {
         if (debug)
            printf("(%s) Appending new value\n", __func__);
         g_ptr_array_add(arry, strdup(new_value));
      }
   }
}
