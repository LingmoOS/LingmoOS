/** \file dumpload.c
 *
 * Load/store VCP settings from/to file.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

/** \cond */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <glib-2.0/glib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util/file_util.h"
#include "util/glib_util.h"
#include "util/glib_string_util.h"
#include "util/report_util.h"

#include "ddcutil_types.h"
/** \endcond */

#include "base/core.h"
#include "base/ddc_errno.h"
#include "base/ddc_packets.h"
#include "base/displays.h"
#include "base/monitor_model_key.h"
#include "base/parms.h"
#include "base/rtti.h"
#include "base/status_code_mgt.h"
#include "base/vcp_version.h"

#include "dynvcp/dyn_feature_codes.h"

#include "i2c/i2c_bus_core.h"

#include "ddc/ddc_displays.h"
#include "ddc/ddc_display_selection.h"
#include "ddc/ddc_output.h"
#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_read_capabilities.h"
#include "ddc/ddc_vcp.h"
#include "ddc/ddc_vcp_version.h"

#include "ddc/ddc_dumpload.h"


/** Frees a #Dumpload_Data struct.  The underlying Vcp_Value_set is also freed.
 *
 * @param data    pointer to #Dumpload_Data struct to free,\n
 *                if NULL, do nothing
 */
void free_dumpload_data(Dumpload_Data * data) {
   bool debug = false;
   DBGMSF(debug, "Starting. data=%p", data);
   if (data) {
      if (data->vcp_values)
         free_vcp_value_set(data->vcp_values);
      free(data);
   }
   DBGMSF(debug, "Done.");
}


/** Reports the contents of a #Dumpload_Data struct
 *
 *  @param  data     pointer to #Dumpload_Data struct
 *  @param  depth    logical indentation depth
 */
void dbgrpt_dumpload_data(Dumpload_Data * data, int depth) {
   int d1 = depth+1;
   rpt_structure_loc("Dumpload_Data", data, depth);
   // rptIval("busno", NULL, data->busno, d1);
   // TODO: timestamp_millis
   // TODO: show abbreviated edidstr
   rpt_str( "mfg_id",       NULL, data->mfg_id,       d1);
   rpt_str( "model",        NULL, data->model,        d1);
   rpt_unsigned(
           " product_code", NULL, data->product_code, d1);
   rpt_str( "serial_ascii", NULL, data->serial_ascii, d1);
   rpt_str( "edid",         NULL, data->edidstr,      d1);
   rpt_str( "vcp_version",  NULL, format_vspec(data->vcp_version), d1);
   rpt_int( "vcp_value_ct", NULL, data->vcp_value_ct, d1);
   rpt_structure_loc("vcp_values", data->vcp_values, d1);
   if (data->vcp_values)
      dbgrpt_vcp_value_set(data->vcp_values, d1);
}

#define ADD_DATA_ERROR(_expl) \
      errinfo_add_cause(  \
         errs,            \
         errinfo_new(    \
            DDCRC_BAD_DATA, __func__, \
            _expl " at line %d: %s", linectr, line) );


/** Given an array of strings stored in a GPtrArray,
 *  convert it a #Dumpload_Data struct.
 *
 *  @param   garray      array of strings
 *  @return  pointer to newly allocated Dumpload_Data struct, or
 *                NULL if the data is not valid.
 *                It is the responsibility of the caller to free this struct.
 */
