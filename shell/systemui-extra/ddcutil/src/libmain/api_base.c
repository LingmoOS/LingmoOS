/** \file api_base.c
 *
 *  C API base functions.
 */

// Copyright (C) 2015-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#include <assert.h>
#include <base/base_services.h>
#include <errno.h>
#include <glib-2.0/glib.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>

#include "public/ddcutil_c_api.h"

#include "util/ddcutil_config_file.h"
#include "util/debug_util.h"
#include "util/file_util.h"
#include "util/report_util.h"
#include "util/sysfs_filter_functions.h"
#include "util/xdg_util.h"

#include "base/build_info.h"
#include "base/core.h"
#include "base/core_per_thread_settings.h"
#include "base/parms.h"
#include "base/per_thread_data.h"
#include "base/rtti.h"
#include "base/thread_retry_data.h"
#include "base/thread_sleep_data.h"
#include "base/tuned_sleep.h"

#include "cmdline/cmd_parser.h"
#include "cmdline/parsed_cmd.h"

#include "i2c/i2c_bus_core.h"   // for testing watch_devices
#include "i2c/i2c_execute.h"    // for i2c_set_addr()

#include "ddc/ddc_common_init.h"
#include "ddc/ddc_display_lock.h"
#include "ddc/ddc_displays.h"
#include "ddc/ddc_multi_part_io.h"
#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_services.h"
#include "ddc/ddc_try_stats.h"
#include "ddc/ddc_vcp.h"
#include "ddc/ddc_watch_displays.h"

#include "libmain/api_error_info_internal.h"
#include "libmain/api_base_internal.h"
#include "libmain/api_services_internal.h"

//
// Forward Declarations
//

void init_api_base();


//
// Precondition Failure
//

DDCA_Api_Precondition_Failure_Mode api_failure_mode = DDCA_PRECOND_STDERR_RETURN;

DDCA_Api_Precondition_Failure_Mode
ddca_set_precondition_failure_mode(
      DDCA_Api_Precondition_Failure_Mode failure_mode)
{
   DDCA_Api_Precondition_Failure_Mode old = api_failure_mode;
   api_failure_mode = failure_mode;
   return old;
}

DDCA_Api_Precondition_Failure_Mode
ddca_get_precondition_failure_mode()
{
   return api_failure_mode;
}


//
// Library Build Information
//

DDCA_Ddcutil_Version_Spec
ddca_ddcutil_version(void) {
   static DDCA_Ddcutil_Version_Spec vspec = {255,255,255};
   static bool vspec_init = false;

   if (!vspec_init) {
#ifndef NDEBUG
      int ct =
#endif
            sscanf(get_base_ddcutil_version(),
                      "%hhu.%hhu.%hhu", &vspec.major, &vspec.minor, &vspec.micro);
      assert(ct == 3);
      vspec_init = true;
   }
   DBGMSG("Returning: %d.%d.%d", vspec.major, vspec.minor, vspec.micro);
   return vspec;
}


/**  Returns the ddcutil version as a string in the form "major.minor.micro".
 *
 */
const char *
ddca_ddcutil_version_string(void) {
   return get_base_ddcutil_version();
}


// Returns the full ddcutil version as a string that may be suffixed with an extension
const char *
ddca_ddcutil_extended_version_string(void) {
   return get_full_ddcutil_version();
}


// Indicates whether the ddcutil library was built with support for USB connected monitors. .
bool
ddca_built_with_usb(void) {
#ifdef USE_USB
   return true;
#else
   return false;
#endif
}


// Alternative to individual ddca_built_with...() functions.
// conciseness vs documentatbility
// how to document bits?   should doxygen doc be in header instead?

DDCA_Build_Option_Flags
ddca_build_options(void) {
   uint8_t result = 0x00;
#ifdef USE_USB
         result |= DDCA_BUILT_WITH_USB;
#endif
#ifdef FAILSIM_ENABLED
         result |= DDCA_BUILT_WITH_FAILSIM;
#endif
   // DBGMSG("Returning 0x%02x", result);
   return result;
}

