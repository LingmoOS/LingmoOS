/** @file glib_util.c
 *
 *  Utility functions for glib.
 */

// Copyright (C) 2014-2021 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

/** \cond */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** \endcond */

#include "glib_util.h"
#include "string_util.h"

#ifdef ALTERNATIVE

// create private copy of g_hash_table_get_keys_as_array()

/**
 * g_hash_table_get_keys_as_array:
 * @hash_table: a #GHashTable
 * @length: (out): the length of the returned array
 *
 * Retrieves every key inside @hash_table, as an array.
 *
 * The returned array is %NULL-terminated but may contain %NULL as a
 * key.  Use @length to determine the true length if it's possible that
 * %NULL was used as the value for a key.
 *
 * Note: in the common case of a string-keyed #GHashTable, the return
 * value of this function can be conveniently cast to (gchar **).
 *
 * You should always free the return result with g_free().  In the
 * above-mentioned case of a string-keyed hash table, it may be
 * appropriate to use g_strfreev() if you call g_hash_table_steal_all()
 * first to transfer ownership of the keys.
 *
 * Returns: (array length=length) (transfer container): a
 *   %NULL-terminated array containing each key from the table.
 *
 * Since: 2.40
 **/

gpointer *
g_hash_table_get_keys_as_array_local (GHashTable *hash_table,
                                      guint      *length)
{
  gpointer *result;
  guint i, j = 0;

  result = g_new(gpointer, hash_table->nnodes + 1);
  for (i = 0; i < hash_table->size; i++)
    {
      if (HASH_IS_REAL (hash_table->hashes[i]))
        result[j++] = hash_table->keys[i];
    }
  g_assert_cmpint (j, ==, hash_table->nnodes);
  result[j] = NULL;

  if (length)
    *length = j;

  return result;
}
#endif


/** Converts a doubly linked list of pointers into a null-terminated array
 * of pointers.
 *
 * @param  glist     pointer to doubly linked list
 * @param  length    where to return number of items in the allocated array,
 *                   (not including the final NULL terminating entry
 *
 * @return pointer to the newly allocated gpointer array
 *
 * @remark The pointers in the linked list are copied to the newly allocated array.
 *   The data pointed to is not duplicated.
 * @remark This function is needed because glib function g_hash_table_get_keys_as_array()
 *   does not exist in glib versions less than 2.40
 */
gpointer *
g_list_to_g_array(
      GList * glist,
      guint * length)
{
   int len = 0;
   gpointer * result = NULL;
   guint ndx = 0;
   GList * lptr;

   len = g_list_length(glist);
   result = g_new(gpointer, len+1);
   for (lptr = glist; lptr!=NULL; lptr=lptr->next) {
      result[ndx++] = lptr->data;
   }
   result[ndx] = NULL;

   *length = len;
   return result;
}


// GCompareFunc functions
// signature:
//    gint
//    (* GCompareFunc) (
//      gconstpointer a,
//      gconstpointer b
//    )

/** String comparison function with signature GCompareFunc
 *
 * @param a pointer to first string
 * @param b pointer to second string
 * @return -1, 0, +1 in the usual way
 *
 * @remark Used by g_ptr_array_sort()
 */
gint
gaux_ptr_scomp(
      gconstpointer a,
      gconstpointer b)
{
   char ** ap = (char **) a;
   char ** bp = (char **) b;
   // printf("(%s) ap = %p -> -> %p -> |%s|\n", __func__, ap, *ap, *ap);
   // printf("(%s) bp = %p -> -> %p -> |%s|\n", __func__, bp, *bp, *bp);
   return g_ascii_strcasecmp(*ap,*bp);
}


/** Integer comparison function with signature GCompareFunc
 */
gint
gaux_ptr_intcomp(
      gconstpointer a,
      gconstpointer b)
{
   int ia = GPOINTER_TO_INT(a);
   int ib = GPOINTER_TO_INT(b);
   gint result = 0;
   if (ia < ib)
      result = -1;
   else if (ia > ib)
      result = 1;

   // printf("(%s) a=%p, ia=%d, b=%p, ib=%d, returning %d\n", __func__, a, ia, b, ib, result);
   return result;
}