Error_Info *
create_dumpload_data_from_g_ptr_array(
      GPtrArray * garray,
      Dumpload_Data ** dumpload_data_loc)
{
   bool debug = false;
   DBGMSF(debug, "Starting.");

   Error_Info * errs = errinfo_new(DDCRC_BAD_DATA, __func__, NULL);
   *dumpload_data_loc = NULL;

   Dumpload_Data * data = calloc(1, sizeof(Dumpload_Data));
#ifndef NDEBUG
   bool valid_data = true;
#endif
   // default:
   data->vcp_version.major = 2;
   data->vcp_version.minor = 0;
   data->vcp_values = vcp_value_set_new(15);      // 15 = initial size

   int     ct;
   int     linectr = 0;

   while ( linectr < garray->len ) {
      char *  line = NULL;
      char    s0[32], s1[257], s2[16];
      char *  head;
      char *  rest;

      line = g_ptr_array_index(garray,linectr);
      linectr++;

      *s0 = '\0'; *s1 = '\0'; *s2 = '\0';
      head = line;
      while (*head == ' ') head++;
      ct = sscanf(head, "%31s %256s %15s", s0, s1, s2);
      if (ct > 0 && *s0 != '*' && *s0 != '#') {
         if (ct == 1) {
            // printf("Invalid data at line %d: %s\n", linectr, line);
            // Error_Info * err = errinfo_new2(
            //                       DDCRC_BAD_DATA, __func__,
            //                       "Invalid data at line %d: %s", linectr, line);
            // errinfo_add_cause(errs, err);
            ADD_DATA_ERROR("Invalid data");
#ifndef NDEBUG
            valid_data = false;
#endif
         }
         else {
            rest = head + strlen(s0);;
            while (*rest == ' ') rest++;
            char * last = rest + strlen(rest) - 1;
            // we already parsed a second token, so don't need to worry that last becomes < head
            while (*last == ' ' || *last == '\n') {
               *last-- = '\0';
            }
            // DBGMSG("rest=|%s|", rest );

            if (streq(s0, "BUS")) {
               // ignore
               // ct = sscanf(s1, "%d", &data->busno);
               // if (ct == 0) {
               //    fprintf(stderr, "Invalid bus number at line %d: %s\n", linectr, line);
               //    valid_data = false;
               // }
            }
            else if (streq(s0, "PRODUCT_CODE")) {
               int ival;
               bool ok = str_to_int(s1, &ival, 10);
               if (ok && ival >= 0 && ival <= 65535)
                  data->product_code = (uint16_t) ival;
#ifndef NDEBUG
               else
                  valid_data = false;
#endif
            }
            else if (streq(s0, "EDID") || streq(s0, "EDIDSTR")) {
               STRLCPY(data->edidstr, s1, sizeof(data->edidstr));
            }
            else if (streq(s0, "MFG_ID")) {
               STRLCPY(data->mfg_id, s1, sizeof(data->mfg_id));
            }
            else if (streq(s0, "MODEL")) {
               STRLCPY(data->model, rest, sizeof(data->model));
            }
            else if (streq(s0, "SN")) {
               STRLCPY(data->serial_ascii, rest, sizeof(data->serial_ascii));
            }
            else if (streq(s0, "VCP_VERSION")) {
               data->vcp_version = parse_vspec(s1);
               // using VCP_SPEC_UNKNOWN as value when invalid,
               // what if monitor had no version, so 0.0 was output?
               if ( vcp_version_eq( data->vcp_version, DDCA_VSPEC_UNKNOWN) ) {
                  // f0printf(ferr(), "Invalid VCP VERSION at line %d: %s\n", linectr, line);
                  // errinfo_add_cause(
                  //            errs,
                  //            errinfo_new2(
                  //                  DDCRC_BAD_DATA, __func__,
                  //                  "Invalid VCP VERSION at line %d: %s\n", linectr, line) );
                  ADD_DATA_ERROR("Invalid VCP VERSION");
#ifndef NDEBUG
                  valid_data = false;
#endif
               }
            }
            else if (streq(s0, "TIMESTAMP_TEXT")   ||
                     streq(s0, "TIMESTAMP_MILLIS")

                    ) {
               // do nothing, just recognize valid field
            }
            else if (streq(s0, "VCP")) {
               if (ct != 3) {
                  // f0printf(ferr(), "Invalid VCP data at line %d: %s\n", linectr, line);
                  ADD_DATA_ERROR("Invalid VCP data");
#ifndef NDEBUG
                  valid_data = false;
#endif
               }
               else {   // found feature id and value
                  Byte feature_id;
                  bool ok = hhs_to_byte_in_buf(s1, &feature_id);
                  if (!ok) {
                     // f0printf(ferr(), "Invalid opcode at line %d: %s", linectr, s1);
                     ADD_DATA_ERROR("Invalid  opcode");
#ifndef NDEBUG
                     valid_data = false;
#endif
                  }
                  else {     // valid opcode
                     DDCA_Any_Vcp_Value * valrec = NULL;
                     // look up opcode, is it valid?

                     // table values need special handling

                     // Problem: without VCP version, can't look up feature in
                     // VCP code table and definitively know if it's a table feature.
                     // One solution: rework data structures to parse later
                     // second solution: vcp version in dumpload data

                     DDCA_Monitor_Model_Key mmk = monitor_model_key_value(
                           data->mfg_id, data->model, data->product_code);

                     Display_Feature_Metadata * dfm =
                                              dyn_get_feature_metadata_by_mmk_and_vspec(
                                                   feature_id,
                                                   mmk,
                                                   data->vcp_version,
                                                   /*with_default=*/ true);
                     bool is_table_feature = dfm->feature_flags & DDCA_NORMAL_TABLE;

                     if (is_table_feature) {
                        // s2 is hex string
                        Byte * ba;
                        int bytect =  hhs_to_byte_array(s2, &ba);
                        if (bytect < 0) {
                           // f0printf(ferr(),
                           //          "Invalid hex string value for opcode at line %d: %s\n",
                           //          linectr, line);
                           ADD_DATA_ERROR("Invalid hex string value for opcode");
#ifndef NDEBUG
                           valid_data = false;
#endif
                        }
                        else {
                           valrec = create_table_vcp_value_by_bytes(
                                 feature_id,
                                 ba,
                                 bytect);
                           free(ba);
                        }
                     }
                     else {   // non-table feature
                        ushort feature_value;
                        ct = sscanf(s2, "%hu", &feature_value);
                        if (ct == 0) {
                           // f0printf(ferr(), "Invalid value for opcode at line %d: %s\n", linectr, line);
                           ADD_DATA_ERROR("Invalid value for opcode");
#ifndef NDEBUG
                           valid_data = false;
#endif
                        }
                        else {
                           // good opcode and value
                           // TODO: opcode and value should be saved in local vars
                           valrec = create_cont_vcp_value(
                              feature_id,
                              0,   // max_val, unused for LOADVCP
                              feature_value);
                        }
                     }   // non-table feature
                     if (valrec) {
                        data->vcp_value_ct++;
                        vcp_value_set_add(data->vcp_values, valrec);
                     }
                     dfm_free(dfm);
                  } // valid opcode

               } // found feature id and value
            }  // VCP

            else {
               // f0printf(ferr(), "Unexpected field \"%s\" at line %d: %s\n", s0, linectr, line );
               errinfo_add_cause(
                          errs,
                          errinfo_new(
                                DDCRC_BAD_DATA, __func__,
                                "Unexpected field \"%s\" at line %d: %s", s0, linectr, line) );
#ifndef NDEBUG
               valid_data = false;
#endif
            }
         }    // more than 1 field on line
      }       // non-comment line
   }          // one line of file

   assert( ( valid_data && errs->cause_ct == 0 ) ||
           (!valid_data && errs->cause_ct >  0) );
   if (errs->cause_ct == 0) {
      errinfo_free(errs);
      errs = NULL;
   }
   else {
      if (data) {
         free_dumpload_data(data);
         data = NULL;
      }
   }
   *dumpload_data_loc = data;
   return errs;
}

