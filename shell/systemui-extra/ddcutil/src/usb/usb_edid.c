/** @file usb_edid.c
 *
 *  Functions to get EDID for USB connected monitors
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <config.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glib-2.0/glib.h>
#include <linux/hiddev.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "util/device_id_util.h"
#include "util/report_util.h"
#include "util/string_util.h"
#ifdef USE_X11
#include "util/x11_util.h"         // for EDID fallback
#endif

#include "usb_util/hiddev_reports.h"
#include "usb_util/hiddev_util.h"

#include "base/core.h"
#include "base/ddc_errno.h"
#include "base/execution_stats.h"
#include "base/linux_errno.h"

#include "i2c/i2c_bus_core.h"     // for EDID fallback
#include "i2c/i2c_bus_selector.h" // for EDID fallback

#include "usb/usb_base.h"

#include "usb/usb_edid.h"


// Trace class for this file

// Doesn't work
// Avoid unused variable warning if all debug code turned off
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-variable"
// static Trace_Group TRACE_GROUP = TRC_USB;
// #pragma GCC diagnostic pop

static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_USB;

void usb_edid_unused_function_to_avoid_unused_variable_warning() {
   printf("0x%02x\n",TRACE_GROUP);
}

// struct model_sn_pair

struct model_sn_pair {
   char * model;
   char * sn;
};

void free_model_sn_pair(struct model_sn_pair * p) {
   if (p) {
      free(p->model);
      free(p->sn);
      free(p);
   }
}

void report_model_sn_pair(struct model_sn_pair * p, int depth) {
   int d1 = depth+1;
   rpt_structure_loc("struct model_sn_pair",p, depth);
   rpt_vstring(d1, "model:  %s", p->model);
   rpt_vstring(d1, "sn:     %s", p->sn);
}


//
// EIZO Specific Functions
//

/* Locates the EIZO specific HID report that returns the model and serial number.
 *
 * Arguments:
 *   fd        file descriptor for open USB HID device
 *
 * Returns:    pointer to a newly allocated struct hid_field_locator
 */

struct hid_field_locator * find_eizo_model_sn_report(int fd) {
   bool debug = false;
   struct hid_field_locator * loc = NULL;
   struct hiddev_devinfo dev_info;

   int rc = hiddev_get_device_info(fd, &dev_info, CALLOPT_ERR_MSG);
   if (rc != 0)
      goto bye;

   if (dev_info.vendor == 0x056d && dev_info.product == 0x0002)  {
      loc = hiddev_find_report(fd, HID_REPORT_TYPE_FEATURE, 0xff000035, /*match_all_ucodes=*/false);
   }

bye:
   if (debug) {
      DBGMSG("Returning: %p", (void*)loc);
      // if (loc)
      //    report_hid_field_locator(loc,2);
   }
   return loc;
}


#ifdef UNUSED
bool is_eizo_monitor(int fd) {
   bool debug = false;
   bool result = false;
   struct hiddev_devinfo dev_info;
   int rc = ioctl(fd, HIDIOCGDEVINFO, &dev_info);
   if (rc != 0) {
      REPORT_IOCTL_ERROR("HIDIOCGDEVINFO", rc);
      goto bye;
   }
   if (dev_info.vendor == 0x056d && dev_info.product == 0x0002)
      result = true;

bye:
   DBGMSF(debug, "Returning %s", bool_repr(result));
   return result;
}
#endif


/* Gets the module and serial number of an Eizo monitor using an Eizo specific report.
 *
 * Finds the specific report, then reads it.
 *
 * Alternatively:
 * Obtains the values by requesting the value of the usage code for the strings,
 * leaving it to hiddev to find the required report.
 *
 * Arguments:
 *   fd      file descriptor of open USB HID device for an Eizo monitor
 *
 * Returns:  model and serial number strings
 */
struct model_sn_pair *  get_eizo_model_sn_by_report(int fd) {
   bool debug = false;
   DBGMSF(debug, "Starting");
   struct model_sn_pair* result = NULL;
   Buffer * modelsn  = NULL;
   Buffer * modelsn2 = NULL;

