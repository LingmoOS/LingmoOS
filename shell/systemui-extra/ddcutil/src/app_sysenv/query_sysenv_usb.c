/** @file query_sysenv_usb.c
 *  Probe the USB environment
 */

// Copyright (C) 2016-2020 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later


#include <config.h>

// #define _GNU_SOURCE 1       // for function group_member

/** \cond */
#include <dirent.h>
#include <errno.h>
#include <glib-2.0/glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
/** \endcond */

#include "util/file_util.h"
#include "util/report_util.h"
#include "util/string_util.h"
#include "util/subprocess_util.h"
#include "util/sysfs_util.h"
#include "util/udev_util.h"
#include "util/udev_usb_util.h"

#include "usb_util/hiddev_reports.h"
#include "usb_util/hiddev_util.h"
#include "usb_util/hidraw_util.h"
#include "usb_util/libusb_util.h"
#include "usb_util/usb_hid_common.h"

#include "base/core.h"
#include "base/ddc_errno.h"

#include "usb/usb_displays.h"

#include "query_sysenv_usb.h"


static bool is_hid_monitor_rdesc(const char * fn) {
   bool debug = false;
   bool result = false;

   char * first_line = file_get_first_line(fn, /*verbose=*/ true);
   DBGMSF(debug, "First line: %s", first_line);
   if ( first_line && str_starts_with( first_line, "05 80"))
      result = true;

   DBGMSF(debug, "fn=%s, returning: %s", fn, sbool(result));
   return result;
}


/* Probe using the UHID debug interface in /sys/kernel/debug/hid
 *
 * For each HID device that's a monitor, displays the HID Report Descriptor.
 *
 * Arguments:
 *    depth     logical indentation depth
 *
 * Returns:     nothing
 */
static void probe_uhid(int depth) {
   int d1 = depth+1;
   int d2 = depth+2;

   bool debug = false;
   DBGMSF(debug, "Starting");

   struct dirent * ep;
   char * dirname = "/sys/kernel/debug/hid/";
   DIR * dp;
   dp = opendir(dirname);
   if (!dp) {
      int errsv = errno;
      rpt_vstring(depth, "Unable to open directory %s: %s", dirname, strerror(errsv));
   }
   else {
      while ( (ep = readdir(dp))) {
         // puts(ep->d_name);
         char fqfn[PATH_MAX];
         if (!streq(ep->d_name, ".") && !streq(ep->d_name, "..") ) {
            // file names look like: "0003:0424:3328:004D"
            // field 1:    ?
            // field 2:    vid
            // field 3:    pid
            // field 4:    appears to be a sequence number of some sort
            //             increases with each call to ddcutil env -v
            snprintf(fqfn, PATH_MAX, "%s%s/rdesc", dirname, ep->d_name);
            // puts(fqfn);
#ifdef FAILS
            puts(ep->d_name);
            printf("(%s) strlen(d_name) = %ld\n", __func__, strlen(ep->d_name));
            uint16_t vid, pid, x, seq = 0;

            int ct = sscanf(ep->d_name, "%hX:%hX:%hX:%hX", &x, &vid, &pid, &seq);
            if (ct != 2)
               printf("(%s) sscanf failed, ct = %d\n", __func__, ct);
            else
               printf("(%s) sscanf ok, vid=0x%04x, pid=0x%04x\n", __func__, vid, pid);
            printf("(%s) x=0x%04x, vid=0x%04x, pid=0x%04x, seq=0x%04x\n", __func__,x,vid,pid,seq);
            // returns ct = 3, seq is unset,  why?
#endif

            char * endptr;
            uint16_t vid = (uint16_t) strtoul(ep->d_name+5,  &endptr, 16);
            uint16_t pid = (uint16_t) strtoul(ep->d_name+10, &endptr, 16);

            bool is_monitor = false;
            if (deny_hid_monitor_by_vid_pid(vid, pid)) {
               rpt_vstring(d1, "Device ignored by vid.pid %d.%d", vid, pid);
            }
            else {
               is_monitor = is_hid_monitor_rdesc(fqfn);
               if (!is_monitor) {
                  is_monitor = force_hid_monitor_by_vid_pid(vid, pid);
               }
               if (is_monitor) {
                  rpt_nl();
                  rpt_vstring(d1, "%s:", fqfn);
                  rpt_file_contents(fqfn, /*verbose=*/true, d2);
               }
            }
         }
      }
      closedir(dp);
   }

   DBGMSF(debug, "Done");
}


/* Probe using the hiddev API
 *
 * Arguments:
 *    depth     logical indentation depth
 *
 * Returns:     nothing
 */