#undef ADD_DATA_ERROR


/** Sets multiple VCP values.
 *
 * @param   dh      display handle
 * @param   vset    values to set
 * @return  #Ddc_Error reflecting the first error, or NULL if no errors
 *
 * This function stops applying values on the first error encountered, and
 * returns the value of that error as its status code.
 *
 * @remark
 * Consider not stopping on error, instead accumulate errors in Error_Info.
 */
Error_Info *
ddc_set_multiple(
      Display_Handle* dh,
      Vcp_Value_Set   vset)
{
   Public_Status_Code psc = 0;
   Error_Info *        ddc_excp = NULL;
   int value_ct = vcp_value_set_size(vset);

   int ndx;
   for (ndx=0; ndx < value_ct; ndx++) {
      DDCA_Any_Vcp_Value * vrec
      = vcp_value_set_get(vset, ndx);
      Byte   feature_code = vrec->opcode;

      // HACK: will this affect intermittent error of silently failing sets?
      // pointless, ddc_it2_write_only) calls call_tuned_sleep() after write
      // if (ndx > 0) {
      //    sleep_millis_with_trace(DDC_TIMEOUT_MILLIS_DEFAULT, __func__, "before set_vcp_value()");
      // }

      ddc_excp = ddc_set_vcp_value(dh, vrec, NULL);
      psc = (ddc_excp) ? ddc_excp->status_code : 0;
      if (ddc_excp) {
         f0printf(ferr(), "Error setting value for VCP feature code 0x%02x: %s\n",
                         feature_code, psc_desc(psc) );
         if (psc == DDCRC_RETRIES)
            f0printf(ferr(), "    Try errors: %s\n", errinfo_causes_string(ddc_excp));
         f0printf(ferr(), "Terminating.");
         break;
      }

   } // for loop

   return ddc_excp;
}


