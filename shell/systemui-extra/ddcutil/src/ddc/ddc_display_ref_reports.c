/** @file ddc_display_ref_reports.c
 *  Report functions factored out of ddc_displays.c due to size of that file.
 *  ddc_display_ref_reports.c and ddc_displays.c cross-reference each other.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <glib-2.0/glib.h>
#include <string.h>
#include <sys/stat.h>

#include "util/data_structures.h"
#include "util/report_util.h"
#include "util/string_util.h"

#include "public/ddcutil_types.h"

#include "base/core.h"
#include "base/monitor_model_key.h"
#include "base/monitor_quirks.h"
#include "base/rtti.h"

#include "i2c/i2c_bus_core.h"
#include "i2c/i2c_sysfs.h"

#ifdef USE_USB
#include "usb/usb_displays.h"
#endif

#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_vcp_version.h"
#include "ddc/ddc_vcp.h"

#include "ddc/ddc_displays.h"
#include "ddc/ddc_display_ref_reports.h"


// Default trace class for this file
static const DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_DDC;

//
// Display_Ref reports
//

/** Gets the controller firmware version as a string
 *
 * @param dh  pointer to display handle
 * @return    pointer to character string, which is valid until the next
 *            call to this function.
 *
 * @remark
 * Consider caching the value in dh->dref
 */
// static
char *
get_firmware_version_string_t(Display_Handle * dh) {
   bool debug = false;

   DBGTRC_STARTING(debug, TRACE_GROUP, "dh=%s", dh_repr(dh));
   static GPrivate  firmware_version_key = G_PRIVATE_INIT(g_free);
   char * version = get_thread_fixed_buffer(&firmware_version_key, 40);

   DDCA_Any_Vcp_Value * valrec = NULL;
   Public_Status_Code psc = 0;
   Error_Info * ddc_excp = ddc_get_vcp_value(
                               dh,
                               0xc9,                     // firmware detection
                               DDCA_NON_TABLE_VCP_VALUE,
                               &valrec);
   psc = (ddc_excp) ? ddc_excp->status_code : 0;
   if (psc != 0) {
      strcpy(version, "Unspecified");
      if (psc != DDCRC_REPORTED_UNSUPPORTED && psc != DDCRC_DETERMINED_UNSUPPORTED) {
         DBGMSF(debug, "get_vcp_value(0xc9) returned %s", psc_desc(psc));
         strcpy(version, "DDC communication failed");
         if (debug || IS_TRACING() || is_report_ddc_errors_enabled())
            errinfo_report(ddc_excp, 1);
      }
      errinfo_free(ddc_excp);
   }
   else {
      g_snprintf(version, 40, "%d.%d", valrec->val.c_nc.sh, valrec->val.c_nc.sl);
      free_single_vcp_value(valrec);
   }
   DBGTRC_DONE(debug, TRACE_GROUP, "Returning: %s", version);
   return version;
}


/** Gets the controller manufacturer name for an open display.
 *
 * @param dh  pointer to display handle
 * @return pointer to character string, which is valid until the next
 * call to this function.
 *
 * @remark
 * Consider caching the value in dh->dref
 */
