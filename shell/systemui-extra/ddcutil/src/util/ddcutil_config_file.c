/** \file ddcutil_config_file.c
 *  Processes an INI file used for ddcutil options
 *
 *  This is not a generic utility file, but is included in
 *  the util directory to simplify its copying unmodified into
 *  the ddcui source tree.
 */

// Copyright (C) 2021-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>

#include "debug_util.h"
#include "simple_ini_file.h"
#include "string_util.h"
#include "xdg_util.h"

#include "ddcutil_config_file.h"


/** Tokenize a string as per the command line
 *
 *  \param  string to tokenize
 *  \param  tokens_loc where to return the address of a null-terminated list of tokens
 *  \return number of tokens
 *
 *  \remark
 *  The caller is responsible for freeing the list of tokens
 */
int tokenize_options_line(char * string, char ***tokens_loc) {
   bool debug = false;
   if (debug)
      printf("(%s) string -> |%s|\n", __func__, string);
   wordexp_t p;
   int flags = WRDE_NOCMD;
   if (debug)
      flags |= WRDE_SHOWERR;
   int rc = wordexp(string, &p, flags);
   if (debug)
      printf("(%s) wordexp returned %d\n", __func__, rc);
   *tokens_loc = p.we_wordv;
   if (debug) {
      printf("(%s) Tokens:\n", __func__);
      ntsa_show(*tokens_loc);
      printf("(%s) Returning: %zd\n", __func__, p.we_wordc);
   }
   return p.we_wordc;
}


/** Processes a ddcutil configuration file, returning an options string obtained
 *  both the global and applictation-specific sections of the configuration file.nd
 *
 *  \param  ddcutil_application     "ddcutil", "libddcutil", "ddcui"
 *  \param  config_filename_loc     where to return fully qualified name of configuration file
 *  \param  untokenized_option_string_loc
 *                                  where to return untokenized string of options obtained from
 *                                  the configuration file
 *  \param  errmsgs                 collects error messages if non-NULL
 *  \param  verbose                 issue error messages if true
 *  \retval 0                       success
 *  \retval -ENOENT                 config file not found
 *  \retval < 0                     other error
 *
 *  An untokenized option string is returned iff rc == 0.
 */
int read_ddcutil_config_file(
      const char *   ddcutil_application,
      char **        config_fn_loc,
      char **        untokenized_option_string_loc,
      GPtrArray *    errmsgs,
      bool           verbose)
{
   bool debug = false;
   if (debug)
      verbose = true;
   if (debug)
      printf("(%s) Starting. ddcutil_application=%s, errmsgs=%p, verbose=%s\n",
             __func__, ddcutil_application, (void*)errmsgs, SBOOL(verbose));

   int result = 0;
   *untokenized_option_string_loc = NULL;
   *config_fn_loc = NULL;

   char * config_fn = find_xdg_config_file("ddcutil", "ddcutilrc");
   if (!config_fn) {
      if (debug)
         printf("(%s) Configuration file not found\n", __func__);
      result = -ENOENT;
      goto bye;
   }
   if (debug)
      printf("(%s) Found configuration file: %s\n", __func__, config_fn);
   *config_fn_loc = config_fn;

   Parsed_Ini_File * ini_file = NULL;
   int load_rc = ini_file_load(config_fn, errmsgs, verbose, &ini_file);
   ASSERT_IFF(load_rc==0, ini_file);
   if (debug)
      fprintf(stderr, "ini_file_load() returned %d\n", load_rc);
   if (verbose) {
      if (errmsgs && errmsgs->len > 0) {
         fprintf(stderr, "Error(s) processing configuration file: %s\n", config_fn);
         for (guint ndx = 0; ndx < errmsgs->len; ndx++) {
            fprintf(stderr, "   %s\n", (char*) g_ptr_array_index(errmsgs, ndx));
         }
      }
   }

   if (load_rc == 0) {
      if (debug) {
         ini_file_dump(ini_file);
      }
      char * global_options  = ini_file_get_value(ini_file, "global",  "options");
      char * ddcutil_options = ini_file_get_value(ini_file, ddcutil_application, "options");

      char * s = g_strdup_printf("%s %s", (global_options)  ? global_options  : "",
                                          (ddcutil_options) ? ddcutil_options : "");
      char * combined_options = strtrim(s);
      free(s);

      if (debug)
         printf("(%s) combined_options= |%s|\n", __func__, combined_options);

      *untokenized_option_string_loc = combined_options;
      ini_file_free(ini_file);
   }
   else
      result = load_rc;

bye:
   ASSERT_IFF(result==0, *untokenized_option_string_loc);
   if (debug)  {
      // check for null to avoid -Wstringop-overflow
      printf("(%s) Done.     untokenized options: |%s|, *config_fn_loc=%s, returning: %d\n",
             __func__,
             (*untokenized_option_string_loc) ? *untokenized_option_string_loc : "(null)",
             (*config_fn_loc)                 ? *config_fn_loc :                 "(null)",
             result);
   }
   return result;
}