#ifdef FUTURE
char * get_library_filename() {
   intmax_t pid = get_process_id();
   return NULL;
}
#endif



//
// Initialization
//

static
Parsed_Cmd * get_parsed_libmain_config() {
   bool debug = false;
   DBGF(debug, "Starting");

   Parsed_Cmd * parsed_cmd = NULL;

   // dummy initial argument list so libddcutil is not a special case
   Null_Terminated_String_Array cmd_name_array = calloc(2, sizeof(char*));
   cmd_name_array[0] = "libddcutil";
   cmd_name_array[1] = NULL;
   DBGF(debug, "cmd_name_array=%p, cmd_name_array[1]=%p -> %s",
                cmd_name_array, cmd_name_array[0], cmd_name_array[0]);

   GPtrArray* errmsgs = g_ptr_array_new_with_free_func(g_free);
   char ** new_argv = NULL;
   int     new_argc = 0;
   char *  untokenized_option_string = NULL;
   char *  config_fn;
   DBGF(debug, "Calling apply_config_file()...");
   int apply_config_rc = apply_config_file(
                                 "libddcutil",  // use this section of config file
                                 1, cmd_name_array,
                                 &new_argc,
                                 &new_argv,
                                 &untokenized_option_string,
                                 &config_fn,
                                 errmsgs);
   // DBGF(debug, "Calling ntsa_free(cmd_name_array=%p", cmd_name_array);
   // ntsa_free(cmd_name_array, false);
   DBGF(debug, "apply_config_file() returned: %d, new_argc=%d, new_argv=%p: ",
                 apply_config_rc, new_argc, new_argv);
   assert(apply_config_rc <= 0);
   assert( new_argc == ntsa_length(new_argv) );
   if (debug)
      ntsa_show(new_argv);

   if (errmsgs->len > 0) {
      f0printf(ferr(),    "Error(s) reading libddcutil configuration from file %s:\n", config_fn);
      SYSLOG(LOG_WARNING, "Error(s) reading libddcutil configuration from file %s:",   config_fn);
      for (int ndx = 0; ndx < errmsgs->len; ndx++) {
         f0printf(fout(),     "   %s\n", (char*) g_ptr_array_index(errmsgs, ndx));
         SYSLOG(LOG_WARNING,  "   %s",   (char*) g_ptr_array_index(errmsgs, ndx));
      }
   }
   g_ptr_array_free(errmsgs, true);
   if (untokenized_option_string && strlen(untokenized_option_string) > 0) {
      fprintf(fout(), "Applying libddcutil options from %s: %s\n", config_fn, untokenized_option_string);
      SYSLOG(LOG_INFO,"Applying libddcutil options from %s: %s",   config_fn, untokenized_option_string);
   }

   // Continue even if config file errors
   // if (apply_config_rc < 0)
   //    goto bye;


   assert(new_argc >= 1);
   DBGF(debug, "Calling parse_command()");
   parsed_cmd = parse_command(new_argc, new_argv, MODE_LIBDDCUTIL);
   if (!parsed_cmd) {
      SYSLOG(LOG_WARNING, "Ignoring invalid configuration file option string: %s",  untokenized_option_string);
      fprintf(ferr(),     "Ignoring invalid configuration file option string: %s\n",untokenized_option_string);
      DBGF(debug, "Retrying parse_command() with no options");
      parsed_cmd = parse_command(1, cmd_name_array, MODE_LIBDDCUTIL);
   }
   if (debug)
      dbgrpt_parsed_cmd(parsed_cmd, 1);
   // DBGF(debug, "Calling ntsa_free(cmd_name_array=%p", cmd_name_array);
   ntsa_free(cmd_name_array, false);
   ntsa_free(new_argv, true);
   free(untokenized_option_string);
   free(config_fn);

   DBGF(debug, "Done.     Returning %p", parsed_cmd);
   return parsed_cmd;
}