static char *
get_controller_mfg_string_t(Display_Handle * dh) {
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "dh = %s", dh_repr(dh));

   const int MFG_NAME_BUF_SIZE = 100;

   static GPrivate  buf_key = G_PRIVATE_INIT(g_free);
   char * mfg_name_buf = get_thread_fixed_buffer(&buf_key, MFG_NAME_BUF_SIZE);

   char * mfg_name = NULL;
   DDCA_Any_Vcp_Value * valrec;

   DDCA_Status ddcrc = 0;
   Error_Info * ddc_excp = ddc_get_vcp_value(dh, 0xc8, DDCA_NON_TABLE_VCP_VALUE, &valrec);
   ddcrc = (ddc_excp) ? ddc_excp->status_code : 0;

   if (ddcrc == 0) {
      DDCA_Feature_Value_Entry * vals = pxc8_display_controller_type_values;
      mfg_name =  sl_value_table_lookup(
                            vals,
                            valrec->val.c_nc.sl);
      if (!mfg_name) {
         g_snprintf(mfg_name_buf, MFG_NAME_BUF_SIZE,
                       "Unrecognized manufacturer code 0x%02x",
                       valrec->val.c_nc.sl);

         mfg_name = mfg_name_buf;
      }
      free_single_vcp_value(valrec);
   }
   else if (ddcrc == DDCRC_REPORTED_UNSUPPORTED || ddcrc == DDCRC_DETERMINED_UNSUPPORTED) {
      mfg_name = "Unspecified";
      errinfo_free(ddc_excp);
   }
   else {
      // if (debug) {
      //    DBGMSG("get_nontable_vcp_value(0xc8) returned %s", psc_desc(ddcrc));
      //    DBGMSG("    Try errors: %s", errinfo_causes_string(ddc_excp));
      // }
      ERRINFO_FREE_WITH_REPORT(ddc_excp, debug || IS_TRACING() || is_report_ddc_errors_enabled() );
      mfg_name = "DDC communication failed";
    }

   DBGTRC_DONE(debug, TRACE_GROUP, "Returning: %s", mfg_name);
   return mfg_name;
}


/** Shows information about a display, specified by a #Display_Ref
 *
 *  This function is used by the DISPLAY command.
 *
 *  Output is written using report functions
 *
 * @param dref   pointer to display reference
 * @param depth  logical indentation depth
 *
 * @remark
 * The detail level shown is controlled by the output level setting
 * for the current thread.
 */