/** Merges the tokenized command string passed to the program with tokens
 *  obtained from the configuration file.
 *
 *  \param   old_argc        original argument count
 *  \param   old_argv        original argument list
 *  \param   config_token_ct number of tokens to insert
 *  \param   config_tokens   list of tokens
 *  \param   merged_argv_loc where to return address of merged argument list
 *  \return  length of merged argument list
 *
 *  \remark
 *  old_argc/old_argv are argc/argv as passed on the command line
 */
static
int merge_command_tokens(
      int      old_argc,
      char **  old_argv,
      int      config_token_ct,
      char **  config_tokens,
      char *** merged_argv_loc)
{
   bool debug = false;
   DBGF(debug, "Starting. old_argc=%d, old_argc=%p, config_token_ct=%d, config_tokens=%p, merged_argv_loc=%p",
               old_argc, old_argv, config_token_ct, config_tokens, merged_argv_loc);

   *merged_argv_loc = NULL;
   int merged_argc  = 0;

   if (config_token_ct > 0) {
      int new_ct = config_token_ct + old_argc + 1;
      DBGF(debug, "config_token_ct = %d, argc=%d, new_ct=%d", config_token_ct, old_argc, new_ct);
      char ** combined = calloc(new_ct, sizeof(char *));
      DBGF(debug, "Allocated combined=%p", combined);
      combined[0] = strdup(old_argv[0]);   // command
      if (debug)
         printf("%s) Allocated combined[0] = %p -> |%s|\n", __func__, combined[0], combined[0] );
      DBGF(debug, "Copying config_token_ct=%d config file tokens", config_token_ct);
      int new_ndx = 1;
      for (int prefix_ndx = 0; prefix_ndx < config_token_ct; prefix_ndx++, new_ndx++) {
         combined[new_ndx] = strdup(config_tokens[prefix_ndx]);
         if(debug)
            printf("(%s) Allocated combined[%d]=%p -> |%s|\n",
                   __func__, new_ndx, combined[new_ndx], combined[new_ndx]);
      }
      DBGF(debug, "Copying %d arguments from old_argv", old_argc-1);
      for (int old_ndx = 1; old_ndx < old_argc; old_ndx++, new_ndx++) {
         combined[new_ndx] = strdup(old_argv[old_ndx]);
         if (debug) printf("(%s) Allocated combined[%d] = %p -> |%s|\n",
               __func__, new_ndx, combined[new_ndx], combined[new_ndx]);
      }
      combined[new_ndx] = NULL;
      if (debug)
         printf("(%s) Final new_ndx = %d\n", __func__, new_ndx);
      *merged_argv_loc = combined;
      merged_argc      = new_ct - 1;
      assert(merged_argc == ntsa_length(combined));
   }
   else {
      *merged_argv_loc = ntsa_copy(old_argv, true);
      merged_argc      = old_argc;
   }

   if (debug) {
       printf("(%s) Returning %d, *merged_argv_loc=%p\n",
              __func__, merged_argc, (void*)*merged_argv_loc);
       printf("(%s) *merged_arv_loc tokens:\n", __func__);
       ntsa_show(*merged_argv_loc);
   }

   return merged_argc;
}