#ifdef TESTING_CLEANUP
void done() {
   printf("(%s) Starting\n", __func__);
   _ddca_terminate();
   SYSLOG(LOG_INFO, "(%s) executing done()", __func__);
   printf("(%s) Done.\n", __func__);
}

void dummy_sigterm_handler() {
   printf("(%s) Executing. library_initialized = %s\n",
         __func__, SBOOL(library_initialized));
}

void atexit_func() {
   printf("(%s) Executing. library_initalized = %s\n",
         __func__, SBOOL(library_initialized));
}
#endif

static FILE * flog = NULL;

bool library_initialized = false;
DDCA_Stats_Type requested_stats = 0;
bool per_thread_stats = false;

/** Initializes the ddcutil library module.
 *
 *  Normally called automatically when the shared library is loaded.
 *
 *  It is not an error if this function is called more than once.
 */
void __attribute__ ((constructor))
_ddca_init(void) {
   bool debug = false;
   char * s = getenv("DDCUTIL_DEBUG_LIBINIT");
   if (s && strlen(s) > 0)
      debug = true;

   if (debug)
      printf("(%s) Starting. library_initialized=%s\n", __func__, sbool(library_initialized));
   if (!library_initialized) {
#ifdef ENABLE_SYSLOG
      openlog("libddcutil", LOG_CONS|LOG_PID, LOG_USER);
      syslog(LOG_INFO, "Initializing.  ddcutil version %s", get_full_ddcutil_version());
#endif
#ifdef TESTING_CLEANUP
      // signal(SIGTERM, dummy_sigterm_handler);
      // atexit(atexit_func);  // TESTING CLAEANUP
#endif
      init_api_base();
      init_base_services();
      Parsed_Cmd* parsed_cmd = get_parsed_libmain_config();
      init_tracing(parsed_cmd);

      if (parsed_cmd->library_trace_file) {
         char * trace_file = (parsed_cmd->library_trace_file[0] != '/')
                ? xdg_state_home_file("ddcutil", parsed_cmd->library_trace_file)
                : strdup(parsed_cmd->library_trace_file);
         if (debug)
            printf("(%s) Setting trace destination %s\n", __func__, trace_file);
         SYSLOG(LOG_INFO, "Trace destination: %s", trace_file);

         fopen_mkdir(trace_file, "a", stderr, &flog);

         // flog = fopen(trace_file, "a");
         if (flog) {
            time_t trace_start_time = time(NULL);
            char * trace_start_time_s = asctime(localtime(&trace_start_time));
            if (trace_start_time_s[strlen(trace_start_time_s)-1] == 0x0a)
                 trace_start_time_s[strlen(trace_start_time_s)-1] = 0;
            fprintf(flog, "%s tracing started %s\n", "libddcutil", trace_start_time_s);
            if (debug) {
               fprintf(stdout, "Writing %s trace output to %s\n", "libddcutil",trace_file);
            }
            set_default_thread_output_settings(flog, flog);
            set_fout(flog);
            set_ferr(flog);

            rpt_set_default_output_dest(flog);    // for future threads
            rpt_push_output_dest(flog);           // for this thread
         }
         else {
            fprintf(stderr, "Error opening libddcutil trace file %s: %s\n",
                            trace_file, strerror(errno));
            SYSLOG(LOG_WARNING, "Error opening libddcutil trace file %s: %s",
                            trace_file, strerror(errno));
         }
         free(trace_file);
      }

      requested_stats = parsed_cmd->stats_types;
      per_thread_stats = parsed_cmd->flags & CMD_FLAG_PER_THREAD_STATS;
      init_api_services();
      submaster_initializer(parsed_cmd);
      free_parsed_cmd(parsed_cmd);

     //  explicitly set the async threshold for testing
     //  int threshold = DISPLAY_CHECK_ASYNC_THRESHOLD_STANDARD;
     //  int threshold = DISPLAY_CHECK_ASYNC_NEVER; //
     //  ddc_set_async_threshold(threshold);

      // no longer needed, values are initialized on first use per-thread
      // set_output_level(DDCA_OL_NORMAL);
      // enable_report_ddc_errors(false);

      // dummy_display_change_handler() will issue messages if display is added or removed
      ddc_start_watch_displays();

      library_initialized = true;

#ifdef TESTING_CLEANUP
      // int atexit_rc = atexit(done);   // TESTING CLEANUP
      // printf("(%s) atexit() returned %d\n", __func__, atexit_rc);
#endif

      DBGTRC_DONE(debug, DDCA_TRC_API, "library initialization executed");
      SYSLOG(LOG_INFO, "Library initialization complete.");
   }
   else {
      DBGTRC_DONE (debug, DDCA_TRC_API, "library was already initialized");
      SYSLOG(LOG_INFO, "Library was already initialized.");
   }
   // TRACED_ASSERT(1==5); for testing

   // Not really necessary, but seeing errno == 2 has perplexed some library users
   errno = 0;
}


