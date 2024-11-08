/** @file thread_sleep_data.c
 *
 *  Struct Thread_Sleep_Data maintains all per-thread sleep data.
 *
 *  This file contains the usual access and report functions, along with
 *  small functions for managing various fields.
 */

// Copyright (C) 2020-2023 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later
 
#include <assert.h>
#include <sys/types.h>

// for syscall
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#include "util/debug_util.h"
#include "util/report_util.h"
#include "util/string_util.h"

#include "base/parms.h"
#include "base/core.h"
#include "base/rtti.h"
#include "base/sleep.h"

#include "base/per_thread_data.h"
#include "base/thread_sleep_data.h"

static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_BASE;

//
// Sleep time adjustment
//

/* Two multipliers are applied to the sleep time determined from the
 * IO mode and event type.
 *
 * sleep_multiplier_factor: set globally, e.g. from arg passed on
 * command line.  Consider making thread specific.
 *
 * sleep_multiplier_ct: Per thread adjustment,initiated by IO retries.
 */

// Defaults for new threads.  Default sleep multiplier factor can be adjusted,
// Default sleep multiplier count cannot.
static       double default_sleep_multiplier_factor = 1.0; // may be changed by --sleep-multiplier option
static const int    default_sleep_multiplier_count  = 1;
static       bool   default_dynamic_sleep_enabled   = false;


//
// Reporting
//

/** Output a report of the sleep data in a #Per_Thread_Data struct in a form
 * intended to be incorporated in program output.
 *
 *  @param  data   pointer to #Per_Thread_Data struct
 *  @param  depth  logical indentation level
 */
void report_thread_sleep_data(Per_Thread_Data * data, int depth) {
   ptd_cross_thread_operation_block();
   int d1 = depth+1;
   int d2 = depth+2;
   rpt_vstring(depth, "Thread %d sleep data:", data->thread_id);
   rpt_label(  d1,   "General:");
   // if (data->dref)
   //    rpt_vstring(d2,    "Display:                           %s", dref_repr_t(data->dref) );
   rpt_vstring(d2,    "Description:                       %s",   (data->description) ? data->description : "Not set");
   rpt_vstring(d2,    "Total sleep time (millis):         %d",   data->total_sleep_time_millis);
   rpt_vstring(d2,    "Current sleep-multiplier factor:  %5.2f", data->sleep_multiplier_factor);
   rpt_vstring(d2,    "Dynamic sleep enabled:             %s",   sbool(data->dynamic_sleep_enabled));

   rpt_label(  d1,    "Sleep multiplier adjustment:");
   rpt_vstring(d2,    "Current adjustment:                %d",   data->sleep_multiplier_ct);
   rpt_vstring(d2,    "Highest adjustment:                %d",   data->highest_sleep_multiplier_ct);
   rpt_label(  d2,    "Number of function calls");
   rpt_vstring(d2,    "   that performed adjustment:      %d",   data->sleep_multipler_changer_ct);

   if ( data->dynamic_sleep_enabled ) {
      rpt_label(  d1, "Dynamic Sleep Adjustment:  ");
      rpt_vstring(d2, "Total successful reads:           %5d",   data->total_ok_status_count);
      rpt_vstring(d2, "Total reads with DDC error:       %5d",   data->total_error_status_count);
      rpt_vstring(d2, "Total ignored status codes:       %5d",   data->total_other_status_ct);
      rpt_vstring(d2, "Current sleep adjustment factor:  %5.2f", data->cur_sleep_adjustment_factor);
//    rpt_vstring(d2, "Thread adjustment increment:      %5.2f", data->thread_adjustment_increment);
      rpt_vstring(d2, "Adjustment check interval         %5d",   data->adjustment_check_interval);

      rpt_vstring(d2, "Calls since last check:           %5d",   data->calls_since_last_check);
      rpt_vstring(d2, "Total adjustment checks:          %5d",   data->total_adjustment_checks);
      rpt_vstring(d2, "Number of adjustments:            %5d",   data->total_adjustment_ct);
//    rpt_vstring(d2, "Number of excess adjustments:     %5d",   data->total_max_adjustment_ct);
      rpt_vstring(d2, "Final sleep adjustment:           %5.2f", data->cur_sleep_adjustment_factor);
   }
}