void
ddc_report_display_by_dref(Display_Ref * dref, int depth) {
   bool debug = false;
   // DBGTRC_STARTING(debug, TRACE_GROUP, "dref=%s, communication flags: %s",
   //               dref_repr_t(dref), dref_basic_flags_t(dref->flags));
   DBGTRC_STARTING(debug, TRACE_GROUP, "dref=%s",  dref_repr_t(dref));
   DBGTRC_NOPREFIX(debug, TRACE_GROUP, "dref->flags: %s", interpret_dref_flags_t(dref->flags));
   TRACED_ASSERT(dref && memcmp(dref->marker, DISPLAY_REF_MARKER, 4) == 0);
   int d1 = depth+1;

   switch(dref->dispno) {
   case DISPNO_BUSY:       // -4
      rpt_vstring(depth, "Busy display");
      break;
   case DISPNO_REMOVED:  // -3
      rpt_vstring(depth, "Removed display");
      break;
   case DISPNO_PHANTOM:    // -2
      rpt_vstring(depth, "Phantom display");
      break;
   case DISPNO_INVALID:   // -1
      rpt_vstring(depth, "Invalid display");
      break;
   case 0:          // valid display, no assigned display number
      d1 = depth;   // adjust indent  ??
      break;
   default:         // normal case
      rpt_vstring(depth, "Display %d", dref->dispno);
   }

   switch(dref->io_path.io_mode) {
   case DDCA_IO_I2C:
      {
         I2C_Bus_Info * curinfo = dref->detail;
         TRACED_ASSERT(curinfo && memcmp(curinfo, I2C_BUS_INFO_MARKER, 4) == 0);

         i2c_report_active_display(curinfo, d1);
      }
      break;
   case DDCA_IO_ADL:
      PROGRAM_LOGIC_ERROR("ADL implementation removed");
      break;
   case DDCA_IO_USB:
#ifdef USE_USB
      usb_show_active_display_by_dref(dref, d1);
#else
      PROGRAM_LOGIC_ERROR("ddcutil not built with USB support");
#endif
      break;
   }

   TRACED_ASSERT(dref->flags & DREF_DDC_COMMUNICATION_CHECKED);

   DDCA_Output_Level output_level = get_output_level();

   if (output_level >= DDCA_OL_NORMAL) {
      if (!(dref->flags & DREF_DDC_COMMUNICATION_WORKING) ) {
         rpt_vstring(d1, "DDC communication failed");
         char msgbuf[100] = {0};
         char * msg = NULL;
         if (dref->dispno == DISPNO_PHANTOM) {
            if (dref->actual_display) {
               snprintf(msgbuf, 100, "Use non-phantom device %s",
                        dref_short_name_t(dref->actual_display));
               msg = msgbuf;
            }
            else {
               // should never occur
               msg = "Use non-phantom device";
            }
         }
         else {
            if (dref->io_path.io_mode == DDCA_IO_I2C)
            {
                I2C_Bus_Info * curinfo = dref->detail;
                if (curinfo->flags & I2C_BUS_EDP)
                    msg = "This is an eDP laptop display. Laptop displays do not support DDC/CI.";
                else if (curinfo->flags & I2C_BUS_LVDS)
                     msg = "This is a LVDS laptop display. Laptop displays do not support DDC/CI.";
                else if ( is_embedded_parsed_edid(dref->pedid) )
                    msg = "This appears to be a laptop display. Laptop displays do not support DDC/CI.";
                // else if ( curinfo->flags & I2C_BUS_BUSY) {
                else if ( dref->dispno == DISPNO_BUSY) {
                   rpt_label(d1, "I2C device is busy");
                   int busno = dref->io_path.path.i2c_busno;

                   GPtrArray * conflicts = collect_conflicting_drivers(busno, -1);
                   if (conflicts && conflicts->len > 0) {
                      // report_conflicting_drivers(conflicts);
                      rpt_vstring(d1, "Likely conflicting drivers: %s", conflicting_driver_names_string_t(conflicts));
                      free_conflicting_drivers(conflicts);
                   }
                   else {
                      struct stat stat_buf;
                      char buf[20];
                      g_snprintf(buf, 20, "/dev/bus/ddcci/%d", dref->io_path.path.i2c_busno);
                      // DBGMSG("buf: %s", buf);
                      int rc = stat(buf, &stat_buf);
                      // DBGMSG("stat returned %d", rc);
                      if (rc == 0)
                         rpt_label(d1, "I2C device is busy.  Likely conflict with driver ddcci.");
                   }
// #ifndef I2C_IO_IOCTL_ONLY
                   msg = "Try using option --force-slave-address";
// #endif
                }
            }
#ifdef TMI
            if (output_level >= DDCA_OL_VERBOSE) {
               if (!msg) {
                  msg = "Is DDC/CI enabled in the monitor's on-screen display?";
               }
            }
#endif
         }
         if (msg) {
            rpt_vstring(d1, msg);
         }
      }
      else {    // communication working
         DDCA_MCCS_Version_Spec vspec = get_vcp_version_by_dref(dref);
         // DBGMSG("vspec = %d.%d", vspec.major, vspec.minor);
         if ( vspec.major   == 0)
            rpt_vstring(d1, "VCP version:         Detection failed");
         else
            rpt_vstring(d1, "VCP version:         %d.%d", vspec.major, vspec.minor);

         if (output_level >= DDCA_OL_VERBOSE) {
            // n. requires write access since may call get_vcp_value(), which does a write
            Display_Handle * dh = NULL;
            DBGMSF(debug, "Calling ddc_open_display() ...");
            Public_Status_Code psc = ddc_open_display(dref, CALLOPT_ERR_MSG, &dh);
            if (psc != 0) {
               rpt_vstring(d1, "Error opening display %s, error = %s",
                                  dref_short_name_t(dref), psc_desc(psc));
            }
            else {
               // display controller mfg, firmware version
               rpt_vstring(d1, "Controller mfg:      %s", get_controller_mfg_string_t(dh) );
               rpt_vstring(d1, "Firmware version:    %s", get_firmware_version_string_t(dh));
               DBGMSF(debug, "Calling ddc_close_display()...");
               ddc_close_display(dh);
            }

            if (dref->io_path.io_mode != DDCA_IO_USB) {
               rpt_vstring(d1, "Monitor returns DDC Null Response for unsupported features: %s",
                                  sbool(dref->flags & DREF_DDC_USES_NULL_RESPONSE_FOR_UNSUPPORTED));
               // rpt_vstring(d1, "Monitor returns success with mh=ml=sh=sl=0 for unsupported features: %s",
               //                    sbool(dref->flags & DREF_DDC_USES_MH_ML_SH_SL_ZERO_FOR_UNSUPPORTED));
            }
         }
         DDCA_Monitor_Model_Key mmk = monitor_model_key_value_from_edid(dref->pedid);
         // DBGMSG("mmk = %s", mmk_repr(mmk) );
         Monitor_Quirk_Data * quirk = get_monitor_quirks(&mmk);
         if (quirk) {
            char * msg = NULL;
            switch(quirk->quirk_type) {
            case  MQ_NONE:
               break;
            case MQ_NO_SETTING:
               msg = "WARNING: Setting feature values has been reported to permanently cripple this monitor!";
               break;
            case MQ_NO_MFG_RANGE:
               msg = "WARNING: Setting manufacturer reserved features has been reported to permanently cripple this monitor!";
               break;
            case MQ_OTHER:
               msg = quirk->quirk_msg;
            }
            if (msg)
               rpt_vstring(d1, msg);
         }
      }
   }

   DBGTRC_DONE(debug, TRACE_GROUP, "");
}


