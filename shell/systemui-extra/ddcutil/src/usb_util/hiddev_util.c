/** @file hiddev_util.c
 */

// Copyright (C) 2016-2021 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later


/** \cond */
#include <config.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <glib-2.0/glib.h>
#include <libudev.h>
#include <linux/hiddev.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "util/coredefs.h"
#include "util/file_util.h"
#include "util/glib_util.h"
#include "util/report_util.h"
#include "util/string_util.h"
#include "util/utilrpt.h"
/** \endcond */

#include "usb_util/usb_hid_common.h"
#include "usb_util/hiddev_reports.h"
#include "usb_util/hiddev_util.h"


static const char* report_type_id_table[] = {
      "invalid value",
      "HID_REPORT_TYPE_INPUT",
      "HID_REPORT_TYPE_OUTPUT",
      "HID_REPORT_TYPE_FEATURE"
};


// To do: converge with similar function hid_report_type_name()
// name hid_report_type_name() is more appropriate since there is nothing hiddev
// specific about this function

/* Returns a string representation of a report type id
 *
 * Arguments:  report_type
 *
 * Returns:  string representation of id
 */
const char * hiddev_report_type_name(__u32 report_type) {
   if (report_type < HID_REPORT_TYPE_MIN || report_type > HID_REPORT_TYPE_MAX)
      report_type = 0;
   return report_type_id_table[report_type];
}


//
// *** Functions to identify hiddev devices representing monitors ***
//

/* Filter to find hiddevN files for scandir() */
static int is_hiddev(const struct dirent *ent) {
   return !strncmp(ent->d_name, "hiddev", strlen("hiddev"));
}


/* Scans /dev to obtain list of hiddev device names
 *
 * Returns:   GPtrArray of device device names.
 */
GPtrArray * get_hiddev_device_names_using_filesys() {
   bool debug = false;
   if (debug) printf("(%s) Starting...\n", __func__);

   const char *hidraw_paths[] = { "/dev/", "/dev/usb/", NULL };
   GPtrArray * dev_names = get_filenames_by_filter(hidraw_paths, is_hiddev);

   if (debug) {
      printf("(%s) Done.     Found %d hiddev devices:\n", __func__, dev_names->len);
      for (int ndx = 0; ndx < dev_names->len; ndx++) {
         printf("              %s\n", (char *) g_ptr_array_index(dev_names,ndx));
      }
   }
   return dev_names;
}


/* Find hiddev device names using udev.
 *
 * Slightly more robust than get_hiddev_device_names_using_filesys() since doesn't
 * make assumptions as to where hiddev devices are found in the /dev tree.
 *
 * Arguments:     none
 * Returns:       array of hiddev path names in /dev
 */
GPtrArray *
get_hiddev_device_names_using_udev() {
   bool debug = false;
   if (debug) printf("(%s) Starting...\n", __func__);

   GPtrArray * dev_names = g_ptr_array_sized_new(10);
   g_ptr_array_set_free_func(dev_names, g_free);

   struct udev *udev;
   struct udev_enumerate *enumerate;
   struct udev_list_entry *
   devices, *dev_list_entry;
   struct udev_device *dev;
   char * subsystem = "usbmisc";   // hiddev devices are in usbmisc subsystem

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
       printf("Can't create udev\n");
       goto bye;
    }

    /* Create a list of the devices in the subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* udev_list_entry_foreach is a macro which expands to
       a loop. The loop will be executed for each member in
       devices, setting dev_list_entry to a list entry
       which contains the device's path in /sys. */
    udev_list_entry_foreach(dev_list_entry, devices) {
       const char *path;

       /* Get the filename of the /sys entry for the device
          and create a udev_device object (dev) representing it */
       path = udev_list_entry_get_name(dev_list_entry);
       dev = udev_device_new_from_syspath(udev, path);
       const char * sysname = udev_device_get_sysname(dev);
       if (str_starts_with(sysname, "hiddev")) {
          g_ptr_array_add(dev_names, strdup(udev_device_get_devnode(dev)));
       }
       udev_device_unref(dev);
    }
    g_ptr_array_sort(dev_names, gaux_ptr_scomp);
    udev_enumerate_unref(enumerate);        // free the enumerator object
    udev_unref(udev);