/** Wraps #report_thread_sleep_data() in a form usable by #ptd_apply_all_sorted()
 *
 *  @param  data pointer to #Per_Thread_Data instance
 *  @param  arg  an integer holding the depth argument
 */
void wrap_report_thread_sleep_data(Per_Thread_Data * data, void * arg) {
   int depth = GPOINTER_TO_INT(arg);
   // rpt_vstring(depth, "Per_Thread_Data:");  // needed?
   report_thread_sleep_data(data, depth);
}


/** Report all #Per_Thread_Data structs.  Note that this report includes
 *  structs for threads that have been closed.
 *
 *  @param depth  logical indentation depth
 */
void report_all_thread_sleep_data(int depth) {
   // int d1 = depth+1;
   bool debug = false;
   DBGMSF(debug, "Starting");
   assert(per_thread_data_hash);
   rpt_label(depth, "Per thread sleep data");
   // For debugging per-thread locks
   // rpt_vstring(d1, "ptd lock count:                  %d", ptd_lock_count);
   // rpt_vstring(d1, "ptd_unlock_count:                %d", ptd_unlock_count);
   // rpt_vstring(d1, "cross thread operations blocked: %d", cross_thread_operation_blocked_count);

   ptd_apply_all_sorted(&wrap_report_thread_sleep_data, GINT_TO_POINTER(depth+1) );
   DBGMSF(debug, "Done");
   rpt_nl();
}


//
// Obtain, initialize, and reset sleep data for current thread
//

Per_Thread_Data * tsd_get_thread_sleep_data() {
   Per_Thread_Data * ptd = ptd_get_per_thread_data();
   // n. init_thread_sleep_data() is called from per-thread data initializer
   assert(ptd->thread_sleep_data_defined);
   return ptd;
}


// initialize a single instance, called from init_per_thread_data()
void tsd_init_thread_sleep_data(Per_Thread_Data * data) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "data=%p", (void*)data);

   data->dynamic_sleep_enabled = default_dynamic_sleep_enabled;
   data->sleep_multiplier_ct = default_sleep_multiplier_count;
   data->highest_sleep_multiplier_ct = 1;

   data->cur_sleep_adjustment_factor = 1.0;
   data->initialized = true;
   data->sleep_multiplier_factor = default_sleep_multiplier_factor;
   // data->thread_adjustment_increment = default_sleep_multiplier_factor;
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
         "Setting data->sleep_multiplier_factor = default_sleep_multiplier_factor = %6.3f",
         default_sleep_multiplier_factor);
   data->adjustment_check_interval = 2;
   data->total_sleep_time_millis = 0;

   data->thread_sleep_data_defined = true;   // vs data->initialized

   DBGTRC_DONE(debug, DDCA_TRC_NONE, "sleep_multiplier_factor = %5.2f", data->sleep_multiplier_factor);
}


#ifdef UNUSED
void reset_thread_sleep_data(Per_Thread_Data * data) {
   ptd_cross_thread_operation_block();
   data->highest_sleep_multiplier_ct = data->sleep_multiplier_ct;
   data->sleep_multipler_changer_ct = 0;
   data->total_ok_status_count = 0;
   data->total_error_status_count = 0;
   data->total_other_status_ct = 0;
   data->total_adjustment_checks = 0;
   data->total_adjustment_ct = 0;
   data->total_max_adjustment_ct = 0;
   data->total_sleep_time_millis = 0;
}


void wrap_reset_thread_sleep_data(Per_Thread_Data * data, void * arg) {
   reset_thread_sleep_data(data);
}
#endif


//
// Operations on all instances
//

#ifdef UNUSED
void reset_all_thread_sleep_data() {
   if (per_thread_data_hash) {
      ptd_apply_all_sorted(&wrap_reset_thread_sleep_data, NULL );
   }
}
#endif


//
// Sleep time adjustment
//