/** Applies VCP settings from a #Dumpload_Data struct to
 *  the monitor specified in that data structure.
 *
 * @param  pdata      pointer to #Dumpload_Data instance
 * @param  dh         display handle for open display,
 *                    if NULL, open the find the display based on the identifiers
 *                    in the data and open it
 *
 * @return   #Ddc_Error describing the first error, or NULL if no error
 */
Error_Info *
loadvcp_by_dumpload_data(
      Dumpload_Data *   pdata,
      Display_Handle *  dh)
{
   assert(pdata);
   FILE * errf = ferr();

   bool debug = false;
   if (debug) {
        DBGMSG("Loading VCP settings for monitor \"%s\", sn \"%s\", dh=%p \n",
               pdata->model, pdata->serial_ascii, dh);
        dbgrpt_dumpload_data(pdata, 0);
   }

   Public_Status_Code psc = 0;
   Error_Info * ddc_excp = NULL;
   Display_Handle * dh_argument = dh;

   if (dh) {
      // If explicit display specified, check that the data is valid for it
      assert(dh->dref->pedid);
      bool ok = true;
      if ( !streq(dh->dref->pedid->model_name, pdata->model) ) {
         f0printf(errf,
            "Monitor model in data (%s) does not match that for specified device (%s)\n",
            pdata->model, dh->dref->pedid->model_name);
         ok = false;
      }
      if (!streq(dh->dref->pedid->serial_ascii, pdata->serial_ascii) ) {
         f0printf(errf,
            "Monitor serial number in data (%s) does not match that for specified device (%s)\n",
            pdata->serial_ascii, dh->dref->pedid->serial_ascii);
         ok = false;
      }
      if (!ok) {
         psc = DDCRC_INVALID_DISPLAY;
         goto bye;
      }
   }

   else if ( strlen(pdata->mfg_id) + strlen(pdata->model) + strlen(pdata->serial_ascii) == 0) {
      // Pathological.  Someone's been messing with the VCP file.
      f0printf(errf, "Monitor manufacturer id, model, and serial number all missing from input.\n");
      psc = DDCRC_INVALID_DISPLAY;
      goto bye;
   }

   else {
     // no Display_Ref passed as argument, just use the identifiers in the data to pick the display
      Display_Identifier * did = create_mfg_model_sn_display_identifier(
                             pdata->mfg_id,
                             pdata->model,
                             pdata->serial_ascii);
      assert(did);
      Display_Ref * dref = get_display_ref_for_display_identifier(
                              did, CALLOPT_NONE);
      free_display_identifier(did);
      if (!dref) {
         f0printf(errf, "Monitor not connected: %s - %s   \n", pdata->model, pdata->serial_ascii );
         psc = DDCRC_INVALID_DISPLAY;
         goto bye;
      }

      // return code == 0 iff dh set
      ddc_open_display(dref, CALLOPT_ERR_MSG, &dh);
      if (!dh) {
         psc = DDCRC_INVALID_DISPLAY;
         goto bye;
      }
   }

   ddc_excp = ddc_set_multiple(dh, pdata->vcp_values);
   psc = (ddc_excp) ? ddc_excp->status_code : 0;

   // close the display only if this function opened it
   if (!dh_argument)
      ddc_close_display(dh);

bye:
   DBGMSF(debug, "Returning: %s", psc_desc(psc));
   if (psc == DDCRC_RETRIES && debug)
      DBGMSG("        Try errors: %s", errinfo_causes_string(ddc_excp));
   return ddc_excp;;
}