/** Reads and tokenizes the appropriate options entries in the config file,
 *  then combines the tokenized options from the ddcutil configuration file
 *  with the command line arguments, returning a new argument list
 *
 *  \param  application_name  for selecting config file segment
 *  \param  old_argc          argc as passed on the command line
 *  \param  old_argv          argv as passed on the command line
 *  \param  new_argc_loc      where to return length of updated argv,
 *                            contains old_argc if error
 *  \param  new_argv_loc      where to return the address of the updated argv
 *                            as a Null_Terminated_String_Array, contains old_argv if error
 *  \param  untokenized_config_options_loc
 *                            where to return untokenized option string obtained from ini file
 *  \param  configure_fn_loc  where to return name of configuration file,
 *                            NULL if not found
 *  \param  errmsgs           collects error messages, if non-NULL
 *  \retval 0                 success.
 *  \retval < 0               error reading or parsing the configuration file.
 *                            n. it is not an error if the configuration file does not exist.
 */
int apply_config_file(
      const char * application_name,     // "ddcutil", "ddcui", "libddcutil"
      int          old_argc,
      char **      old_argv,
      int *        new_argc_loc,
      char ***     new_argv_loc,
      char**       untokenized_config_options_loc,
      char**       configure_fn_loc,
      GPtrArray *  errmsgs)
{
   bool debug = false;
   DBGF(debug, "Starting. application_name=%s, errmsgs=%p", application_name, (void*)errmsgs);

   *untokenized_config_options_loc = NULL;
   *configure_fn_loc = NULL;
   *new_argv_loc = NULL;
   *new_argc_loc = 0;
   int result = 0;

   int read_config_rc = read_ddcutil_config_file(
                           application_name,
                           configure_fn_loc,
                           untokenized_config_options_loc,
                           errmsgs,
                           debug);   // verbose
   ASSERT_IFF(read_config_rc==0, *untokenized_config_options_loc);
   DBGF(debug, "read_ddcutil_config_file() returned %d, configure_fn: %s",
             read_config_rc, *configure_fn_loc);

   if (read_config_rc == -ENOENT) {
      *new_argv_loc = ntsa_copy(old_argv, true);
      *new_argc_loc = old_argc;
      result = 0;
   }
   else if (read_config_rc < 0) {
      *new_argv_loc = ntsa_copy(old_argv, true);
      *new_argc_loc = old_argc;
      result = read_config_rc;
   }
   else {
      char ** cmd_prefix_tokens = NULL;
      int prefix_token_ct =
            tokenize_options_line(*untokenized_config_options_loc, &cmd_prefix_tokens);

      DBGF(debug, "prefix_token_ct = %d, cmd_prefix_tokens: ", prefix_token_ct);
      if (debug)
         ntsa_show(cmd_prefix_tokens);

      *new_argc_loc =  merge_command_tokens(
                          old_argc,
                          old_argv,
                          prefix_token_ct,
                          cmd_prefix_tokens,
                          new_argv_loc);
      assert(*new_argc_loc == ntsa_length(*new_argv_loc));

      DBGF(debug, "calling ntsa_free() for cmd_prefix_tokens=%p ...", cmd_prefix_tokens);
      ntsa_free(cmd_prefix_tokens, true);
      // ntsa_free(old_argv, false);  // can't free system's argv
   }

   if (debug) {
       DBGF(debug, "Done.     *new_argc_loc=%d, *new_argv_loc=%p, returning %d",
                    *new_argc_loc, *new_argv_loc, result);
       printf("(%s) *new_argv_loc tokens:\n", __func__);
       ntsa_show(*new_argv_loc);
       printf("(%s) *untokenized_config_options_loc=%p->|%s|\n",
              __func__, *untokenized_config_options_loc, *untokenized_config_options_loc);
   }

   return result;
}