/** The following set of definitions and functions, called from ddc_report_displays(),
 *  address a highly unlikely corner case, but one which has been reported.
 *  (See https://github.com/rockowitz/ddcutil/issues/280)
 *
 *  Normally, the drm connector associated with an I2C bus, e.g. CARD0-DP-1,
 *  is obtained by traversing /sys, starting with /sys/bus/i2c,
 *  using function find_sys_drm_connector_by_busion().
 *
 *  However, it is possible that a driver does not put sufficient information
 *  in /sys. This is the case with the proprietary nvidia driver. In this
 *  situation function find_sys_drm_connector_by_edid() is used instead.
 *  However, this method can give the wrong answer if two monitors have the same
 *  EDID, i.e. the EDID inadequately distinguishes monitors.
 *
 *  In issue #280, the nvidia proprietary driver was in use, and there were two
 *  identical Asus PG239Q monitors.  The EDID contained neither an ASCII nor a
 *  binary serial number, and the monitors had the same manufacture year/week,
 *  so the EDIDs were identical. In this case the wrong connector name was
 *  reported for one monitor.
 *
 *  It is at least possible in this situation to warn the user that the
 *  DRM connector name may be incorrect.
 */
typedef struct {
   Byte * edid;                     ///< 128 byte EDID
   Bit_Set_256 bus_numbers;         ///< numbers of the busses whose monitor has this EDID
} EDID_Use_Record;


/** Create array of #Edid_Use_Record
 */
static GPtrArray *
create_edid_use_table() {
   GPtrArray * result = g_ptr_array_new();
   return result;
}


/** Frees an array of #Edid_Use_Record
 */
static void
free_edid_use_table(GPtrArray* table) {
      // free's each Edid_Use_Record, but not the edid the records point to
      g_ptr_array_free(table, true);
   }


/** Returns the EDID_Use_Record for a particular EDID.
 *  If one does not yet exist, it is created.
 *
 *  @param records #GPtrArray of #EDID_Use_Record
 *  @param edid    edid to search for
 *  @return #EDID_Use_Record
 */
static EDID_Use_Record *
get_edid_use_record(GPtrArray * records, Byte * edid) {
   assert(edid);
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "records = %p, records->len = %d, edid -> ...%s",
         records, records->len, hexstring_t(edid+122,6));

   EDID_Use_Record * result = NULL;
   for (int ndx = 0; ndx < records->len; ndx++) {
      EDID_Use_Record * cur = g_ptr_array_index(records, ndx);
      if (memcmp(cur->edid,edid,128) == 0) {
         result = cur;
         DBGTRC_DONE(debug, DDCA_TRC_NONE, "Returning existing EDID_Use_Record %p for edid %s",
                       cur, hexstring_t(edid+122,6));
         break;
      }
   }
   if (!result) {
      result = calloc(1, sizeof(EDID_Use_Record));
      result->edid = edid;
      DBGTRC_DONE(debug, DDCA_TRC_NONE, "Returning new EDID_Use_Record %p for edid %s",
                    result, hexstring_t(edid+122,6) );
      g_ptr_array_add(records, result);
   }
   return result;
 }