GPtrArray *
gaux_ptr_array_truncate(
      GPtrArray * gpa,
      int         limit)
{
   assert(gpa);
   bool debug = false;
   if (debug)
      printf("(%s) Starting.  gpa->len=%d, limit=%d\n", __func__, gpa->len, limit);
   if (limit > 0) {
      int removect = gpa->len - limit;
      if (removect > 0) {
         g_ptr_array_remove_range(gpa, limit, removect);
      }
   }
   else if (limit < 0) {
      int removect = gpa->len + limit;
      if (removect > 0) {
         g_ptr_array_remove_range(gpa, 0, removect);
      }
   }
   if (debug)
      printf("(%s) Done.  gpa->len=%d\n", __func__, gpa->len);
   return gpa;
}


// Future:


GPtrArray *
gaux_ptr_array_append_array(
      GPtrArray * dest,
      GPtrArray * src,
      GAuxDupFunc dup_func)
{
   assert(dest);
   if (src) {
      for (guint ndx = 0; ndx < src->len; ndx++) {
         gpointer v = g_ptr_array_index(src,ndx);
         if (dup_func)
            v = dup_func(v);
         g_ptr_array_add(dest, v);
      }
   }
   return dest;
}

GPtrArray *
gaux_ptr_array_join(
      GPtrArray *    gpa1,
      GPtrArray *    gpa2,
      GAuxDupFunc    dup_func,
      GDestroyNotify element_free_func)
{
   int new_len = gpa1->len + gpa2->len;
   GPtrArray * dest = g_ptr_array_sized_new(new_len);
   if (element_free_func)
      g_ptr_array_set_free_func(dest,element_free_func);
   for (guint ndx = 0; ndx < gpa1->len; ndx++) {
      gpointer v = g_ptr_array_index(gpa1,ndx);
      if (dup_func)
         v = dup_func(v);
      g_ptr_array_add(dest, v);
   }
   for (guint ndx = 0; ndx < gpa2->len; ndx++) {
      gpointer v = g_ptr_array_index(gpa2,ndx);
      if (dup_func)
         v = dup_func(v);
      g_ptr_array_add(dest, v);
   }
   return dest;
}

GPtrArray *
gaux_ptr_array_copy(
      GPtrArray *    src,
      GAuxDupFunc    dup_func,
      GDestroyNotify element_free_func)
{
   GPtrArray * dest = g_ptr_array_sized_new(src->len);
   if (element_free_func)
      g_ptr_array_set_free_func(dest, element_free_func);
   for (guint ndx = 0; ndx < src->len; ndx++) {
      gpointer v = g_ptr_array_index(src,ndx);
      if (dup_func)
         v = dup_func(v);
      g_ptr_array_add(dest, v);
   }
   return dest;
}


GPtrArray *
gaux_ptr_array_from_null_terminated_array(
      gpointer *     src,
      GAuxDupFunc    dup_func,
      GDestroyNotify element_free_func)
{
   GPtrArray * result = g_ptr_array_new();
   if (dup_func)
      g_ptr_array_set_free_func(result, element_free_func);
   gpointer* p = src;
   while (*p) {
      gpointer v = (dup_func) ? dup_func(*p) : *p;
      g_ptr_array_add(result, v);
   }
   return result;
}


// GEqualFunc

gboolean gaux_streq(gconstpointer a, gconstpointer b) {
   gboolean result = streq((const char *) a, (const char *) b);
   // printf("(%s) a=|%s|, b=|%s|, returning %s\n", __func__, (const char * )a, (const char *) b, sbool(result));
   return result;
}


/** Implements g_ptr_array_find_with_equal_func(), which requires glib 2.54.
 *
 */