/* Two multipliers are applied to the sleep time determined from the
 * io mode and event type.
 *
 * A default sleep_multiplier_factor: is set globally,
 * e.g. from the --sleep-multiplier option passed on command line.
 * It can be adjusted on a per thread basis..
 *
 * The sleep multiplier count is intended for short-term dynamic
 * adjustment, typically by retry mechanisms within a single operation.
 * It is normally 1.
 */

//
// Sleep Multiplier Factor
//

/** Sets the default sleep multiplier factor, used for the creation of any new threads.
 * This is a global value and is a floating point number.
 *
 *  @param multiplier
 *
 *  @remark Intended for use only during program initialization.  If used
 *          more generally, get and set of default sleep multiplier needs to
 *          be protected by a lock.
 *  @todo
 *  Add Sleep_Event_Type bitfield to make sleep factor dependent on event type?
 */
void tsd_set_default_sleep_multiplier_factor(double multiplier) {
   bool debug = false;
   DBGTRC(debug, DDCA_TRC_NONE,
                    "Executing. Setting default_sleep_multiplier_factor = %6.3f", multiplier);
   assert(multiplier >= 0);
   default_sleep_multiplier_factor = multiplier;
}


/** Gets the default sleep multiplier factor.
 *
 *  @return sleep multiplier factor
 */
double tsd_get_default_sleep_multiplier_factor() {
   bool debug = false;
   DBGTRC(debug, DDCA_TRC_NONE,
          "Returning default_sleep_multiplier_factor = %6.3f", default_sleep_multiplier_factor);
   return default_sleep_multiplier_factor;
}


/** Gets the sleep multiplier factor for the current thread.
 *
 *  @return sleep multiplier factor
 */
double tsd_get_sleep_multiplier_factor() {
   bool debug = false;
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   double result = data->sleep_multiplier_factor;
   DBGTRC(debug, TRACE_GROUP, "Returning %6.3f", result );
   return result;
}


/** Sets the sleep multiplier factor for the current thread.
 *
 *  @param factor  sleep multiplier factor
 */
void tsd_set_sleep_multiplier_factor(double factor) {
   bool debug = false;

   // Need to guard with mutex!

   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "factor = %6.3f", factor);
   assert(factor >= 0);
   ptd_cross_thread_operation_block();
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   data->sleep_multiplier_factor = factor;
   // data->thread_adjustment_increment = factor;
   DBGTRC_DONE(debug, DDCA_TRC_NONE, "");
}


#ifdef UNUSED
default_dynamic_sleep_enabled

void set_global_sleep_multiplier_factor(double factor) {
   bool debug = false;
   DBGMSF(debug, "factor = %5.2f", factor);
   global_sleep_multiplier_factor = factor;
   // set_sleep_multiplier_factor_all(factor);   // only applies to new threads, do not change existing threads
}


double get_global_sleep_multiplier_factor() {
   return global_sleep_multiplier_factor;
}
#endif


//
// Sleep Multiplier Count
//

/** Gets the multiplier count for the current thread.
 *
 *  @return multiplier count
 */
int tsd_get_sleep_multiplier_ct() {
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   return data->sleep_multiplier_ct;
}


/** Sets the multiplier count for the current thread.
 *
 *  @param multipler_ct  value to set
 */
void tsd_set_sleep_multiplier_ct(int multiplier_ct) {
   bool debug = false;
   DBGMSF(debug, "Setting sleep_multiplier_ct = %d for current thread", multiplier_ct);
   assert(multiplier_ct > 0 && multiplier_ct < 100);
   ptd_cross_thread_operation_start();
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   data->sleep_multiplier_ct = multiplier_ct;
   if (multiplier_ct > data->highest_sleep_multiplier_ct)
      data->highest_sleep_multiplier_ct = multiplier_ct;
   ptd_cross_thread_operation_end();
}


/** Increment the number of function executions on this thread
 *  that changed the sleep multiplier count.
 */
void tsd_bump_sleep_multiplier_changer_ct() {
   ptd_cross_thread_operation_block();
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   data->sleep_multipler_changer_ct++;
}