/** Reads the monitor identification and VCP values from a null terminated
 *  string array and applies those values to the selected monitor.
 *
 *  \param  ntsa    null terminated array of strings
 *  \param  dh      display handle
 *  \return pointer to #Ddc_Error describing the first error, NULL if if success
 */
Error_Info *
loadvcp_by_ntsa(
      Null_Terminated_String_Array ntsa,
      Display_Handle *             dh)
{
   bool debug = false;

   DDCA_Output_Level output_level = get_output_level();
   bool verbose = (output_level >= DDCA_OL_VERBOSE);
   // DBGMSG("output_level=%d, verbose=%d", output_level, verbose);
   if (debug) {
      DBGMSG("Starting.  ntsa=%p", ntsa);
      verbose = true;
   }
   // Public_Status_Code psc = 0;
   Error_Info * ddc_excp = NULL;

   GPtrArray * garray = ntsa_to_g_ptr_array(ntsa);

   Dumpload_Data * pdata = NULL;
   ddc_excp = create_dumpload_data_from_g_ptr_array(garray, &pdata);
   DBGMSF(debug, "create_dumpload_data_from_g_ptr_array() returned %p", pdata);
   assert( (ddc_excp == NULL && pdata != NULL) ||
           (ddc_excp != NULL && pdata == NULL) );
   if (!pdata) {
      // f0printf(ferr(), "Unable to load VCP data from string\n");
      // psc = DDCRC_ARG;     // was DDCRC_INVALID_DATA;
      // ddc_excp = errinfo_new(psc, __func__);
   }
   else {
      if (verbose) {
           f0printf(fout(), "Loading VCP settings for monitor \"%s\", sn \"%s\" \n",
                           pdata->model, pdata->serial_ascii);
           rpt_push_output_dest(fout());
           dbgrpt_dumpload_data(pdata, 0);
           rpt_pop_output_dest();
      }
      ddc_excp = loadvcp_by_dumpload_data(pdata, dh);
      free_dumpload_data(pdata);
   }
   return ddc_excp;
}


/** Reads the monitor identification and VCP values from a single string
 *  whose fields are separated by ';' and applies those values to the
 *  selected monitor.
 *
 *  \param   catenated    data string
 *  \param   dh           display handle
 *  \return  pointer to #Ddc_Error describing first error, NULL if success
 */
// n. called from ddct_public:
Error_Info *
loadvcp_by_string(
      char *           catenated,
      Display_Handle * dh)
{
   Null_Terminated_String_Array nta = strsplit(catenated, ";");
   Error_Info * ddc_excp = loadvcp_by_ntsa(nta, dh);

   ntsa_free(nta, /* free_strings */ true);
   return ddc_excp;
}


//
// DUMPVCP
//

//
// Support for the DUMPVCP command and for returning profile info in the API
//