   struct hid_field_locator * loc = find_eizo_model_sn_report(fd);
   // DBGMSF(debug, "find_eizo_model_sn_report() returned: %p", loc);
   if (loc)
      modelsn = hiddev_get_multibyte_report_value_by_hid_field_locator(fd, loc);
   modelsn2 = hiddev_get_multibyte_value_by_ucode(
                         fd,
                         0xff000035,  // usage code
                         16);         // num_values
   if (modelsn2 && modelsn2->len >= 16)
      buffer_set_length(modelsn2, 16);
   // printf("modelsn:\n");
   // buffer_dump(modelsn);
   // printf("modelsn2:\n");
   // buffer_dump(modelsn2);
   assert(buffer_eq(modelsn, modelsn2));
   if (modelsn2)
      buffer_free(modelsn2, __func__);

   if (modelsn) {
      assert(modelsn->len >= 16);
      result = calloc(1, sizeof(struct model_sn_pair));
      result->model = calloc(1,9);
      result->sn    = calloc(1,9);
      memcpy(result->sn, modelsn->bytes,8);
      result->sn[8] = '\0';
      memcpy(result->model, modelsn->bytes+8, 8);
      result->model[8] = '\0';
      rtrim_in_place(result->sn);
      rtrim_in_place(result->model);
      free(modelsn);
   }

   if (loc)
      free_hid_field_locator(loc);

   if (debug) {
      if (result) {
         DBGMSG("Returning: %p -> mode=|%s|, sn=|%s|",
                (void*) result, result->model, result->sn);
         // report_model_sn_pair(result, 1);
      }
      else
         DBGMSG("Returning: %p", (void*) result);
   }
   return result;
}


//
//  EDID Retrieval
//

#ifdef USE_X11
/* Obtains EDID from X11.
 *
 * Arguments:
 *   model_name
 *   sn_ascii
 *
 * Returns:   parsed EDID if found
 */
Parsed_Edid * get_x11_edid_by_model_sn(char * model_name, char * sn_ascii) {
   bool debug = false;
   DBGMSF(debug, "Starting.  model_name=|%s|, sn_ascii=|%s|", model_name, sn_ascii);
   Parsed_Edid * parsed_edid = NULL;

   GPtrArray* edid_recs = get_x11_edids();
   // puts("");
   // printf("EDIDs reported by X11 for connected xrandr outputs:\n");
   // DBGMSG("Got %d X11_Edid_Recs\n", edid_recs->len);

   for (int ndx=0; ndx < edid_recs->len; ndx++) {
      X11_Edid_Rec * prec = g_ptr_array_index(edid_recs, ndx);
      // printf(" Output name: %s -> %p\n", prec->output_name, prec->edid);
      // hex_dump(prec->edid, 128);
      DBGMSF(debug, "Comparing EDID for xrandr output: %s", prec->output_name);
      parsed_edid = create_parsed_edid2(prec->edidbytes, "X11");
      if (parsed_edid) {
         if (debug) {
            bool verbose_edid = false;
            report_parsed_edid(parsed_edid, verbose_edid, 2 /* depth */);
         }
         if (streq(parsed_edid->model_name, model_name) &&
               streq(parsed_edid->serial_ascii, sn_ascii) )
         {
            DBGMSF(debug, "Found matching EDID from X11");
            break;
         }
         free_parsed_edid(parsed_edid);
      }
      else {
         if (debug || get_output_level() >= DDCA_OL_VERBOSE) {
            DBGMSG("Unparsable EDID for output name: %s -> %p",
                   prec->output_name, prec->edidbytes);
            rpt_hex_dump(prec->edidbytes, 128, /*depth=*/ 1);
         }
      }
   }


#ifdef MOCK_DATA_FOR_DEVELOPMENT
   if (!parsed_edid && edid_recs->len > 0) {
      printf("(%s) HACK FOR TESTING: Using last X11 EDID\n", __func__);
      X11_Edid_Rec * prec = g_ptr_array_index(edid_recs, edid_recs->len-1);
      parsed_edid = create_parsed_edid(prec->edidbytes);
   }
#endif

   g_ptr_array_free(edid_recs, true);
   DBGMSF(debug, "returning %p", parsed_edid);
   return parsed_edid;
}
#endif