/** Cleanup at library termination
 *
 *  - Terminates thread that watches for display addition or removal.
 *  - Releases heap memory to avoid error reports from memory analyzers.
 */
void __attribute__ ((destructor))
_ddca_terminate(void) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_API, "library_initialized = %s", SBOOL(library_initialized));
   if (library_initialized) {
      if (debug)
         dbgrpt_distinct_display_descriptors(2);
      ddc_discard_detected_displays();
      if (requested_stats)
         ddc_report_stats_main(requested_stats, per_thread_stats, 0);
      release_base_services();
      ddc_stop_watch_displays();
      free_regex_hash_table();
      library_initialized = false;
      if (flog)
         fclose(flog);
      DBGTRC_DONE(debug, DDCA_TRC_API, "library termination complete");
   }
   else {
      DBGTRC_DONE(debug, DDCA_TRC_API, "library was already terminated");   // should be impossible
   }
   SYSLOG(LOG_INFO, "Terminating.");
   closelog();
}


//
// Error Detail
//

DDCA_Error_Detail *
ddca_get_error_detail() {
   bool debug = false;
   DBGMSF(debug, "Starting");

   DDCA_Error_Detail * result = dup_error_detail(get_thread_error_detail());

   if (debug) {
      DBGMSG("Done.     Returning: %p", result);
      if (result)
         report_error_detail(result, 2);
   }
   return result;
}


void
ddca_free_error_detail(DDCA_Error_Detail * ddca_erec) {
   free_error_detail(ddca_erec);
}


void ddca_report_error_detail(DDCA_Error_Detail * ddca_erec, int depth) {
   report_error_detail(ddca_erec, depth);
}


// DDCA_Error_Detail * ddca_dup_error_detail(DDCA_Error_Detail * original) {
//     return dup_error_detail(original);
// }


//
// Status Code Management
//

const char *
ddca_rc_name(DDCA_Status status_code) {
   char * result = NULL;
   Status_Code_Info * code_info = find_status_code_info(status_code);
   if (code_info)
      result = code_info->name;
   return result;
}


const char *
ddca_rc_desc(DDCA_Status status_code) {
   char * result = "unknown status code";
   Status_Code_Info * code_info = find_status_code_info(status_code);
   if (code_info)
      result = code_info->description;
   return result;
}


// quick and dirty for now
// TODO: make thread safe, wrap in mutex
bool
ddca_enable_error_info(bool enable) {
   bool old_value = report_freed_exceptions;
   report_freed_exceptions = enable;            // global in core.c
   return old_value;
}

//
// Output redirection
//

// Redirects output that normally would go to STDOUT
void
ddca_set_fout(FILE * fout) {
   // DBGMSG("Starting. fout=%p", fout);
   set_fout(fout);
}


void
ddca_set_fout_to_default(void) {
   set_fout_to_default();
}


