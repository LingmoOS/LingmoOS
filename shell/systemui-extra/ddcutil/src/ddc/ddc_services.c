/** @file ddc_services.c
#include <base/base_services.h>
 *
 * ddc layer initialization and configuration, statistics management
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

/** \cond */
#include <stdio.h>

#include "util/report_util.h"
/** \endcond */

#include "base/feature_metadata.h"
#include "base/parms.h"
#include "base/rtti.h"
#include "base/sleep.h"
#include "base/tuned_sleep.h"
#include "base/thread_retry_data.h"
#include "base/thread_sleep_data.h"

#include "vcp/parse_capabilities.h"
#include "vcp/persistent_capabilities.h"
#include "vcp/vcp_feature_codes.h"

#include "dynvcp/dyn_feature_codes.h"
#include "dynvcp/dyn_feature_files.h"

#include "i2c/i2c_services.h"

#include "usb/usb_displays.h"

#include "ddc/ddc_display_lock.h"
#include "ddc/ddc_displays.h"
#include "ddc/ddc_display_ref_reports.h"
#include "ddc/ddc_dumpload.h"
#include "ddc/ddc_multi_part_io.h"
#include "ddc/ddc_output.h"
#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_read_capabilities.h"
#include "ddc/ddc_try_stats.h"
#include "ddc/ddc_vcp.h"
#ifdef BUILD_SHARED_LIB
#include "ddc/ddc_watch_displays.h"
#endif

#include "ddc/ddc_services.h"


//
// Statistics
//

// /** Resets all DDC level statistics */
// void ddc_reset_ddc_stats() {
//    try_data_reset2_all();
// }


/** Resets all statistics */
void ddc_reset_stats_main() {
   // ddc_reset_ddc_stats();
   try_data_reset2_all();
   reset_execution_stats();
}


/** Master function for reporting statistics.
 *
 * \param stats                  bitflags indicating which statistics to report
 * \param show_per_thread_stats  include per thread execution stats
 * \param depth logical          indentation depth
 */
void ddc_report_stats_main(DDCA_Stats_Type stats, bool show_per_thread_stats, int depth) {
   // DBGMSG("show_per_thread_stats: %s", sbool(show_per_thread_stats));
   // int d1 = depth+1;
   rpt_nl();
   rpt_label(depth, "EXECUTION STATISTICS");
   rpt_nl();

   if (stats & DDCA_STATS_TRIES) {
      ddc_report_ddc_stats(depth);
      rpt_nl();

      // Consistency check:
      // report_all_thread_retry_data(depth);
   }

   if (stats & DDCA_STATS_ERRORS) {
      report_all_status_counts(depth);   // error code counts
      rpt_nl();
   }

   if (stats & DDCA_STATS_CALLS) {
      report_execution_stats(depth);
      rpt_nl();

#ifdef OLD
      if (dsa_is_enabled()) {
         // for now just report current thread
         dsa_report_stats(depth);
         rpt_nl();
      }
#endif

      report_io_call_stats(depth);
      rpt_nl();
      report_sleep_stats(depth);
      rpt_nl();
      report_elapsed_stats(depth);
      rpt_nl();
   }

   if (stats & (DDCA_STATS_ELAPSED)) {
      report_elapsed_summary(depth);
      rpt_nl();
   }


   if (show_per_thread_stats) {
      rpt_label(depth, "PER-THREAD EXECUTION STATISTICS");
      rpt_nl();
      ptd_list_threads(depth);
       if (stats & DDCA_STATS_TRIES) {
           report_all_thread_maxtries_data(depth);
      }
      if (stats & DDCA_STATS_ERRORS) {
          report_all_thread_status_counts(depth);
          rpt_nl();
      }
      if (stats & DDCA_STATS_CALLS) {
          report_all_thread_sleep_data(depth);
      }
      if (stats & (DDCA_STATS_ELAPSED)) {
          // need a report_all_thread_elapsed_summary()
          report_elapsed_summary(depth);     // temp?
          //   rpt_nl();
      }

      // Reports locks held by per_thread_data() to confirm that locking has
      // trivial affect on performance.
      //dbgrpt_per_thread_data_locks(depth+1);
   }
}


/** Master initialization function for DDC services
 */
void init_ddc_services() {
   bool debug = false;
   DBGMSF(debug, "Starting");

   // i2c:
   init_i2c_services();

   // usb
#ifdef USE_USB
   init_usb_displays();
#endif

   // ddc:
   try_data_init();
   init_persistent_capabilities();
   init_parse_capabilities();
   init_vcp_feature_codes();
   init_dyn_feature_codes();    // must come after init_vcp_feature_codes()
   init_dyn_feature_files();
   init_ddc_display_lock();
   init_ddc_display_ref_reports();
   init_ddc_displays();
   init_ddc_dumpload();
   init_ddc_output();
   init_ddc_packet_io();
   init_ddc_read_capabilities();
   init_ddc_multi_part_io();
   init_ddc_vcp();
#ifdef BUILD_SHARED_LIB
   init_ddc_watch_displays();
#endif

   // dbgrpt_rtti_func_name_table(1);
   DBGMSF(debug, "Done");
}