/** Formats a timestamp in a way usable in a filename, specifically:
 *    YYYMMDD-HHMMSS
 *
 *  \param  time_millis   timestamp in milliseconds
 *  \param  buf           buffer in which to return the formatted timestamp
 *  \param  bufsz         buffer size
 *  \return formatted timestamp
 *
 * \remark
 * If buf == NULL or bufsz == 0, then this function allocates a buffer.
 * It is the responsibility of the caller to free this buffer.
 */
char *
format_timestamp(time_t time_millis, char * buf, int bufsz) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "buf=%p, bufsz=%d", buf, bufsz);
   if (bufsz == 0 || buf == NULL) {
      bufsz = 128;
      buf = calloc(1, bufsz);
      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "Allocated new buffer: %p", buf);
   }
   struct tm tm = *localtime(&time_millis);
   snprintf(buf, bufsz, "%4d%02d%02d-%02d%02d%02d",
                  tm.tm_year+1900,
                  tm.tm_mon+1,
                  tm.tm_mday,
                  tm.tm_hour,
                  tm.tm_min,
                  tm.tm_sec
                 );
   DBGTRC_DONE(debug, DDCA_TRC_NONE, "Returning: %p", buf);
   return buf;
}


#ifdef UNUSED
/* Returns monitor identification information in an array of strings.
 * The strings are written in the format of the DUMPVCP command.
 *
 * Arguments:
 *    dh       display handle for monitor
 *    vals     GPtrArray to which the identification strings are appended.
 *
 * Returns:  nothing
 */
void collect_machine_readable_monitor_id(Display_Handle * dh, GPtrArray * vals) {
   char buf[400];
   int bufsz = sizeof(buf)/sizeof(char);

   Parsed_Edid * edid = ddc_get_parsed_edid_by_dh(dh);
   snprintf(buf, bufsz, "MFG_ID  %s",  edid->mfg_id);
   g_ptr_array_add(vals, strdup(buf));
   snprintf(buf, bufsz, "MODEL   %s",  edid->model_name);
   g_ptr_array_add(vals, strdup(buf));
   snprintf(buf, bufsz, "SN      %s",  edid->serial_ascii);
   g_ptr_array_add(vals, strdup(buf));

   char hexbuf[257];
   hexstring2(edid->bytes, 128,
              NULL /* no separator */,
              true /* uppercase */,
              hexbuf, 257);
   snprintf(buf, bufsz, "EDID    %s", hexbuf);
   g_ptr_array_add(vals, strdup(buf));
}
#endif


/** Appends TIMESTAMP_TEXT and TIMESTAMP_MILLIS lines to an array of strings.
 *  The strings are written in the format of the DUMPVCP command.
 *
 *  \param  time_millis timestamp in milliseconds
 *  \param  vals        GPtrArray to which the timestamp strings are appended.
 */
void
collect_machine_readable_timestamp(time_t time_millis, GPtrArray* vals) {
   bool debug = false;
   DBGTRC_STARTING(debug, DDCA_TRC_NONE, "");
   // temporarily use same output format as filename, but format the
   // date separately here for flexibility
   char timestamp_buf[30];
   // n. since buffer provided, format_timestamp() does not allocate
   format_timestamp(time_millis, timestamp_buf, sizeof(timestamp_buf));
   char buf[400];
   int bufsz = sizeof(buf)/sizeof(char);
   snprintf(buf, bufsz, "TIMESTAMP_TEXT %s", timestamp_buf );
   g_ptr_array_add(vals, strdup(buf));

   // time_t is problematic.  %jd handles x32_abi where time_t is of type long long
   // but per the c11 spec time_t can be a real type
   // Value is ignored on input, so just don't output it and avoid the issues. (11/2018)
   // snprintf(buf, bufsz, "TIMESTAMP_MILLIS %jd", time_millis);
   // g_ptr_array_add(vals, strdup(buf));

   DBGTRC_DONE(debug, DDCA_TRC_NONE, "Appended %s", buf);
}