static void probe_hiddev(int depth) {
   int d1 = depth+1;
   int rc;

   rpt_label(depth, "hiddev device names found in file system:");
   GPtrArray * hiddev_devices = get_hiddev_device_names_using_filesys();
   for (int ndx = 0; ndx < hiddev_devices->len; ndx++) {
      rpt_vstring(d1, "%s", (char *) g_ptr_array_index(hiddev_devices,ndx));
   }
   g_ptr_array_free(hiddev_devices, true);

   rpt_label(depth, "hiddev device names found in udev:");
   hiddev_devices = get_hiddev_device_names_using_udev();
   for (int ndx = 0; ndx < hiddev_devices->len; ndx++) {
      rpt_vstring(d1, "%s", (char *) g_ptr_array_index(hiddev_devices,ndx));
   }

   rpt_vstring(depth, "Examining hiddev devices reported by udev...");

   // rpt_vstring(0, "Checking for USB HID devices using hiddev...");
   // GPtrArray * hiddev_devices = get_hiddev_device_names();
   rpt_vstring(depth, "Found %d USB HID devices.", hiddev_devices->len);
   for (int devndx=0; devndx<hiddev_devices->len; devndx++) {
      rpt_nl();
      errno=0;
      char * curfn = g_ptr_array_index(hiddev_devices,devndx);
      if (!is_possible_monitor_by_hiddev_name(curfn)) {
         rpt_vstring(depth,
               "Device %s is not a HID device or is a mouse or keyboard.  Skipping", curfn);
      }
      else {
         int fd = usb_open_hiddev_device(curfn, CALLOPT_RDONLY);    // do not emit error msg
         if (fd < 0) {      // fd is -errno
             rpt_vstring(depth, "Unable to open device %s: %s", curfn, strerror(errno));
             Usb_Detailed_Device_Summary * devsum = lookup_udev_usb_device_by_devname(curfn, true);
             if (devsum) {
                // rpt_vstring(d1, "Detailed device summary for testing: ");
                // report_usb_detailed_device_summary(devsum, d1+1);

                rpt_vstring(d1, "USB bus %s, device %s, vid:pid: %s:%s - %s:%s",
                                devsum->busnum_s,
                                devsum->devnum_s,
                                devsum->vendor_id,
                                devsum->product_id,
                                devsum->vendor_name,
                                devsum->product_name);
                free_usb_detailed_device_summary(devsum);
             }
         }
         else {
             char * cgname = get_hiddev_name(fd);
             struct hiddev_devinfo dev_info;
             errno = 0;
             rc = ioctl(fd, HIDIOCGDEVINFO, &dev_info);
             if (rc != 0) {
                rpt_vstring(d1, "Device %s, unable to retrieve information: %s",
                       curfn, strerror(errno));
             }
             else {
                char dev_summary[200];
                snprintf(dev_summary, 200,
                         "Device %s, devnum.busnum: %d.%d, vid:pid: %04x:%04x - %s",
                         curfn,
                         dev_info.busnum, dev_info.devnum,
                         dev_info.vendor, dev_info.product & 0xffff,
                         cgname);
                rpt_vstring(depth, "%s", dev_summary);
                bool b0 = is_hiddev_monitor(fd);
                if (b0)
                   rpt_vstring(d1, "Identifies as a USB HID monitor");
                else {
                   rpt_vstring(d1, "Not a USB HID monitor");
                   b0 = force_hiddev_monitor(fd);
                   if (b0)
                      rpt_vstring(d1, "Device vid/pid matches exception list.  Forcing report for device.\n");
                }

                if (get_output_level() >= DDCA_OL_VERBOSE) {
                   if (b0) {
                      char * simple_devname = strstr(curfn, "hiddev");
                      Udev_Usb_Devinfo * dinfo = get_udev_usb_devinfo("usbmisc", simple_devname);
                      if (dinfo) {
                         // report_hidraw_devinfo(dinfo, d2);
                         rpt_vstring(
                            d1,"Busno:Devno as reported by get_udev_usb_devinfo() for %s: %03d:%03d",
                               simple_devname, dinfo->busno, dinfo->devno);
                         free(dinfo);
                      }
                      else
                         rpt_vstring(d1, "Error getting busno:devno using get_udev_usb_devinfo()");

                      dbgrpt_hiddev_device_by_fd(fd, d1);
                   }
                }
                if (b0) {
                   Buffer * edid_buffer = hiddev_get_edid(fd);
                   if (edid_buffer) {
                      Parsed_Edid * parsed_edid = create_parsed_edid2(edid_buffer->bytes, "USB");  // copies bytes
                      if (!parsed_edid) {
                         rpt_label(d1, "get_hiddev_edid() returned invalid EDID");
                         // if debug or verbose, dump the bad edid  ??
                         // if (debug || get_output_level() >= OL_VERBOSE) {
                         // }
                      }
                      else {
                         report_parsed_edid(parsed_edid, /*verbose*/ true, d1);
                         free_parsed_edid(parsed_edid);
                      }
                   }
                   else {
                      rpt_label(d1, "Unable to read EDID using hiddev");
                   }
                }


             }
             free(cgname);
         }
         close(fd);
      }
   }

   // n. free function was set at allocation time
   g_ptr_array_free(hiddev_devices, true);
}


