/** @file tuned_sleep.c
 *
 *  Perform sleep. The sleep time is determined by io mode, sleep event time,
 *  and applicable multipliers.
 */

// Copyright (C) 2019-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <assert.h>
#include <sys/types.h>

#include "public/ddcutil_types.h"

#include "base/core.h"
#include "base/dynamic_sleep.h"
#include "base/execution_stats.h"
#include "base/per_thread_data.h"
#include "base/rtti.h"
#include "base/sleep.h"
#include "base/thread_sleep_data.h"

// Trace class for this file
static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_SLEEP;

//
// Deferred sleep
//
// If enabled, sleep is not performed immediately, but instead before the next
// DDC call that requires that a wait has occurred. The elapsed time between
// when the call is requested and when it actually occurs is subtracted from
// the specified sleep time to obtain the actual sleep time.
//
// In testing, this has proven to have a negligible effect on elapsed
// execution time.
//

static bool deferred_sleep_enabled = false;
bool suppress_se_post_read = false;


/** Enables or disables deferred sleep.
 *  @param  onoff new  setting
 *  @return old setting
 */
bool enable_deferred_sleep(bool onoff) {
   // DBGMSG("enable = %s", sbool(onoff));
   bool old = deferred_sleep_enabled;
   deferred_sleep_enabled = onoff;
   return old;
}


/** Reports whether deferred sleep is enabled.
 *  @return true/false
 */
bool is_deferred_sleep_enabled() {
   return deferred_sleep_enabled;
}


/** Given a sleep event type, return its sleep time in milliseconds as per the
 *  DDC/CI spec, and also whether the sleep can be deferred.
 *
 *  @param event_type
 *  @param special_sleep_time_millis sleep time for SE_SPECIAL
 *  @param is_deferrable_loc where to return flag indicating whether the sleep
 *                           can be deferred or must be performed immiediately
 */
static int get_sleep_time(
      Sleep_Event_Type event_type,
      int              special_sleep_time_millis,
      bool*            is_deferrable_loc)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
         "Sleep event type = %s, special_sleep_time_millis=%d",
         sleep_event_name(event_type),  special_sleep_time_millis);

   int spec_sleep_time_millis = 0;
   bool deferrable_sleep = false;

   switch (event_type) {
   // Sleep events with values defined in DDC/CI spec
   case SE_WRITE_TO_READ:
      // 4.3 Get VCP Feature & VCP Feature Reply:
      //     The host should wait at least 40 ms in order to enable the decoding and
      //     and preparation of the reply message by the display
      // 4.6 Capabilities Request & Reply:
      //     write to read interval unclear, assume 50 ms
      // Note: ddc_i2c_write_read_raw() is used for both normal VCP feature reads
      // and reads within a capabilities or table command.  It can't distinguish a
      // normal write/read from one inside a multi part read.  So this sleep time
      // is used for both.
      //  spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_BETWEEN_GETVCP_WRITE_READ;
      spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_DEFAULT;
      // spec_sleep_time_millis = 0; // *** TEMP ***
      deferrable_sleep = deferred_sleep_enabled;
      break;
   case SE_POST_WRITE: // post SET VCP FEATURE write, between SET TABLE write fragments, after final?
      // 4.4 Set VCP Feature:
      //   The host should wait at least 50ms to ensure next message is received by the display
      spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_DEFAULT;
      deferrable_sleep = deferred_sleep_enabled;
      break;
   case SE_POST_READ:
      deferrable_sleep = deferred_sleep_enabled;
      spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_DEFAULT;
      if (suppress_se_post_read) {
         DBGMSG("Suppressing SE_POST_READ");
         spec_sleep_time_millis = 0;
      }
      break;
   case SE_POST_SAVE_SETTINGS:
      // 4.5 Save Current Settings:
      // The host should wait at least 200 ms before sending the next message to the display
      deferrable_sleep = deferred_sleep_enabled;
      spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_POST_SAVE_SETTINGS; // per DDC spec
      break;
   case SE_PRE_MULTI_PART_READ:
      // before reading capabilities - this is based on testing, not defined in spec
      spec_sleep_time_millis = 200;
      break;
   case SE_POST_CAP_TABLE_SEGMENT:
      // 4.6 Capabilities Request & Reply:
      //     The host should wait at least 50ms before sending the next message to the display
      // 4.8.1 Table Write
      //     The host should wait at least 50ms before sending the next message to the display
      // 4.8.2 Table Read
      //     The host should wait at least 50ms before sending the next message to the display
      spec_sleep_time_millis = DDC_TIMEOUT_MILLIS_BETWEEN_CAP_TABLE_FRAGMENTS;
      break;
   case SE_SPECIAL:    // UNUSED
      // 4/2020: no current use
      spec_sleep_time_millis = special_sleep_time_millis;
      break;
   }

   *is_deferrable_loc = deferrable_sleep;
   DBGTRC_DONE(debug, TRACE_GROUP,
         "Returning: %d, *is_deferrable_loc = %s", spec_sleep_time_millis, SBOOL(*is_deferrable_loc));
   return spec_sleep_time_millis;
}