#ifdef UNUSED
// apply the sleep-multiplier to any existing threads
// it will be set for new threads from global_sleep_multiplier_factor
void set_sleep_multiplier_factor_all(double factor) {
   // needs mutex
   bool debug = false;
   DBGMSF(debug, "Starting. factor = %5.2f", factor);
   if (thread_sleep_data_hash) {
      GHashTableIter iter;
      gpointer key, value;
      g_hash_table_iter_init (&iter,thread_sleep_data_hash);
      while (g_hash_table_iter_next (&iter, &key, &value)) {
         Per_Thread_Data * data = value;
         DBGMSF(debug, "Thread id: %d", data->thread_id);
         data->sleep_multiplier_factor = factor;
      }
   }
}
#endif


//
// Dynamic Sleep
//

#ifdef UNUSED
/** Enable or disable dynamic sleep adjustment on the current thread
 *
 *  @param enabled   true/false i.e. enabled, disabled
 */
void tsd_enable_dynamic_sleep(bool enabled) {
   bool debug = false;
   DBGMSF(debug, "enabled = %s", sbool(enabled));
   ptd_cross_thread_operation_start();
   // bool this_function_owns_lock = ptd_lock_if_unlocked();
   Per_Thread_Data * data = tsd_get_thread_sleep_data();
   data->dynamic_sleep_enabled = enabled;
   ptd_cross_thread_operation_end();
   // ptd_unlock_if_needed(this_function_owns_lock);
}
#endif


/** Enable or disable dynamic sleep adjustment on all existing threads
 *
 *  @param enable  true/false
 */
void tsd_enable_dsa_all(bool enable) {
   // needs mutex
   ptd_cross_thread_operation_start();
   bool debug = false;
   DBGMSF(debug, "Starting. enable = %s", sbool(enable) );
   default_dynamic_sleep_enabled = enable;  // for initializing new threads
   if (per_thread_data_hash) {
      GHashTableIter iter;
      gpointer key, value;
      g_hash_table_iter_init (&iter,per_thread_data_hash);
      while (g_hash_table_iter_next (&iter, &key, &value)) {
         Per_Thread_Data * data = value;
         DBGMSF(debug, "Thread id: %d", data->thread_id);
         data->dynamic_sleep_enabled = enable;
      }
   }
   ptd_cross_thread_operation_end();
}


/** Enable or disable dynamic sleep adjustment for the current thread.
 *
 *  @param true/false
 */
void tsd_dsa_enable(bool enabled) {
   ptd_cross_thread_operation_block();
   Per_Thread_Data * tsd = tsd_get_thread_sleep_data();
   tsd->dynamic_sleep_enabled = enabled;
}


/** Enable or disable dynamic sleep adjustment for all current threads and new threads
 *
 *  @param enabled  true/false
 */
void tsd_dsa_enable_globally(bool enabled) {
   bool debug = false;
   DBGMSF(debug, "Executing.  enabled = %s", sbool(enabled));
   ptd_cross_thread_operation_start();
   default_dynamic_sleep_enabled = enabled;
   tsd_enable_dsa_all(enabled) ;
   ptd_cross_thread_operation_end();
}


#ifdef UNUSED
// Is dynamic sleep enabled on the current thread?
bool tsd_dsa_is_enabled() {
   ptd_cross_thread_operation_start();     // needed
   Per_Thread_Data * tsd = tsd_get_thread_sleep_data();
   bool result = tsd->dynamic_sleep_enabled;
   ptd_cross_thread_operation_end();
   return result;
}
#endif

#ifdef UNUSED
void tsd_set_dsa_enabled_default(bool enabled) {
   default_dynamic_sleep_enabled = enabled;
}
#endif


bool tsd_get_dsa_enabled_default() {
   return default_dynamic_sleep_enabled;
}


void init_thread_sleep_data() {
   RTTI_ADD_FUNC(tsd_init_thread_sleep_data);
   RTTI_ADD_FUNC(tsd_get_default_sleep_multiplier_factor);
   RTTI_ADD_FUNC(tsd_set_default_sleep_multiplier_factor);
   RTTI_ADD_FUNC(tsd_get_sleep_multiplier_factor);
   RTTI_ADD_FUNC(tsd_set_sleep_multiplier_factor);
}