#ifdef UNUSED
// for completeness, currently unused
/* Save profile related information (timestamp, monitor identification,
 * VCP values) in external form in a GPtrArray of string.
 *
 * Arguments:
 *    dh           display handle
 *    time_millis  timestamp in milliseconds
 *    pvals        Location of newly allocated GptrArray
 *
 * Returns:
 *    status code
 *
 * It is the responsibility of the caller to free the newly allocated
 * GPtrArray.
 */
Public_Status_Code
collect_profile_related_values(
      Display_Handle*  dh,
      time_t           timestamp_millis,
      GPtrArray**      pvals)
{
   bool debug = false;
   DBGMSF(debug, "Starting");
   assert( get_output_level() == OL_PROGRAM);
   Public_Status_Code psc = 0;
   GPtrArray * vals = g_ptr_array_sized_new(50);

   collect_machine_readable_timestamp(timestamp_millis, vals);
   collect_machine_readable_monitor_id(dh, vals);
   psc = show_vcp_values(
            dh,
            VCP_SUBSET_PROFILE,
            vals,
            false /* force_show_unsupported */);
   *pvals = vals;
   if (debug) {
      DBGMSG("Done.  *pvals->len=%d *pvals: ", vals->len);
      int ndx = 0;
      for (;ndx < vals->len; ndx++) {
         DBGMSG("  |%s|", g_ptr_array_index(vals,ndx) );
      }
   }
   return psc;
}
#endif


/** Primary function for the DUMPVCP command.
 *
 *  Writes DUMPVCP data to the in-core Dumpload_Data structure
 *
 *  \param   dh                 display handle for connected display
 *  \param   dumpload_data_loc  address at which to return pointer to newly allocated
 *                             Dumpload_Data struct.  It is the responsibility of the
 *                             caller to free this data structure.
 *  \return status code
 */
Public_Status_Code
dumpvcp_as_dumpload_data(
      Display_Handle * dh,
      Dumpload_Data** dumpload_data_loc)
{
   bool debug = false;
   DBGMSF(debug, "Starting. dh=%s", dh_repr(dh));
   Public_Status_Code psc = 0;
   Dumpload_Data * dumped_data = calloc(1, sizeof(Dumpload_Data));

   // timestamp:
   dumped_data->timestamp_millis = time(NULL);

   DBGMSF(debug, "Before get_vcp_version_by_dh()");
   dumped_data->vcp_version = get_vcp_version_by_dh(dh);  // use function to ensure set

   // identification information from edid:
   // Parsed_Edid * edid = ddc_get_parsed_edid_by_dh(dh);
   Parsed_Edid * edid = dh->dref->pedid;
   assert(edid);
   DBGMSF(debug, "Have EDID");

   dumped_data->product_code = edid->product_code;
   memcpy(dumped_data->mfg_id, edid->mfg_id, sizeof(dumped_data->mfg_id));
   memcpy(dumped_data->model,  edid->model_name, sizeof(dumped_data->model));
   memcpy(dumped_data->serial_ascii, edid->serial_ascii, sizeof(dumped_data->serial_ascii));
   memcpy(dumped_data->edidbytes, edid->bytes, 128);
   assert(sizeof(dumped_data->edidstr) == 257);
   hexstring2(edid->bytes, 128,
              NULL /* no separator */,
              true /* uppercase */,
              dumped_data->edidstr, 257);

   // VCP values
   Vcp_Value_Set vset = vcp_value_set_new(50);
   psc = ddc_collect_raw_subset_values(
             dh,
             VCP_SUBSET_PROFILE,
             vset,
             true,               //  ignore_unsupported
             ferr());
   if (psc == 0) {
      dumped_data->vcp_values = vset;             // NOTE REFERENCE, BE CAREFUL WHEN FREE
      dumped_data->vcp_value_ct = vcp_value_set_size(vset);
   }

   if (psc != 0 && dumped_data)
      free(dumped_data);
   else
      *dumpload_data_loc = dumped_data;

   if (debug) {
      DBGMSG("Returning: %s, *pdumpload_data=%p", psc_desc(psc), *dumpload_data_loc);
      dbgrpt_dumpload_data(*dumpload_data_loc, 1);
   }
   return psc;
}