/** Adjusts the sleep time.
 *
 *  First the sleep multiplier is applied.
 *
 *  Then, if dynamic sleep is not enabled (the usual case) the sleep time is
 *  multiplied by the per thread sleep multiplier count. This count is
 *  maintained by the IO retry logic.
 *
 *  If dynamic sleep is enabled, the sleep time is multiplied by the value
 *  returned from dsa_update_adjustment_factor().
 *
 */
static int adjust_sleep_time(Display_Handle * dh, int spec_sleep_time_millis) {
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
                          "dh=%s, spec_sleep_time_millis=%d", dh_repr(dh), spec_sleep_time_millis);

   Per_Thread_Data * tsd = tsd_get_thread_sleep_data();
   int adjusted_sleep_time_millis = spec_sleep_time_millis * tsd->sleep_multiplier_factor;
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
         "tsd->sleep_multiplier_factor=%2.1f, adjusted_sleep_time_millis=%d",
         tsd->sleep_multiplier_factor, adjusted_sleep_time_millis);

   if (tsd->dynamic_sleep_enabled) {
      dsa_update_adjustment_factor(dh, spec_sleep_time_millis);
      adjusted_sleep_time_millis =
            tsd->cur_sleep_adjustment_factor * adjusted_sleep_time_millis;

      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
                "Dynamic sleep enabled. Updated adjustment factor: %4.2f",
                tsd->cur_sleep_adjustment_factor);
   }
   else {
      adjusted_sleep_time_millis =
            tsd->sleep_multiplier_ct * adjusted_sleep_time_millis;

      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
             "Dynamic sleep disabled. sleep_multiplier_ct = %d",
             tsd->sleep_multiplier_ct);
   }

   DBGTRC_DONE(debug, TRACE_GROUP, "Returning %d milliseconds", adjusted_sleep_time_millis);
   return adjusted_sleep_time_millis;
}


/** Determine the for the period of time to sleep after a DDC IO operation, then
 *  either sleep immediately or, if deferrable sleep is in effect, queue
 *  the sleep for later execution.
 *
 *  Steps:
 *  - Determine the spec sleep time for the event type.
 *  - Call adjust_sleep_time() to modify the sleep time based on the
 *    sleep multiplier and the error rate.
 *  - If deferrable sleep is not in effect (normal case) sleep for the
 *    calculated time.
 *  - If deferrable sleep is in effect, note in the thread-specific data the
 *    earliest possible time for the next DDC operation in the current thread.
 *
 * @param event_type  reason for sleep
 * @oaran special_sleep_time_millis  sleep time for event_type SE_SPECIAL
 * @param func        name of function that invoked sleep
 * @param lineno      line number in file where sleep was invoked
 * @param filename    name of file from which sleep was invoked
 * @param msg         text to append to trace message
 */