gboolean
gaux_ptr_array_find_with_equal_func(
      GPtrArray *    haystack,
      gconstpointer  needle,
      GEqualFunc     equal_func,
      guint *        index_loc)
{
   bool result = false;
   if (index_loc)
      *index_loc = -1;
   if (haystack && (haystack->len > 0) && needle) {
      for (guint ndx = 0; ndx < haystack->len; ndx++) {
         if (equal_func)
            result = equal_func(g_ptr_array_index(haystack,ndx), needle);
         else
            result = g_ptr_array_index(haystack,ndx) == needle;
         if (result) {
            if (index_loc)
               *index_loc = ndx;
            break;
         }
      }
   }
   return result;
}


//
// Thread utilities
//

/** Handles the boilerplate of obtaining a thread specific buffer that can
 *  change size.
 *
 *  If parm **bufsz_key_ptr** is NULL, the buffer is reallocated with the
 *  specified size with each call to this function.
 *
 *  If parm **bufsz_key_ptr** is non-NULL, then the buffer is reallocated
 *  only if the requested size is larger than the current size.  That is,
 *  the buffer can grow in size but never shrink.
 *
 *  \param  buf_key_ptr    address of a **GPrivate** used as the identifier
 *                         for the buffer
 *  \param  bufsz_key_ptr  address of **GPrivate** used as an identifier for
 *                         the current buffer size
 *  \param  required_size  size of buffer to allocate
 *  \return pointer to thread specific buffer
 */
void *
get_thread_dynamic_buffer(
      GPrivate * buf_key_ptr,
      GPrivate * bufsz_key_ptr,
      guint16    required_size)
{
   // printf("(%s) buf_key_ptr=%p, bufsz_key_ptr=%p, required_size=%d\n",
   //        __func__, buf_key_ptr, bufsz_key_ptr, required_size);

   char * buf       = g_private_get(buf_key_ptr);
   int * bufsz_ptr  = NULL;
   if (bufsz_key_ptr)
      bufsz_ptr = g_private_get(bufsz_key_ptr);

   // GThread * this_thread = g_thread_self();
   // printf("(%s) this_thread=%p, buf=%p, bufsz_ptr=%p\n", __func__, this_thread, buf, bufsz_ptr);
   // if (bufsz_ptr)
   //    printf("(%s) *bufsz_ptr = %d\n", __func__, *bufsz_ptr);


   // unnecessary if use g_private_replace() instead of g_private_set()
   // if (buf)
   //    g_free(buf);

   if ( !bufsz_ptr || *bufsz_ptr < required_size) {
      buf = g_new(char, required_size);
      // printf("(%s) Calling g_private_set()\n", __func__);
      g_private_replace(buf_key_ptr, buf);

      if (bufsz_key_ptr) {
         if (!bufsz_ptr) {
            bufsz_ptr = g_new(int, 1);
            g_private_set(bufsz_key_ptr, bufsz_ptr);
         }
         *bufsz_ptr = required_size;
      }
   }

   // printf("(%s) Returning: %p\n", __func__, buf);
   return buf;
}


/** Handles the boilerplate of obtaining a thread specific fixed size buffer.
 *  The first call to this function in a thread with a given key address
 *  allocates the buffer.  Subsequent calls in the thread for the same key
 *  address return the same buffer.
 *
 *  \param  buf_key_ptr  address of a **GPrivate** used as the identifier
 *                       for the buffer
 *  \param  buffer_size  size of buffer to allocate
 *  \return pointer to thread specific buffer
 *
 *  \remark
 *  When the buffer is first allocated, all bytes are set to 0.
 */
void *
get_thread_fixed_buffer(
      GPrivate * buf_key_ptr,
      guint16    buffer_size)
{
   // printf("(%s) buf_key_ptr=%p, buffer_size=%d\n", __func__, buf_key_ptr, buffer_size);
   assert(buffer_size > 0);

   char * buf = g_private_get(buf_key_ptr);

   // GThread * this_thread = g_thread_self();
   // printf("(%s) this_thread=%p, buf=%p\n", __func__, this_thread, buf);

   if (!buf) {
      buf = g_new0(char, buffer_size);   //g_new0 initializes to 0
      g_private_set(buf_key_ptr, buf);
   }

   // printf("(%s) Returning: %p\n", __func__, buf);
   return buf;
}