bye:
   if (debug) {
      printf("(%s) Done.     Found %d hiddev devices:\n", __func__, dev_names->len);
      for (int ndx = 0; ndx < dev_names->len; ndx++) {
         printf("              %s\n", (char *) g_ptr_array_index(dev_names,ndx));
      }
   }
   return dev_names;
}


/* Find hiddev device names.
 *
 * Arguments:     none
 * Returns:       array of hiddev path names in /dev
 *
 * Allows for easily switching between alternative implementations.
 */
GPtrArray * get_hiddev_device_names() {
   // temp, call both for testing
   GPtrArray * result = get_hiddev_device_names_using_udev();
   // g_ptr_array_free(result1, true);
   // GPtrArray * result2 = get_hiddev_device_names_using_filesys();
   return result;
}


/* Check for specific USB devices that should be treated as
 * monitors, even though the normal monitor check fails.
 *
 * This is a hack.
 *
 * Arguments:
 *   fd       file descriptor
 *
 * Returns    true/false
 */
bool force_hiddev_monitor(int fd) {
   bool debug = false;
   bool result = false;

   struct hiddev_devinfo dev_info;

   int rc = ioctl(fd, HIDIOCGDEVINFO, &dev_info);
   if (rc != 0) {
      REPORT_USB_IOCTL_ERROR("HIDIOCGDEVINFO", errno);
      goto bye;
   }

   result = force_hid_monitor_by_vid_pid( dev_info.vendor, dev_info.product);

bye:
   if (debug)
      printf("(%s) vid=0x%04x, pid=0x%04x, returning: %s\n", __func__,
             dev_info.vendor, dev_info.product, sbool(result));
   return result;
}


/* Check if an open hiddev device represents a USB compliant monitor.
 *
 * Arguments:
 *    fd       file descriptor
 *
 * Returns:    true/false
 *
 * Per USB Monitor Control Class Specification section 5.5,
 * "to identify a HID class device as a monitor, the devices's
 * HID Report Descriptor must contain a top-level collection with
 * a usage of Monitor Control from the USB Monitor Usage Page."
*/
bool is_hiddev_monitor(int fd) {
   bool debug = false;
   if (debug)
      printf("(%s) Starting\n", __func__);

   int monitor_collection_index = -1;
   int cndx = 0;   // indexes start at 0
   int ioctl_rc = 0;
   for (cndx=0; ioctl_rc != -1; cndx++) {
      struct hiddev_collection_info  cinfo;
      memset(&cinfo, 0, sizeof(cinfo));
      errno = 0;
      cinfo.index = cndx;
      ioctl_rc = ioctl(fd, HIDIOCGCOLLECTIONINFO, &cinfo);
      if (ioctl_rc == -1)
         continue;
      assert(ioctl_rc == 0);
      if (debug)
         printf("(%s) cndx=%d, cinfo.level=%d, cinfo.usage=0x%08x\n", __func__,
                cndx, cinfo.level, cinfo.usage);
      if (cinfo.level == 0 && cinfo.usage == 0x00800001) { // USB Monitor Usage Page/Monitor Control
         monitor_collection_index = cndx;
         break;
      }
   }
   bool result = (monitor_collection_index >= 0);

   // FOR TESTING
   // if (!result)
   //    result = force_hiddev_monitor(fd);

   if (debug)
      printf("(%s) Returning: %s\n", __func__, sbool(result));
   return result;
}


/* Checks that all usages of a field have the same usage code.
 *
 * Arguments:
 *   fd            file descriptor of open hiddev device
 *   finfo         pointer to hiddev_field_info struct describing the field
 *   field_index   actual field index, value in finfo may have been changed by
 *                 HIDIOCGFIELDINFO call, that filled in hiddev_field_info
 *
 * Returns:        usage code if all are identical, 0 if not
 */