/** Record the #Display_Ref's I2C bus number in the #EDID_Use_Record
 *  for the display.
 *
 *  @param records  #GPtrArray of #EDID_Use_Record
 *  @param dref     display reference
 *
 *  @remark
 *  Does nothing unless the display reference is for an I2C device and the
 *  drm connector was found using the EDID.
 */
static void
record_i2c_edid_use(GPtrArray * edid_use_records, Display_Ref * dref) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "edid_use_records=%p, dref=%s", edid_use_records, dref_repr_t(dref));
   if (dref->io_path.io_mode == DDCA_IO_I2C) {
      I2C_Bus_Info * binfo = (I2C_Bus_Info *) dref->detail;
      if (binfo -> drm_connector_found_by == DRM_CONNECTOR_FOUND_BY_EDID) {
         EDID_Use_Record * cur = get_edid_use_record(edid_use_records, binfo->edid->bytes);
         cur->bus_numbers = bs256_insert(cur->bus_numbers, binfo->busno);
         DBGTRC_DONE(debug, DDCA_TRC_NONE, "Updated bus list %s for edid %s",
                         bs256_to_string_decimal(cur->bus_numbers, NULL, ", "),
                         hexstring_t(binfo->edid->bytes+122,6));
      }
   }
}


/** Report i2c buses having identical EDID, for which the DRM
 *  connector name was found using the EDID
 *
 *  @param  edid_use_records  GPtrArray of #EDID_Use_Record
 *  @param  depth             logical indentation depth
 */
static void
report_ambiguous_connector_for_edid(GPtrArray * edid_use_records, int depth) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "edid_use_records->len = %d", edid_use_records->len);
   for (int ndx = 0; ndx < edid_use_records->len; ndx++) {
      EDID_Use_Record * cur_use_record = g_ptr_array_index(edid_use_records, ndx);
      if (bs256_count(cur_use_record->bus_numbers) > 1) {
         rpt_vstring(depth,"Displays with I2C bus numbers %s have identical EDIDs.",
                             bs256_to_string_decimal(cur_use_record->bus_numbers, NULL, ", "));
         rpt_label(depth, "DRM connector names may not be accurate.");
      }
   }
   DBGTRC_DONE(debug, DDCA_TRC_NONE, "");
}


/** Reports all displays found.
 *
 * Output is written to the current report destination using report functions.
 *
 * @param   include_invalid_displays  if false, report only valid displays\n
 *                                    if true,  report all displays
 * @param   depth       logical indentation depth
 *
 * @return total number of displays reported
 */
int
ddc_report_displays(bool include_invalid_displays, int depth) {
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "");

   ddc_ensure_displays_detected();

   int display_ct = 0;
   GPtrArray * all_displays = ddc_get_all_displays();
   GPtrArray * edid_use_records = create_edid_use_table();
   // DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "Created edid_use_records = %p", edid_use_records);
   for (int ndx=0; ndx<all_displays->len; ndx++) {
      Display_Ref * dref = g_ptr_array_index(all_displays, ndx);
      TRACED_ASSERT(memcmp(dref->marker, DISPLAY_REF_MARKER, 4) == 0);
      if (dref->dispno > 0 || include_invalid_displays) {
         display_ct++;
         ddc_report_display_by_dref(dref, depth);
         rpt_title("",0);

         // Note the EDID for each bus
         record_i2c_edid_use(edid_use_records, dref);
      }
   }
   if (display_ct == 0) {
      rpt_vstring(depth, "No %sdisplays found.", (!include_invalid_displays) ? "active " : "");
      if ( get_output_level() >= DDCA_OL_NORMAL ) {
         // rpt_label(depth, "Is DDC/CI enabled in the monitor's on screen display?");
         rpt_label(depth, "Run \"ddcutil environment\" to check for system configuration problems.");
      }
   }
   else if (get_output_level() >= DDCA_OL_VERBOSE && display_ct > 1) {
      report_ambiguous_connector_for_edid(edid_use_records, depth);
   }
   free_edid_use_table(edid_use_records);

   DBGTRC_DONE(debug, TRACE_GROUP, "Returning: %d", display_ct);
   return display_ct;
}