/** Converts a Dumpload_Data structure to an array of strings
 *
 *  \param data     pointer to Dumpload_Data instance
 *  \return         array of strings
 *
 * \remark
 * Note that the result shares no memory with data
 */
GPtrArray *
convert_dumpload_data_to_string_array(Dumpload_Data * data) {
   bool debug = false;
   DBGMSF(debug, "Starting. data=%p", data);
   assert(data);
   if (debug)
      dbgrpt_dumpload_data(data, 1);

   GPtrArray * strings = g_ptr_array_sized_new(30);
   g_ptr_array_set_free_func(strings, g_free);

   collect_machine_readable_timestamp(data->timestamp_millis, strings);

   char buf[300];
   int bufsz = sizeof(buf)/sizeof(char);
   snprintf(buf, bufsz, "MFG_ID  %s",  data->mfg_id);
   g_ptr_array_add(strings, strdup(buf));
   snprintf(buf, bufsz, "MODEL   %s",  data->model);
   g_ptr_array_add(strings, strdup(buf));
   snprintf(buf, bufsz, "PRODUCT_CODE  %d", data->product_code);
   g_ptr_array_add(strings, strdup(buf));
   snprintf(buf, bufsz, "SN      %s",  data->serial_ascii);
   g_ptr_array_add(strings, strdup(buf));

   char hexbuf[257];
   hexstring2(data->edidbytes, 128,
              NULL /* no separator */,
              true /* uppercase */,
              hexbuf, 257);
   snprintf(buf, bufsz, "EDID    %s", hexbuf);
   g_ptr_array_add(strings, strdup(buf));

   if (!vcp_version_eq(data->vcp_version, DDCA_VSPEC_UNKNOWN)) {
      snprintf(buf, bufsz, "VCP_VERSION %d.%d", data->vcp_version.major, data->vcp_version.minor);
      g_ptr_array_add(strings, strdup(buf));
   }

   for (int ndx=0; ndx < data->vcp_values->len; ndx++) {
      // n. get_formatted_value_for_feature_table_entry() also has code for table type values
      DDCA_Any_Vcp_Value * vrec = vcp_value_set_get(data->vcp_values,ndx);
      char buf[200];
      snprintf(buf, 200, "VCP %02X %5d",
                         vrec->opcode,
                         VALREC_CUR_VAL(vrec));
      g_ptr_array_add(strings, strdup(buf));
   }
   return strings;
}


/** Returns the output of the DUMPVCP command in a single string.
 *  Each field is separated by a semicolon.
 *
 *  The caller is responsible for freeing the returned string.
 *
 *  \param  dh          display handle of open monitor
 *  \param  result_loc  location at which to return string
 *  \return status code
 */
// n. called from api_feature_access.c
Public_Status_Code
dumpvcp_as_string(Display_Handle * dh, char ** result_loc) {
   bool debug = false;
   DBGMSF(debug, "Starting, dh=%s", dh_repr(dh));

   Public_Status_Code psc    = 0;
   Dumpload_Data *    data   = NULL;
   *result_loc = NULL;

   psc = dumpvcp_as_dumpload_data(dh, &data);
   if (psc == 0) {
      GPtrArray * strings = convert_dumpload_data_to_string_array(data);
      *result_loc = join_string_g_ptr_array(strings, ";");
      free_dumpload_data(data);
   }
   DBGMSF(debug, "Returning: %s, *result_loc=|%s|", psc_desc(psc), *result_loc);
   return psc;
}


void init_ddc_dumpload() {
   RTTI_ADD_FUNC(format_timestamp);
   RTTI_ADD_FUNC(collect_machine_readable_timestamp);
}