__u32 hiddev_get_identical_ucode(int fd, struct hiddev_field_info * finfo, __u32 field_index) {
   // assert(finfo->flags & HID_FIELD_BUFFERED_BYTE);
   __u32 result = 0;

   for (int undx = 0; undx < finfo->maxusage; undx++) {
      struct hiddev_usage_ref uref = {
          .report_type = finfo->report_type,
          .report_id   = finfo->report_id,
          .field_index = field_index,         // actual field index, not value changed by HIDIOCGFIELDINFO
          .usage_index = undx
      };
      // printf("(%s) report_type=%d, report_id=%d, field_index=%d, usage_index=%d\n",
      //       __func__, rinfo->report_type, rinfo->report_id, field_index=saved_field_index, undx);
      int rc = ioctl(fd, HIDIOCGUCODE, &uref);    // Fills in usage code
      if (rc != 0) {
          REPORT_USB_IOCTL_ERROR("HIDIOCGUCODE", errno);
          result = 0;
          break;
      }
      // printf("(%s) uref.field_index=%d, uref.usage_code = 0x%08x\n",
      //        __func__, uref.field_index, uref.usage_code);
      if (undx == 0)
         result = uref.usage_code;
      else if (uref.usage_code != result) {
         result = 0;
         break;
      }
   }   // loop over usages

   return result;
}


/* Collects all the usage values for a field and returns them in a Buffer.
 *
 * The field must meet the following requirements:
 *   All usages must have the same usage code
 *   All values must be single byte
 *
 * The function should only be called for INPUT and FEATURE reports.  (assertion check)
 *
 * This function assumes that HIDIOCGREPORT has already been called
 *
 * Arguments:
 *   fd           file descriptor
 *   finfo        pointer to filled in hiddev_field_info for the field
 *   field_index  actual field index to use
 *
 * Returns:   Buffer with accumulated value,
 *            NULL if multiple usage codes or some usage value is > 0xff
 *            It is the responsibility of the caller to free the returned buffer.
 */
Buffer * hiddev_collect_single_byte_usage_values(
            int                         fd,
            struct hiddev_field_info *  finfo,
            __u32                       field_index)
{
   bool debug = false;
   Buffer * result = buffer_new(finfo->maxusage, __func__);
   bool ok = true;
   __s32 common_usage_code;

   // n.b. assumes HIDIOCGREPORT has been called

   assert(finfo->report_type != HID_REPORT_TYPE_OUTPUT);

   // bound is < finfo->maxusage, not <= finfo->maxusage
   // undx == finfo->maxusage returns errno=22, invalid argument
   for (int undx = 0; undx < finfo->maxusage; undx++) {
       struct hiddev_usage_ref uref = {
           .report_type = finfo->report_type,   // rinfo.report_type;
           .report_id =   finfo->report_id,     // rinfo.report_id;
           .field_index = field_index,   // use original value, not value changed by HIDIOCGFIELDINFO
           .usage_index = undx
       };
       // printf("(%s) report_type=%d, report_id=%d, field_index=%d, maxusage=%d, usage_index=%d\n",
       //       __func__, finfo->report_type, finfo->report_id, finfo->field_index, finfo->maxusage, undx);
       int rc = ioctl(fd, HIDIOCGUCODE, &uref);    // Fills in usage code
       if (rc != 0) {
           REPORT_USB_IOCTL_ERROR("HIDIOCGUCODE", errno);
           ok = false;
           break;
       }
       // printf("(%s) uref.field_index=%d, uref.usage_code = 0x%08x\n",
       //        __func__, uref.field_index, uref.usage_code);
       if (undx == 0) {
          common_usage_code = uref.usage_code;
       }
       else if (uref.usage_code != common_usage_code) {
          ok = false;
          if (debug)
             printf("(%s) Multiple usage codes", __func__);
          break;
       }

       rc = ioctl(fd, HIDIOCGUSAGE, &uref);  // Fills in usage value
       if (rc != 0) {
          REPORT_USB_IOCTL_ERROR("HIDIOCGUSAGE", errno);
          ok = false;
          break;
       }
       if (uref.value &0xffffff00) {     // if high order bytes non-zero
          if (true)
             printf("(%s) High order bytes of value for usage %d are non-zero\n", __func__, undx);
          ok = false;
          break;
       }
       Byte b = uref.value;
       buffer_add(result, b);
       if (false)
          printf("(%s) usage = %d, value=0x%08x, byte = 0x%02x\n",
                 __func__, undx, uref.value, uref.value&0xff);
    }   // loop over usages


   if (!ok && result) {
      buffer_free(result, __func__);
      result = NULL;
   }

   if (debug) {
      printf("(%s) Returning: %p\n", __func__, result);
      // if (result)
      //    buffer_dump(result);
   }
   return result;
}