Parsed_Edid * get_fallback_hiddev_edid(int fd, struct hiddev_devinfo * dev_info) {
   bool debug = false;
   DBGMSF(debug, "Starting");

   Parsed_Edid * parsed_edid = NULL;
   char * edid_source;

   struct model_sn_pair * model_sn = NULL;

   // Special handling for Eizo monitors
   if (dev_info->vendor == 0x056d && dev_info->product == 0x0002) {   // if is EIZO monitor?
      DBGMSG("*** Special fixup for Eizo monitor ***");

      model_sn  = get_eizo_model_sn_by_report(fd);

      if (model_sn) {
         // Should there be a ddc level function to find non-usb EDID?
         I2C_Bus_Info * bus_info = i2c_find_bus_info_by_mfg_model_sn(
                                  NULL,               // mfg_id
                                  model_sn->model,
                                  model_sn->sn,
                                  DISPSEL_NONE);
         if (bus_info) {
            DBGMSG("Using EDID for /dev/i2c-%d", bus_info->busno);
            parsed_edid = bus_info->edid;
            edid_source = "I2C";
            // STRLCPY(parsed_edid->edid_source, "I2C", EDID_SOURCE_FIELD_SIZE);
            // result = NULL;   // for testing - both i2c and X11 methods work
         }
         else {    // ADL
#ifdef OLD
            Display_Ref * dref = adlshim_find_display_by_mfg_model_sn(
                                    NULL,              // mfg_id
                                    model_sn->model,
                                    model_sn->sn);
            parsed_edid = adlshim_get_parsed_edid_by_dref(dref);
            DDCA_Adlno adlno = adlshim_find_adlno_by_mfg_model_sn(
                                    NULL,              // mfg_id
                                    model_sn->model,
                                    model_sn->sn);
            if (adlno.iAdapterIndex >= 0)   {    //  {-1,-1} means unset
               parsed_edid = adlshim_get_parsed_edid_by_adlno(adlno.iAdapterIndex, adlno.iDisplayIndex);
               edid_source = "ADL";
                  // STRLCPY(parsed_edid->edid_source, "ADL", EDID_SOURCE_FIELD_SIZE);
            }
            // memory leak: not freeing dref because don't want to clobber parsed_edid
            // need to review Display_Ref lifecycle
#endif
            PROGRAM_LOGIC_ERROR("ADL implementation removed");
         }
      }
   }

#ifdef USE_X11
   if (!parsed_edid && model_sn) {
      parsed_edid = get_x11_edid_by_model_sn(model_sn->model, model_sn->sn);
      edid_source = "X11";
   }
#endif

   if (model_sn)
      free_model_sn_pair(model_sn);
   if (parsed_edid)
      STRLCPY(parsed_edid->edid_source, edid_source, EDID_SOURCE_FIELD_SIZE);
   DBGMSF(debug, "Returning: %p", parsed_edid);
   return parsed_edid;
}


/* Retrieves the EDID (128 bytes) from a hiddev device representing a HID
 * compliant monitor.
 *
 * Arguments:
 *    fd     file descriptor
 *
 * Returns:
 *    pointer to Buffer struct containing the EDID
 *
 * It is the responsibility of the caller to free the returned buffer.
 */
Parsed_Edid * get_hiddev_edid_with_fallback(int fd, struct hiddev_devinfo * dev_info)  {
   bool debug = false;
   if (debug) {
      DBGMSG("Starting");
      dbgrpt_hiddev_devinfo(dev_info, true, 1);
   }

   Parsed_Edid * parsed_edid = NULL;

   Buffer * edid_buffer = hiddev_get_edid(fd);    // in hiddev_util.c
   // try alternative - both work, pick one
   Buffer * edid_buf2   = hiddev_get_multibyte_value_by_ucode(fd, 0x00800002, 128);
   if (edid_buffer && edid_buffer->len > 128)
      buffer_set_length(edid_buffer,128);
   // printf("edid_buffer:\n");
   // buffer_dump(edid_buffer);
   // printf("edid_buf2:\n");
   // buffer_dump(edid_buf2);
   assert(buffer_eq(edid_buffer, edid_buf2));
   if (edid_buf2)
      buffer_free(edid_buf2, __func__);

   if (edid_buffer) {
       parsed_edid = create_parsed_edid2(edid_buffer->bytes, "USB");  // copies bytes
       if (!parsed_edid) {
          DBGMSF(debug, "get_hiddev_edid() returned invalid EDID");
          // if debug or verbose, dump the bad edid  ??
       }
       buffer_free(edid_buffer, __func__);
       edid_buffer = NULL;
    }

   if (!parsed_edid)
      parsed_edid = get_fallback_hiddev_edid(fd, dev_info);

   DBGMSF(debug, "Returning: %p", parsed_edid);
   return parsed_edid;
}