/** Master function to query USB aspects of the system environment
  */
void query_usbenv() {
   DDCA_Output_Level output_level = get_output_level();

   rpt_label(0, "The following tests probe for monitors that use USB for VCP communication...");
   rpt_nl();

   rpt_vstring(0, "Checking for USB connected monitors...");
   rpt_nl();

   rpt_vstring(1, "Using lsusb to summarize USB devices...");
   execute_shell_cmd_rpt("lsusb|sort", 2);
   rpt_nl();

   rpt_vstring(1, "USB device topology...");
   execute_shell_cmd_rpt("lsusb -t", 2);
   rpt_nl();

   if (output_level >= DDCA_OL_VERBOSE) {
      rpt_vstring(1, "libusb verbose report...");
      execute_shell_cmd_rpt("lsusb -v", 2);
      rpt_nl();
   }

   rpt_vstring(1, "Listing /dev/usb...");
   execute_shell_cmd_rpt("ls -l /dev/usb", 2);
   rpt_nl();

   rpt_vstring(1, "Listing /dev/hiddev*...");
   execute_shell_cmd_rpt("ls -l /dev/hiddev*", 2);
   rpt_nl();

   rpt_vstring(1, "Listing /dev/bus/usb...");
   execute_shell_cmd_rpt("ls -l /dev/bus/usb", 2);
   rpt_nl();

   rpt_vstring(1, "Listing /dev/hidraw*...");
   execute_shell_cmd_rpt("ls -l /dev/hidraw*", 2);
   rpt_nl();

   rpt_vstring(1, "Searching /sys for hiddev* entries...");
   execute_shell_cmd_rpt("find /sys -name \"hiddev*\"", 2);
   rpt_nl();

   rpt_vstring(1, "Searching /sys for hidraw* entries...");
   execute_shell_cmd_rpt("find /sys -name \"hidraw*\"", 2);
   rpt_nl();

   if (output_level >= DDCA_OL_VERBOSE) {
      char * subsys_name = "usbmisc";
      rpt_nl();
      rpt_vstring(0, "Probing USB HID devices using udev, susbsystem %s...", subsys_name);
      probe_udev_subsystem(subsys_name, /*show_usb_parent=*/ true, 1);

      subsys_name = "hidraw";
      rpt_nl();
      rpt_vstring(0, "Probing USB HID devices using udev, susbsystem %s...", subsys_name);
      probe_udev_subsystem(subsys_name, /*show_usb_parent=*/ true, 1);
   }

#ifdef DISABLE
   if (output_level >= DDCA_OL_VERBOSE) {
      // currently an overwhelming amount of information - need to display
      // only possible HID connected monitors
      rpt_nl();
      rpt_vstring(0, "Probing possible HID monitors using libusb...");
      probe_libusb(/*possible_monitors_only=*/ true, /*depth=*/ 1);

      rpt_nl();
      rpt_vstring(0, "Checking for USB connected monitors on /dev/hidraw* ...");
      probe_hidraw(
            true,    // possible_monitors_only
            1);      // logical indentation depth

       // printf("\nProbing using hidapi...\n");
       // don't use.  wipes out /dev/hidraw  and /dev/usb/hiddev devices it opens
       // no addional information.   Feature values are returned as with libusb -
       // leading byte is report number
       // note that probe_hidapi() tests all possible report numbers, not just those
       // listed in the report descriptor.  Found some additional reports in the
       // vendor specific range on the Apple Cinema display
       // probe_hidapi(1);
   }
#endif

   rpt_nl();
   rpt_vstring(0, "Checking for USB HID devices using hiddev...");
   probe_hiddev(1);

   rpt_nl();
   rpt_vstring(0, "Checking for USB HID Report Descriptors in /sys/kernel/debug/hid...");
   probe_uhid(1);

   rpt_nl();
   rpt_vstring(0, "Checking for USB connected monitors complete");
}