// Redirects output that normally would go to STDERR
void
ddca_set_ferr(FILE * ferr) {
   set_ferr(ferr);
}


void
ddca_set_ferr_to_default(void) {
   set_ferr_to_default();
}


//
// Output capture - convenience functions
//

typedef struct {
   FILE * in_memory_file;
   char * in_memory_bufstart; ;
   size_t in_memory_bufsize;
   DDCA_Capture_Option_Flags flags;
} In_Memory_File_Desc;


static In_Memory_File_Desc *
get_thread_capture_buf_desc() {
   static GPrivate  in_memory_key = G_PRIVATE_INIT(g_free);

   In_Memory_File_Desc* fdesc = g_private_get(&in_memory_key);

   // GThread * this_thread = g_thread_self();
   // printf("(%s) this_thread=%p, fdesc=%p\n", __func__, this_thread, fdesc);

   if (!fdesc) {
      fdesc = g_new0(In_Memory_File_Desc, 1);
      g_private_set(&in_memory_key, fdesc);
   }

   // printf("(%s) Returning: %p\n", __func__, fdesc);
   return fdesc;
}


void
ddca_start_capture(DDCA_Capture_Option_Flags flags) {
   In_Memory_File_Desc * fdesc = get_thread_capture_buf_desc();

   if (!fdesc->in_memory_file) {
      fdesc->in_memory_file = open_memstream(&fdesc->in_memory_bufstart, &fdesc->in_memory_bufsize);
      ddca_set_fout(fdesc->in_memory_file);   // n. ddca_set_fout() is thread specific
      fdesc->flags = flags;
      if (flags & DDCA_CAPTURE_STDERR)
         ddca_set_ferr(fdesc->in_memory_file);
   }
   // printf("(%s) Done.\n", __func__);
}


char *
ddca_end_capture(void) {
   In_Memory_File_Desc * fdesc = get_thread_capture_buf_desc();
   // In_Memory_File_Desc * fdesc = &in_memory_file_desc;

   char * result = "\0";
   // printf("(%s) Starting.\n", __func__);
   assert(fdesc->in_memory_file);
   if (fflush(fdesc->in_memory_file) < 0) {
      ddca_set_ferr_to_default();
      SEVEREMSG("flush() failed. errno=%d", errno);
      return strdup(result);
   }
   // n. open_memstream() maintains a null byte at end of buffer, not included in in_memory_bufsize
   result = strdup(fdesc->in_memory_bufstart);
   if (fclose(fdesc->in_memory_file) < 0) {
      ddca_set_ferr_to_default();
      SEVEREMSG("fclose() failed. errno=%d", errno);
      return result;
   }
   // free(fdesc->in_memory_file); // double free, fclose() frees in memory file
   fdesc->in_memory_file = NULL;
   ddca_set_fout_to_default();
   if (fdesc->flags & DDCA_CAPTURE_STDERR)
      ddca_set_ferr_to_default();

   // printf("(%s) Done. result=%p\n", __func__, result);
   return result;
}


#ifdef UNUSED
/** Returns the current size of the in-memory capture buffer.
 *
 *  @return number of characters in current buffer, plus 1 for
 *          terminating null
 *  @retval -1 no capture buffer on current thread
 *
 *  @remark defined and tested but does not appear useful
 */
int ddca_captured_size() {
   // printf("(%s) Starting.\n", __func__);
   In_Memory_File_Desc * fdesc = get_thread_capture_buf_desc();

   int result = -1;
   // n. open_memstream() maintains a null byte at end of buffer, not included in in_memory_bufsize
   if (fdesc->in_memory_file) {
      fflush(fdesc->in_memory_file);
      result = fdesc->in_memory_bufsize + 1;   // +1 for trailing \0
   }
   // printf("(%s) Done. result=%d\n", __func__, result);
   return result;
}
#endif


//
// Message Control
//

DDCA_Output_Level
ddca_get_output_level(void) {
   return get_output_level();
}