//
// *** Functions for EDID retrieval ***
//

/* Checks if a field in a HID report represents an EDID
 *
 * Arguments:
 *    fd           file descriptor for open hiddev device
 *    rinfo        pointer to hiddev_report_info struct
 *    field_index  index number of field to check
 *
 * Returns:        true if the field represents an EDID, false if not
 *
 * The field must have at least 128 usages, and the usage code for each must
 * be USB Monitor/EDID information
 */
bool hiddev_is_field_edid(int fd, struct hiddev_report_info * rinfo, int field_index) {
   bool debug = false;
   if (debug)
      printf("(%s) report_type=%d, report_id=%d, field index = %d\n",
             __func__, rinfo->report_type, rinfo->report_id, field_index);

   bool all_usages_edid = false;
   int rc;

   struct hiddev_field_info finfo = {
      .report_type = rinfo->report_type,
      .report_id   = rinfo->report_id,
      .field_index = field_index
   };

   int saved_field_index = field_index;
   rc = ioctl(fd, HIDIOCGFIELDINFO, &finfo);
   if (rc != 0)
      REPORT_USB_IOCTL_ERROR("HIDIOCGFIELDINFO", errno);
   assert(rc == 0);
   if (debug) {
      if (finfo.field_index != saved_field_index && debug) {
         printf("(%s) !!! ioctl(HIDIOCGFIELDINFO) changed field_index from %d to %d\n",
                __func__, saved_field_index, finfo.field_index);
         printf("(%s)   rinfo.num_fields=%d, finfo.maxusage=%d\n",
                __func__, rinfo->num_fields, finfo.maxusage);
      }
   }

   if (finfo.maxusage >= 128)
      all_usages_edid = ( hiddev_get_identical_ucode(fd, &finfo, field_index) == 0x00800002 );

   if (debug)
      printf("(%s) Returning: %s", __func__, sbool(all_usages_edid));
   return all_usages_edid;
}



#define EDID_SIZE 128


//
// hid_field_locator functions
//

void free_hid_field_locator(struct hid_field_locator * location) {
   if (location) {
      if (location->finfo)
         free(location->finfo);
      free(location);
   }
}


void report_hid_field_locator(struct hid_field_locator * ploc, int depth) {
   int d1 = depth+1;

   rpt_structure_loc("struct hid_field_locator", ploc, depth);
   if (ploc) {
      rpt_vstring(d1, "%-20s %u", "report_type:",  ploc->report_type );
      rpt_vstring(d1, "%-20s %u", "report_id:",  ploc->report_id );
      rpt_vstring(d1, "%-20s %u", "field_index:",  ploc->field_index);
      // report_hiddev_report_info(ploc->rinfo, d1);
      dbgrpt_hiddev_field_info(ploc->finfo, d1);
   }
}


