/** @file debug_util.c
 *
 * Functions for debugging
 */

// Copyright (C) 2016-2021 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"
/** \cond */
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <glib-2.0/glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef UNUSED
#ifdef TARGET_BSD
#include <pthread_np.h>
#else
#include <sys/types.h>
#include <sys/syscall.h>
#include <syslog.h>
#endif
#endif

/** \endcond */

#include "string_util.h"

#include "debug_util.h"

// HACK
#ifdef TARGET_BSD
#undef HAVE_EXECINFO_H
#endif

#ifdef HAVE_EXECINFO_H
/* Extracts the function name and offset from a backtrace line
 *
 * \param  bt_line   line returned by backtrace()
 * \param  name_only if true, return only the name
 * \return string of form "name+offset" or just "name".
 *         It is the responsibility of the caller to free this string.
 */
static char * extract_function(char * bt_line, bool name_only) {
   bool debug = false;
   if (debug)
      printf("\n(%s) bt_line = |%s|\n", __func__, bt_line);
   char * result = NULL;
   char * start = strchr(bt_line, '(');
   if (!start) {
      result = strdup("???");
   }
   else {
      start++;          // character after paren
      char * end = strchr(start, ')');

      if (!end)
         end = bt_line + strlen(bt_line);
      int len = end - start;
      if (debug) {
         printf("(%s) start=%p, end=%p, len=%d\n", __func__, start, end, len);
         printf("(%s) extract is |%.*s|\n", __func__, len, start);
      }
      result = malloc(len+1);
      memcpy(result, start, len);
      result[len] = '\0';
   }
   if (name_only) {
      char *p = strchr(result, '+');
      if (p)
         *p = '\0';
   }

   if (debug)
      printf("(%s) Returning |%s|\n", __func__, result);
   return result;
}
#endif


#ifdef OLD
/** Show the call stack.
 *
 * @param  stack_adjust  number of initial stack frames to ignore, to hide this
 *                       function and possibly some number of immediate callers
 *
 * @remark
 * Note that the call stack is approximate.  The underlying system function, backtrace()
 * relies on external symbols, so static functions will not be properly shown.
 */
void show_backtrace(int stack_adjust)
{
   bool debug = false;
   if (debug)
      printf("(%s) Starting.  stack_adjust = %d\n", __func__, stack_adjust);

   int j, nptrs;
   const int MAX_ADDRS = 100;
   void *buffer[MAX_ADDRS];
   char **strings;

   nptrs = backtrace(buffer, MAX_ADDRS);
   if (debug)
      printf("(%s) backtrace() returned %d addresses\n", __func__, nptrs);

   strings = backtrace_symbols(buffer, nptrs);
   if (strings == NULL) {
      perror("backtrace_symbols unavailable");
   }
   else {
      printf("Current call stack\n");
      for (j = 0; j < nptrs; j++) {
         if (j < stack_adjust) {
            if (debug)
               printf("(%s) Suppressing %s\n", __func__, strings[j]);
         }
         else {
            // printf("   %s\n", strings[j]);
            char * s = extract_function(strings[j], true);
            printf("   %s\n", s);
            bool final = (streq(s, "main")) ? true : false;
            free(s);
            if (final)
                  break;
         }
      }

      free(strings);
   }
}
#endif

GPtrArray * get_backtrace(int stack_adjust) {
#ifdef HAVE_EXECINFO_H
   bool debug = false;
   if (debug)
      printf("(%s) Starting.  stack_adjust = %d\n", __func__, stack_adjust);

   GPtrArray * result = NULL;
   int j, nptrs;
   const int MAX_ADDRS = 100;
   void *buffer[MAX_ADDRS];
   char **strings;

   nptrs = backtrace(buffer, MAX_ADDRS);
   if (debug)
      printf("(%s) backtrace() returned %d addresses\n", __func__, nptrs);

   strings = backtrace_symbols(buffer, nptrs);
   if (strings == NULL) {
      if (debug)
         perror("backtrace_symbols unavailable");
   }
   else {
      result = g_ptr_array_sized_new(nptrs-stack_adjust);
      if (debug)
         printf("Current call stack\n");
      for (j = 0; j < nptrs; j++) {
         if (j < stack_adjust) {
            if (debug)
               printf("(%s) Suppressing %s\n", __func__, strings[j]);
         }
         else {
            // printf("   %s\n", strings[j]);
            char * s = extract_function(strings[j], true);
            if (debug)
               printf("   %s\n", s);
            g_ptr_array_add(result, s);
            bool final = (streq(s, "main")) ? true : false;
            if (final)
                  break;
         }
      }

      free(strings);
   }
   return result;
#else
   return NULL;
#endif
}

void show_backtrace(int stack_adjust) {
   GPtrArray * callstack = get_backtrace(stack_adjust);
   if (!callstack) {
      perror("backtrace unavailable");
   }
   else {
      printf("Current call stack:\n");
      for (int ndx = 0; ndx < callstack->len; ndx++) {
         printf("   %s\n", (char *) g_ptr_array_index(callstack, ndx));
      }
      g_ptr_array_free(callstack, true);
   }
}


bool simple_dbgmsg(
        bool              debug_flag,
        const char *      funcname,
        const int         lineno,
        const char *      filename,
        char *            format,
        ...)
{
   bool debug_func = false;
   if (debug_func)
      printf("(simple_dbgmsg) Starting. debug_flag=%s, funcname=%s filename=%s, lineno=%d\n",
                       sbool(debug_flag), funcname, filename, lineno);

#ifdef UNUSED
   char thread_prefix[15] = "";
    int tid = pthread_getthreadid_np();
       pid_t pid = syscall(SYS_getpid);
       snprintf(thread_prefix, 15, "[%7jd]", (intmax_t) pid);  // is this proper format for pid_t
#endif

   bool msg_emitted = false;
   if ( debug_flag ) {
      va_list(args);
      va_start(args, format);
      char * buffer = g_strdup_vprintf(format, args);
      va_end(args);

      char * buf2 = g_strdup_printf("(%-32s) %s", funcname, buffer);

      f0puts(buf2, stdout);
      f0putc('\n', stdout);
      fflush(stdout);
      free(buffer);
      free(buf2);
      msg_emitted = true;
   }

   return msg_emitted;
}