DDCA_Output_Level
ddca_set_output_level(DDCA_Output_Level newval) {
     return set_output_level(newval);
}


char *
ddca_output_level_name(DDCA_Output_Level val) {
   return output_level_name(val);
}


bool
ddca_enable_report_ddc_errors(bool onoff) {
   return enable_report_ddc_errors(onoff);
}


bool
ddca_is_report_ddc_errors_enabled(void) {
   return is_report_ddc_errors_enabled();
}


//
// Global Settings
//

int
ddca_max_max_tries(void) {
   return MAX_MAX_TRIES;
}


int
ddca_get_max_tries(DDCA_Retry_Type retry_type) {
   // stats for multi part writes and reads are separate, but the
   // max tries for both are identical
#ifndef NDEBUG
   Retry_Op_Value result3 = try_data_get_maxtries2((Retry_Operation) retry_type);
#endif
   // new way using retry_mgt
   Retry_Op_Value result2 = trd_get_thread_max_tries((Retry_Operation) retry_type);
   // assert(result == result2);
   assert(result2 == result3);
   return result2;
}


DDCA_Status
ddca_set_max_tries(
      DDCA_Retry_Type retry_type,
      int             max_tries)
{
   DDCA_Status rc = 0;
   free_thread_error_detail();
   if (max_tries < 1 || max_tries > MAX_MAX_TRIES)
      rc = DDCRC_ARG;
   else {
      try_data_set_maxtries2((Retry_Operation) retry_type, max_tries);
      // for DDCA_MULTI_PART_TRIES, set both  MULTI_PART_WRITE_OP and MULTI_PART_READ_OP
      if (retry_type == DDCA_MULTI_PART_TRIES)
         try_data_set_maxtries2(MULTI_PART_WRITE_OP, max_tries);

      // new way, set in retry_mgt
      trd_set_thread_max_tries((Retry_Operation) retry_type, max_tries);
      if (retry_type == DDCA_MULTI_PART_TRIES)
           trd_set_thread_max_tries(MULTI_PART_WRITE_OP, max_tries);
   }
   return rc;
}


bool
ddca_enable_verify(bool onoff) {
   return ddc_set_verify_setvcp(onoff);
}


bool
ddca_is_verify_enabled() {
   return ddc_get_verify_setvcp();
}

#ifdef NOT_NEEDED
void ddca_lock_default_sleep_multiplier() {
   lock_default_sleep_multiplier();
}

void ddca_unlock_sleep_multiplier() {
   unlock_default_sleep_multiplier();
}
#endif


// deprecated, now a NOOP
bool
ddca_enable_sleep_suppression(bool newval) {
   return false;
}

// deprecated, now a NOOP
bool
ddca_is_sleep_suppression_enabled() {
   return false;
}


double
ddca_set_default_sleep_multiplier(double multiplier)
{
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_API, "Setting multiplier = %6.3f", multiplier);

   double old_value = -1.0;
   if (multiplier >= 0.0 && multiplier <= 10.0) {
      old_value = tsd_get_default_sleep_multiplier_factor();
      tsd_set_default_sleep_multiplier_factor(multiplier);
    }

   DBGTRC_DONE(debug, DDCA_TRC_API, "Returning: %6.3f", old_value);
   return old_value;
}


double
ddca_get_default_sleep_multiplier()
{
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_API, "");
   double result = tsd_get_default_sleep_multiplier_factor();
   DBGTRC(debug, DDCA_TRC_API, "Returning %6.3f", result);
   return result;
}

void
ddca_set_global_sleep_multiplier(double multiplier)
{
   ddca_set_default_sleep_multiplier(multiplier);
   return;
}

double
ddca_get_global_sleep_multiplier()
{
   return ddca_get_default_sleep_multiplier();
}