/* Test if all, or alternatively at least 1, usage codes of a field match a specified usage code.
 *
 * Arguments:
 *   fd                 file descriptor of open hiddev device
 *   report_type        HID_REPORT_TYPE_INPUT, HID_REPORT_TYPE_OUTPUT, or HID_REPORT_TYPE_FEATURE
 *   report_id          report number
 *   field_index        field index
 *   ucode              usage code to test against
 *   require_all_match  if true, all usages must be the specified value
 *
 * Returns:    field information if true, NULL if false
 */
struct hiddev_field_info *
test_field_ucode(
   int    fd,
   __u32  report_type,
   __u32  report_id,
   __u32  field_index,
   __u32  ucode,
   bool   require_all_match)
{
   bool debug = false;
   if (debug)
      printf("(%s) report_type=%d, report_id=%d, field index=%d, ucode=0x%08x, require_all_match=%s\n",
             __func__, report_type, report_id, field_index, ucode, sbool(require_all_match));

   struct hiddev_field_info * result = NULL;
   int rc;

   struct hiddev_field_info finfo = {
        .report_type = report_type,
        .report_id   = report_id,
        .field_index = field_index
   };

   int saved_field_index = field_index;
   rc = ioctl(fd, HIDIOCGFIELDINFO, &finfo);
   if (rc != 0) {
      REPORT_USB_IOCTL_ERROR("HIDIOCGFIELDINFO", errno);
      goto bye;
   }
   if (debug) {
      if (finfo.field_index != saved_field_index && debug)
         printf("(%s) !!! ioctl(HIDIOCGFIELDINFO) changed field_index from %d to %d\n",
                __func__, saved_field_index, finfo.field_index);
   }
   // result->field_id = fndx;

   bool is_matched = false;
   if (require_all_match) {
      __u32 ucode_found = hiddev_get_identical_ucode(fd, &finfo, field_index);

      if (ucode_found == ucode)
         is_matched = true;
   }
   else {
      // loop over all usage codes
      int undx;
      for (undx = 0; undx < finfo.maxusage; undx++) {
         struct hiddev_usage_ref uref = {
            .report_type = report_type,
            .report_id =   report_id,
            .field_index = field_index,
            .usage_index = undx
         };
         rc = ioctl(fd, HIDIOCGUCODE, &uref);    // Fills in usage code
         if (rc != 0) {
            REPORT_USB_IOCTL_ERROR("HIDIOCGUCODE", errno);
            break;
         }
         if (uref.usage_code == ucode) {
            is_matched = true;
            break;
         }
      }   // loop over usages
   }
   if (is_matched) {
      result = malloc(sizeof(struct hiddev_field_info));
      memcpy(result, &finfo, sizeof(struct hiddev_field_info));
   }

bye:
   if (debug) {
      printf("(%s) Returning: %p\n", __func__, result);
      if (result)
         dbgrpt_hiddev_field_info(result, 1);
   }

   return result;
}


/* Look through all reports of a given type (HID_REPORT_TYPE_FEATURE, etc) to
 * find one having a field with a given usage code.
 *
 * Arguments:
 *   fd                file descriptor of open hiddev device
 *   report_type       HID_REPORT_TYPE_INPUT, HID_REPORT_TYPE_OUTPUT, or HID_REPORT_TYPE_FEATURE
 *   ucode             usage code
 *   match_all_ucodes  if true, all usages of the field must match ucode
 *                     if false, at least one usage must match ucode
 *
 * Returns:            record identifying the report and field
 */