/** Debugging function to display the contents of a #Display_Ref.
 *
 * @param dref  pointer to #Display_Ref
 * @param depth logical indentation depth
 */
void
ddc_dbgrpt_display_ref(Display_Ref * dref, int depth) {
   assert(dref);
   bool debug = false;
   DBGMSF(debug, "Starting. dref=%s", dref_repr_t(dref));
   int d1 = depth+1;
   int d2 = depth+2;

   rpt_structure_loc("Display_Ref", dref, depth);
   rpt_int("dispno", NULL, dref->dispno, d1);

   dbgrpt_display_ref(dref, d1);

   rpt_vstring(d1, "io_mode: %s", io_mode_name(dref->io_path.io_mode));
   switch(dref->io_path.io_mode) {
   case(DDCA_IO_I2C):
         rpt_vstring(d1, "I2C bus information: ");
         I2C_Bus_Info * businfo = dref->detail;
         TRACED_ASSERT( memcmp(businfo->marker, I2C_BUS_INFO_MARKER, 4) == 0);
         i2c_dbgrpt_bus_info(businfo, d2);
         break;
   case(DDCA_IO_ADL):
         PROGRAM_LOGIC_ERROR("ADL implementation removed");
         break;
   case(DDCA_IO_USB):
#ifdef USE_USB
         rpt_vstring(d1, "USB device information: ");
         Usb_Monitor_Info * moninfo = dref->detail;
         TRACED_ASSERT(memcmp(moninfo->marker, USB_MONITOR_INFO_MARKER, 4) == 0);
         dbgrpt_usb_monitor_info(moninfo, d2);
#else
         PROGRAM_LOGIC_ERROR("Built without USB support");
#endif
   break;
   }

   // set_output_level(saved_output_level);
   DBGMSF(debug, "Done");
}


/** Emits a debug report a GPtrArray of display references
 *
 *  @param msg       initial message line
 *  @param ptrarray  array of pointers to #Display_Ref
 *  @param depth     logical indentation depth
 */
void
ddc_dbgrpt_drefs(char * msg, GPtrArray * ptrarray, int depth) {
   int d1 = depth + 1;
   rpt_vstring(depth, "%s", msg);
   if (ptrarray->len == 0)
      rpt_vstring(d1, "None");
   else {
      for (int ndx = 0; ndx < ptrarray->len; ndx++) {
         Display_Ref * dref = g_ptr_array_index(ptrarray, ndx);
         TRACED_ASSERT(dref);
         dbgrpt_display_ref(dref, d1);
      }
   }
}


#ifdef UNUSED
void dbgrpt_valid_display_refs(int depth) {
   rpt_vstring(depth, "Valid display refs = all_displays:");
   if (all_displays) {
      if (all_displays->len == 0)
         rpt_vstring(depth+1, "None");
      else {
         for (int ndx = 0; ndx < all_displays->len; ndx++) {
            Display_Ref * dref = g_ptr_array_index(all_displays, ndx);
            rpt_vstring(depth+1, "%p, dispno=%d", dref, dref->dispno);
         }
      }
   }
   else
      rpt_vstring(depth+1, "all_displays == NULL");
}
#endif


void init_ddc_display_ref_reports() {

   RTTI_ADD_FUNC(ddc_report_display_by_dref);
   RTTI_ADD_FUNC(ddc_report_displays);
   RTTI_ADD_FUNC(get_controller_mfg_string_t);
   RTTI_ADD_FUNC(get_edid_use_record);
   RTTI_ADD_FUNC(get_firmware_version_string_t);
   RTTI_ADD_FUNC(record_i2c_edid_use);
   RTTI_ADD_FUNC(report_ambiguous_connector_for_edid);
}