// for current thread
double
ddca_set_sleep_multiplier(double multiplier)
{
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_API, "Setting multiplier = %6.3f", multiplier);

   double old_value = -1.0;
   if (multiplier >= 0.0 && multiplier <= 10.0) {
      old_value = tsd_get_sleep_multiplier_factor();
      tsd_set_sleep_multiplier_factor(multiplier);
   }

   DBGTRC_DONE(debug, DDCA_TRC_API, "Returning: %6.3f", old_value);
   return old_value;
}

double
ddca_get_sleep_multiplier()
{
   bool debug = false;
   DBGTRC(debug, DDCA_TRC_API, "");
   double result = tsd_get_sleep_multiplier_factor();
   DBGTRC(debug, DDCA_TRC_API, "Returning %6.3f", result);
   return result;
}


#ifdef FUTURE

/** Gets the I2C timeout in milliseconds for the specified timeout class.
 * @param timeout_type timeout type
 * @return timeout in milliseconds
 */
int
ddca_get_timeout_millis(
      DDCA_Timeout_Type timeout_type) {
   return 0;    // *** UNIMPLEMENTED ***
}

/** Sets the I2C timeout in milliseconds for the specified timeout class
 * @param timeout_type  timeout class
 * @param millisec      timeout to set, in milliseconds
 */
void
ddca_set_timeout_millis(
      DDCA_Timeout_Type timeout_type,
      int               millisec)
{
   // *** UNIMPLEMENTED
}
#endif

#ifdef FUTURE
bool
ddca_enable_force_slave_address(bool onoff) {
   return false;
}


bool
ddca_is_force_slave_address_enabled(void) {
   return false;
}
#endif


bool
ddca_enable_force_slave_address(bool onoff) {
   bool old = i2c_forceable_slave_addr_flag;
   i2c_forceable_slave_addr_flag = onoff;
   return old;
}


bool
ddca_is_force_slave_address_enabled(void) {
   return i2c_forceable_slave_addr_flag;
}



//
// Tracing
//

void
ddca_add_traced_function(const char * funcname) {
   add_traced_function(funcname);
}


void
ddca_add_traced_file(const char * filename) {
   add_traced_file(filename);
}


void
ddca_set_trace_groups(DDCA_Trace_Group trace_flags) {
   set_trace_groups(trace_flags);
}


void
ddca_add_trace_groups(DDCA_Trace_Group trace_flags) {
   add_trace_groups(trace_flags);
}


DDCA_Trace_Group
ddca_trace_group_name_to_value(char * name) {
   return trace_class_name_to_value(name);
}


void
ddca_set_trace_options(DDCA_Trace_Options  options) {
   // DBGMSG("options = 0x%02x", options);
   // global variables in core.c

   if (options & DDCA_TRCOPT_TIMESTAMP)
      dbgtrc_show_time = true;
   if (options & DDCA_TRCOPT_WALLTIME)
      dbgtrc_show_time = true;
   if (options & DDCA_TRCOPT_THREAD_ID)
      dbgtrc_show_thread_id = true;
}


//
// Statistics
//

// TODO: Add functions to access ddcutil's runtime error statistics


#ifdef UNUSED
void
ddca_register_thread_dref(DDCA_Display_Ref dref) {
   ptd_register_thread_dref( (Display_Ref *) dref);
}
#endif

void
ddca_set_thread_description(
      const char * description)
{
   ptd_set_thread_description( description );
}

void
ddca_append_thread_description(
      const char * description)
{
   ptd_append_thread_description(description);
}

const char *
ddca_get_thread_descripton() {
   return ptd_get_thread_description_t();
}

void
ddca_reset_stats(void) {
   // DBGMSG("Executing");
   ddc_reset_stats_main();
}

// TODO: Functions that return stats in data structures
void
ddca_show_stats(
      DDCA_Stats_Type stats_types,
      bool            by_thread,
      int             depth)
{
   if (stats_types)
      ddc_report_stats_main( stats_types, by_thread, depth);
}


void init_api_base() {
   RTTI_ADD_FUNC(ddca_set_sleep_multiplier);
   RTTI_ADD_FUNC(ddca_set_default_sleep_multiplier);
}