struct hid_field_locator*
hiddev_find_report(int fd, __u32 report_type, __u32 ucode, bool match_all_ucodes) {
   bool debug = false;
   if (debug)
      printf("(%s) Starting.  report_type=%d, ucode=0x%08x, match_all_ucodes=%s\n",
            __func__, report_type, ucode, sbool(match_all_ucodes));

   struct hid_field_locator * result = NULL;

   struct hiddev_report_info rinfo = {
      .report_type = report_type,
      .report_id   = HID_REPORT_ID_FIRST
   };

   int report_id_found   = -1;
   int field_index_found = -1;
   struct hiddev_field_info * finfo_found = NULL;
   int reportinfo_rc = 0;
   //while (reportinfo_rc == 0 && report_id_found == -1) {
   while (reportinfo_rc == 0 && !finfo_found) {
      // printf("(%s) Report counter %d, report_id = 0x%08x %s\n",
      //       __func__, rptct, rinfo.report_id, interpret_report_id(rinfo.report_id));

      errno = 0;
      reportinfo_rc = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
      if (reportinfo_rc != 0) {    // no more reports
         if (reportinfo_rc != -1)
            REPORT_USB_IOCTL_ERROR("HIDIOCGREPORTINFO", errno);
         break;
      }

      for (int fndx = 0; fndx < rinfo.num_fields && field_index_found == -1; fndx++) {
         // finfo_found = is_field_edid(fd, &rinfo, fndx);
         // *** TEMP *** FORCE FAILURE
         finfo_found = test_field_ucode(fd, report_type, rinfo.report_id, fndx, ucode, match_all_ucodes);
         if (finfo_found) {
            report_id_found    = rinfo.report_id;
            field_index_found  = fndx;
            break;
         }
      }

      rinfo.report_id |= HID_REPORT_ID_NEXT;
   }  // loop over reports


   // assert( (report_id_found >=  0 && finfo_found  ) ||
   //         (report_id_found == -1 && !finfo_found ) );

   if (finfo_found) {
   // if (report_id_found >= 0) {
      result = calloc(1, sizeof(struct hid_field_locator));
      // result->rinfo = calloc(1, sizeof(struct hiddev_report_info));
      // memcpy(result->rinfo, &rinfo, sizeof(struct hiddev_report_info));
      result->finfo       = finfo_found;   // returned by is_field_edid() or test_field_ucode()
      result->report_type = rinfo.report_type;
      result->report_id   = report_id_found;
      result->field_index = field_index_found;    // finfo.field_index may have been changed by HIDIOGREPORTINFO
   }

   if (debug) {
      if (result) {
         printf("(%s) Returning: %p\n", __func__, result);
         printf("(%s)    report_id=%d, field_index=%d\n",
                __func__, result->report_id, result->field_index);
         // report_hid_field_locator(result, 1);
      }
      else
         printf("(%s) Returning NULL", __func__);
   }
   return result;
}


/* Finds the report describing the EDID.
 *
 * Arguments:
 *   fd          file descriptor of open hiddev device
 *
 * Returns:      pointer to newly allocated struct hid_field_locator representing
 *               the feature report and field within that report that returns
 *               the EDID,
 *               NULL if not found
 *
 * It is the responsibility of the caller to free the returned struct.
 */
struct hid_field_locator *
locate_edid_report(int fd) {
   bool debug = false;

   struct hid_field_locator* result = NULL;
   result = hiddev_find_report(fd, HID_REPORT_TYPE_FEATURE, 0x00800002, /*match_all_ucodes=*/true);
   if (result) {
      // find_report() should have parm specifying minimum number of usages
      if (result->finfo->maxusage < 128) {
         printf("(%s) Located report contains less than 128 usages.  Discarding.\n", __func__);
         free_hid_field_locator(result);
         result = NULL;
      }
   }

   if (debug) {
      if (result) {
         printf("(%s) Returning %p report_id=%d, field_index=%d\n",
                __func__, result, result->report_id, result->field_index);
         // report_hid_field_locator(result, 1);
      }
      else
         printf("(%s) Returning NULL", __func__);
   }

   return result;
}


//
// Get multibyte value
//

/* Base function for retrieving a multibyte field value using a
 * call to HIDIOCGUSAGES.  The value to be retrieved is specified
 * using a struct hiddev_usage_ref_multi.
 *
 * Arguments:
 *    fd           file descriptor of open hiddev device
 *    uref_multi   specifies value to be retrieve, and buffer
 *                 in which to return bytes
 *
 * Returns:
 *    pointer to Buffer struct containing the value, NULL if error
 *
 * It is the responsibility of the caller to free the returned buffer.
 */
