/** @file app_interrogate.c
 *  Implement the INTERROGATE command
 */

// Copyright (C) 2021-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <assert.h>
#include <glib-2.0/glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"

#include "public/ddcutil_types.h"

#include "util/string_util.h"

#include "base/core.h"
#include "base/parms.h"
#include "base/rtti.h"
#include "base/thread_sleep_data.h"

#include "cmdline/parsed_cmd.h"

#include "vcp/persistent_capabilities.h"

#include "i2c/i2c_execute.h"

#include "ddc/ddc_displays.h"
#include "ddc/ddc_services.h"
#include "ddc/ddc_try_stats.h"

#include "app_sysenv/query_sysenv.h"

#include "app_probe.h"

#include "app_interrogate.h"

// Default trace class for this file
static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_TOP;

#ifdef ENABLE_ENVCMDS

static void reset_stats() {
   ddc_reset_stats_main();
}


/** Execute the INTERROGATE command
 *
 *  This convenience command executes the ENVIRONMENT, DETECT, and
 *  for each detected display, the PROBE command.
 *
 *  \param parsed_cmd  parsed command line
 */
void app_interrogate(Parsed_Cmd * parsed_cmd)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "");
   dup2(1,2);   // redirect stderr to stdout
   // set_ferr(fout);    // ensure that all messages are collected - made unnecessary by dup2()
   f0printf(fout(), "Setting output level very-verbose...\n");
   set_output_level(DDCA_OL_VV);  // affects this thread only
   f0printf(fout(), "Setting maximum retries...\n");
   try_data_set_maxtries2(WRITE_ONLY_TRIES_OP, MAX_MAX_TRIES);
   try_data_set_maxtries2(WRITE_READ_TRIES_OP, MAX_MAX_TRIES);
   try_data_set_maxtries2(MULTI_PART_READ_OP,  MAX_MAX_TRIES);
   try_data_set_maxtries2(MULTI_PART_WRITE_OP, MAX_MAX_TRIES);
   f0printf(fout(), "Forcing --stats...\n");
   parsed_cmd->stats_types = DDCA_STATS_ALL;
   f0printf(fout(), "Forcing --disable-capabilities-cache\n");
   enable_capabilities_cache(false);
   f0printf(fout(), "Forcing --force-slave-address..\n");
   i2c_forceable_slave_addr_flag = true;
   f0printf(fout(), "This command will take a while to run...\n\n");

   ddc_ensure_displays_detected();    // *** ???
   DBGTRC_NOPREFIX(debug, TRACE_GROUP, "display detection complete");

   // ENVIRONMENT command
   query_sysenv();
#ifdef USE_USB
   // 7/2017: disable, USB attached monitors are rare, and this just
   // clutters the output
   f0printf(fout(), "\nSkipping USB environment exploreration.\n");
   f0printf(fout(), "Issue command \"ddcutil usbenvironment --verbose\" if there are any USB attached monitors.\n");
   // query_usbenv();
#endif
   f0printf(fout(), "\nStatistics for environment exploreration:\n");
   ddc_report_stats_main(DDCA_STATS_ALL, parsed_cmd->flags & CMD_FLAG_PER_THREAD_STATS, 0);
   reset_stats();

   // PROBE command
   f0printf(fout(), "Setting output level normal. Table features will be skipped...\n");
   set_output_level(DDCA_OL_NORMAL);  // affects this thread only

   tsd_dsa_enable_globally(parsed_cmd->flags & CMD_FLAG_DSA);   // should this apply to INTERROGATE?
   GPtrArray * all_displays = ddc_get_all_displays();
   for (int ndx=0; ndx < all_displays->len; ndx++) {
      Display_Ref * dref = g_ptr_array_index(all_displays, ndx);
      assert( memcmp(dref->marker, DISPLAY_REF_MARKER, 4) == 0);
      if (dref->dispno < 0) {
         f0printf(fout(), "\nSkipping invalid display on %s\n", dref_short_name_t(dref));
      }
      else {
         f0printf(fout(), "\nProbing display %d\n", dref->dispno);
         app_probe_display_by_dref(dref);
         f0printf(fout(), "\nStatistics for probe of display %d:\n", dref->dispno);
         ddc_report_stats_main(DDCA_STATS_ALL, parsed_cmd->flags & CMD_FLAG_PER_THREAD_STATS, 0);
      }
      reset_stats();
   }
   f0printf(fout(), "\nDisplay scanning complete.\n");
   DBGTRC_DONE(debug, TRACE_GROUP, "");
}
#endif

void init_app_interrogate() {
   RTTI_ADD_FUNC(app_interrogate);
}