void tuned_sleep_with_trace(
      Display_Handle * dh,
      Sleep_Event_Type event_type,
      int              special_sleep_time_millis,
      const char *     func,
      int              lineno,
      const char *     filename,
      const char *     msg)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
         "dh=%s, sleep event type=%s, special_sleep_time_millis=%d",
         dh_repr(dh), sleep_event_name(event_type),special_sleep_time_millis);
   DBGTRC_NOPREFIX(debug, TRACE_GROUP,
         "Called from func=%s, filename=%s, lineno=%d, msg=|%s|",
         func, filename, lineno, msg);
   assert(dh);
   assert( (event_type != SE_SPECIAL && special_sleep_time_millis == 0) ||
           (event_type == SE_SPECIAL && special_sleep_time_millis >  0) );
   assert(dh->dref->io_path.io_mode == DDCA_IO_I2C);

   bool deferrable_sleep = false;
   int spec_sleep_time_millis = get_sleep_time(event_type, special_sleep_time_millis, &deferrable_sleep);
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
          "After get_sleep_time(). spec_sleep_time_millis = %d, deferrable sleep: %s",
          spec_sleep_time_millis,SBOOL(deferrable_sleep));

   int adjusted_sleep_time_millis = adjust_sleep_time(dh, spec_sleep_time_millis);
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
         "After adjust_sleep_time(), adjusted_sleep_time_millis = %d", adjusted_sleep_time_millis);

   record_sleep_event(event_type);

   if (deferrable_sleep) {
      uint64_t new_deferred_time =
            cur_realtime_nanosec() + (1000 *1000) * (int) adjusted_sleep_time_millis;
      if (new_deferred_time > dh->dref->next_i2c_io_after) {
         dh->dref->next_i2c_io_after = new_deferred_time;
         DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
                "Updated deferred sleep time, new_deferred_time=%"PRIu64"", new_deferred_time);
      }
   }
   else {
      char msg_buf[100];
      const char * evname = sleep_event_name(event_type);
      if (msg)
         g_snprintf(msg_buf, 100, "Event type: %s, %s", evname, msg);
      else
         g_snprintf(msg_buf, 100, "Event_type: %s", evname);

      sleep_millis_with_trace(adjusted_sleep_time_millis, func, lineno, filename, msg_buf);
      Per_Thread_Data * ptd = ptd_get_per_thread_data();
      ptd->total_sleep_time_millis += adjusted_sleep_time_millis;
   }

   DBGTRC_DONE(debug, TRACE_GROUP, "");
}


/** Compares if the current clock time is less than the delayed io start time
 *  for a display handle, and if so sleeps for the difference.
 *
 *  The delayed io start time is stored in the display reference associated with
 *  the display handle, so persists across open and close
 *
 *  @param  dh        #Display_Handle
 *  #param  func      name of function performing check
 *  @param  lineno    line number of check
 *  @param  filename  file from which the check is invoked
 */
void check_deferred_sleep(
      Display_Handle * dh,
      const char *     func,
      int              lineno,
      const char *     filename)
{
   bool debug = false;
   uint64_t curtime = cur_realtime_nanosec();

   DBGTRC_STARTING(debug, TRACE_GROUP,"Checking from %s() at line %d in file %s", func, lineno, filename);
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "curtime=%"PRIu64", next_i2c_io_after=%"PRIu64,
                                curtime / (1000*1000), dh->dref->next_i2c_io_after/(1000*1000));
   if (dh->dref->next_i2c_io_after > curtime) {
      int sleep_time = (dh->dref->next_i2c_io_after - curtime)/ (1000*1000);
      DBGTRC_NOPREFIX(debug, TRACE_GROUP, "Sleeping for %d milliseconds", sleep_time);
      sleep_millis_with_trace(sleep_time, func, lineno, filename, "deferred");
      DBGTRC_DONE(debug, TRACE_GROUP,"");
   }
   else {
      DBGTRC_DONE(debug, TRACE_GROUP, "No sleep necessary");
   }
}


/** Module initialization */
void init_tuned_sleep() {
   RTTI_ADD_FUNC(get_sleep_time);
   RTTI_ADD_FUNC(adjust_sleep_time);
   RTTI_ADD_FUNC(check_deferred_sleep);
   RTTI_ADD_FUNC(tuned_sleep_with_trace);
}