Buffer *
get_multibyte_value_by_uref_multi(
      int                             fd,
      struct hiddev_usage_ref_multi * uref_multi
     )
{
   bool debug = false;
   if (debug) {
      printf("(%s) Starting. fd=%d, uref_multi=%p\n",  __func__, fd, uref_multi);
      dbgrpt_hiddev_usage_ref_multi(uref_multi, 1);
   }
   int      rc;
   Buffer * result = NULL;

#ifndef NDEBUG
   __u32 report_type = uref_multi->uref.report_type;
   assert(report_type == HID_REPORT_TYPE_FEATURE ||
          report_type == HID_REPORT_TYPE_INPUT);   // *** CG19 ***
#endif

   rc = ioctl(fd, HIDIOCGUSAGES, uref_multi);  // Fills in usage value
   if (rc != 0) {
      if (errno == EINVAL) {
         if (debug)
            printf("(%s) ioctl(HIDIOCGUSAGES) returned EINVAL\n", __func__);
      }
      else
         REPORT_USB_IOCTL_ERROR("HIDIOCGUSAGES", errno);
      goto bye;
   }
   result = buffer_new(uref_multi->num_values, __func__);
   for (int ndx=0; ndx<uref_multi->num_values; ndx++)
      buffer_add(result, uref_multi->values[ndx] & 0xff);

bye:
   if (debug) {
      printf("Returning: %p\n", result);
      if (result) {
         buffer_dump(result);
      }
   }
   return result;
}


/* Retrieve the bytes of a multibyte field value using a
 * call to HIDIOCGUSAGES.  The field to be retrieved is
 * specified using a struct hid_field_locator.
 *
 * Arguments:
 *    fd     file descriptor of open hiddev device
 *    loc    pointer to hid_field_locator struct
 *
 * Returns:
 *    pointer to Buffer struct containing the value, NULL if error
 *
 * It is the responsibility of the caller to free the returned buffer.
 */
Buffer * hiddev_get_multibyte_report_value_by_hid_field_locator(
      int                        fd,
      struct hid_field_locator * loc)
{
   bool debug = false;
   if (debug) {
      printf("(%s) Starting.  hid_field_locator=%p\n", __func__, loc);
      // report_hid_field_locator(loc, 1);
   }
   Buffer * result = NULL;

   struct hiddev_report_info rinfo;
   rinfo.report_type = loc->report_type;
   rinfo.report_id   = loc->report_id;
   rinfo.num_fields  = 0;       // initialize to avoid valgrind warning

   int rc = ioctl(fd, HIDIOCGREPORT, &rinfo);
   if (rc != 0) {
      if (errno == EINVAL) {
         if (debug)
            printf("(%s) ioctl(HIDIOCGUSAGES) returned EINVAL\n", __func__);
      }
      else
         REPORT_USB_IOCTL_ERROR("HIDIOCGREPORT", errno);
      goto bye;
   }

   struct hiddev_usage_ref_multi uref_multi;
   memset(&uref_multi, 0, sizeof(uref_multi));  // initialize all fields to make valgrind happy
   uref_multi.uref.report_type = loc->report_type;
   uref_multi.uref.report_id   = loc->report_id;
   uref_multi.uref.field_index = loc->field_index;
   uref_multi.uref.usage_index = 0;
   uref_multi.uref.usage_code  = 0;
   uref_multi.num_values       = loc->finfo->maxusage;

   result = get_multibyte_value_by_uref_multi(fd, &uref_multi);

bye:
   if (debug)
      printf("(%s) Returning: %p\n", __func__, result);
   return result;
}


/* Retrieve the bytes of a multibyte field value using a call to HIDIOCGUSAGES.
 * It is left to hiddev to determine the correct report of the specified report
 * type to use to get the value of the specified usage code.
 *
 * Arguments:
 *    fd           file descriptor of open hiddev device
 *    report_type  HID_REPORT_TYPE_FEATURE or HID_REPORT_TYPE_INPUT
 *    usage_code   usage code to retrieve
 *    num_values   number of bytes
 *
 * Returns:
 *    pointer to Buffer struct containing the value, NULL if error
 *
 * It is the responsibility of the caller to free the returned buffer.
 */
Buffer *
hiddev_get_multibyte_value_by_report_type_and_ucode(
      int   fd,
      __u32 report_type,
      __u32 usage_code,
      __u32 num_values)
{
   bool debug = false;
   if (debug)
      printf("(%s) Starting. fd=%d, report_type=%d, usage_code=0x%08x, num_values=%d\n",
                 __func__, fd, report_type, usage_code, num_values);
   Buffer * result = NULL;

   assert(report_type == HID_REPORT_TYPE_FEATURE || report_type == HID_REPORT_TYPE_INPUT);

   struct hiddev_usage_ref_multi uref_multi;
   memset(&uref_multi, 0, sizeof(uref_multi));  // initialize all fields to make valgrind happy
   uref_multi.uref.report_type = report_type;
   uref_multi.uref.report_id   = HID_REPORT_ID_UNKNOWN;
   uref_multi.uref.usage_code  = usage_code;
   uref_multi.num_values       = num_values; // needed? yes!

   result = get_multibyte_value_by_uref_multi(fd, &uref_multi);

   if (debug)
      printf("(%s) Returning: %p\n", __func__, result);
   return result;
}


/* Retrieve the bytes of a multibyte field value using a call to HIDIOCGUSAGES.
 * It is left to hiddev to determine the correct report to use to get the value
 * of the specified usage code.  Both Feature and Input reports are tried.
 *
 * This function first tries to get a value using a feature report.
 * If that fails, an input report is tried.
 *
 * Arguments:
 *    fd           file descriptor of open hiddev device
 *    usage_code   usage code to retrieve
 *    num_values   number of bytes
 *
 * Returns:
 *    pointer to Buffer struct containing the value
 *
 * It is the responsibility of the caller to free the returned buffer.
 */
Buffer *
hiddev_get_multibyte_value_by_ucode(int fd,__u32 usage_code, __u32 num_values) {
   Buffer * result = hiddev_get_multibyte_value_by_report_type_and_ucode(
                        fd, HID_REPORT_TYPE_FEATURE, usage_code, num_values);
    if (!result)
            result = hiddev_get_multibyte_value_by_report_type_and_ucode(
                        fd, HID_REPORT_TYPE_INPUT, usage_code, num_values);
    return result;
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
Buffer * hiddev_get_edid(int fd)  {
   bool debug = false;
   if (debug)
      printf("(%s) Starting\n", __func__);

   Buffer * result = NULL;
   struct hid_field_locator * loc = locate_edid_report(fd);
   if (loc) {
      result = hiddev_get_multibyte_report_value_by_hid_field_locator(fd, loc);
      free_hid_field_locator(loc);
   }

   if (debug)
      printf("(%s) Returning: %p\n", __func__, result);
   return result;
}


//
// *** Miscellaneous functions ***
//

/* Returns the name of a hiddev device, as reported by ioctl HIDIOCGNAME.
 *
 * Arguments:
 *    fd         file descriptor of open hiddev device
 *
 * Returns:      pointer to newly allocated string,
 *               NULL if ioctl call fails (should never happen)
 */
char * get_hiddev_name(int fd) {
   // printf("(%s) Starting. fd=%d\n", __func__, fd);
   const int blen = 256;
   char buf1[blen];
   for (int ndx=0; ndx < blen; ndx++)
   buf1[ndx] = '\0';   // initialize to avoid valgrind warning
   // returns length of result, including terminating null
   int rc = ioctl(fd, HIDIOCGNAME(blen), buf1);
   // printf("(%s) HIDIOCGNAME returned %d\n", __func__, rc);
   // hex_dump(buf1,64);
   char * result = NULL;
   if (rc >= 0)
      result = strdup(buf1);
   // printf("(%s) Returning |%s|\n", __func__, result);
   return result;
}
