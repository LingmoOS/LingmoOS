/** @file ddc_vcp.c
 *  Virtual Control Panel access
 *  Basic functions to get and set single values and save current settings.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <config.h>

/** \cond */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/error_info.h"
#include "util/report_util.h"
#include "util/string_util.h"
#include "util/utilrpt.h"
/** \endcond */

#include "base/ddc_errno.h"
#include "base/ddc_packets.h"
#include "base/displays.h"
#include "base/rtti.h"
#include "base/status_code_mgt.h"

#include "i2c/i2c_bus_core.h"

#ifdef USE_USB
#include "usb/usb_displays.h"
#include "usb/usb_vcp.h"
#endif

#include "vcp/vcp_feature_codes.h"

#include <dynvcp/dyn_feature_codes.h>

#include "ddc/ddc_multi_part_io.h"
#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_vcp_version.h"

#include "ddc/ddc_vcp.h"

// Trace class for this file
static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_DDC;

typedef struct {
   bool   verify_setvcp;
} Thread_Vcp_Settings;

static Thread_Vcp_Settings * get_thread_vcp_settings() {
   static GPrivate per_thread_key = G_PRIVATE_INIT(g_free);

   Thread_Vcp_Settings *settings = g_private_get(&per_thread_key);

   // GThread * this_thread = g_thread_self();
   // printf("(%s) this_thread=%p, settings=%p\n", __func__, this_thread, settings);

   if (!settings) {
      settings = g_new0(Thread_Vcp_Settings, 1);
      settings->verify_setvcp = false;     // set by g_new0(), but be explicit

      g_private_set(&per_thread_key, settings);
   }

   // printf("(%s) Returning: %p\n", __func__, settings);
   return settings;
}


//
//  Save Control Settings
//

/** Executes the DDC Save Control Settings command.
 *
 * @param  dh handle of open display device
 * @return NULL if success, pointer to #Error_Info if failure
 */
Error_Info *
ddc_save_current_settings(
      Display_Handle * dh)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
          "Invoking DDC Save Current Settings command. dh=%s", dh_repr(dh));

   Error_Info * ddc_excp = NULL;
   if (dh->dref->io_path.io_mode == DDCA_IO_USB) {
      // command line parser should block this case
      PROGRAM_LOGIC_ERROR("MCCS over USB does not have Save Current Settings command");
      ddc_excp = errinfo_new(DDCRC_UNIMPLEMENTED, __func__,
                              "MCCS over USB does not have Save Current Settings command" );
   }
   else {
      DDC_Packet * request_packet_ptr =
         create_ddc_save_settings_request_packet("save_current_settings:request packet");
      // DBGMSG("create_ddc_save_settings_request_packet returned packet_ptr=%p", request_packet_ptr);
      // dump_packet(request_packet_ptr);

      ddc_excp = ddc_write_only_with_retry(dh, request_packet_ptr);

      if (request_packet_ptr)
         free_ddc_packet(request_packet_ptr);
   }

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   return ddc_excp;
}


//
// Set VCP feature value
//

/** Sets a non-table VCP feature value.
 *
 *  @param  dh            display handle for open display
 *  @param  feature_code  VCP feature code
 *  @param  new_value     new value
 *  @return NULL if success,
 *          pointer to #Error_Info from #ddc_write_only_with_retry() if failure
 */
Error_Info *
ddc_set_nontable_vcp_value(
      Display_Handle * dh,
      Byte             feature_code,
      int              new_value)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
          "Writing feature 0x%02x , new value = %d, dh=%s",
          feature_code, new_value, dh_repr(dh) );

   Public_Status_Code psc = 0;
   Error_Info * ddc_excp = NULL;
   if (dh->dref->io_path.io_mode == DDCA_IO_USB) {
#ifdef USE_USB
      psc = usb_set_nontable_vcp_value(dh, feature_code, new_value);
#else
      PROGRAM_LOGIC_ERROR("ddcutil not built with USB support");
#endif
   }
   else {
      DDC_Packet * request_packet_ptr =
         create_ddc_setvcp_request_packet(feature_code, new_value, "set_vcp:request packet");
      // DBGMSG("create_ddc_getvcp_request_packet returned packet_ptr=%p", request_packet_ptr);
      // dump_packet(request_packet_ptr);

      ddc_excp = ddc_write_only_with_retry(dh, request_packet_ptr);
      psc = (ddc_excp) ? ddc_excp->status_code : 0;

      if (request_packet_ptr)
         free_ddc_packet(request_packet_ptr);
   }

   if ( psc==DDCRC_RETRIES )
      DBGTRC_NOPREFIX(debug, TRACE_GROUP, "Try errors: %s", errinfo_causes_string(ddc_excp));  // needed?
   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   return ddc_excp;
}


/** Sets a table VCP feature value.
 *
 *  @param  dh            display handle for open display
 *  @param  feature_code  VCP feature code
 *  @param  bytes         pointer to table bytes
 *  @param  bytect        number of bytes
 *  @return NULL  if success
 *          DDCRC_UNIMPLEMENTED if io mode is USB
 *          #Error_Info from #multi_part_write_with_retry() otherwise
 */
Error_Info *
set_table_vcp_value(
      Display_Handle *  dh,
      Byte              feature_code,
      Byte *            bytes,
      int               bytect)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "Writing feature 0x%02x , bytect = %d",
                                       feature_code, bytect);

   Public_Status_Code psc = 0;
   Error_Info * ddc_excp = NULL;
   if (dh->dref->io_path.io_mode == DDCA_IO_USB) {
#ifdef USE_USB
      psc = DDCRC_UNIMPLEMENTED;
#else
      PROGRAM_LOGIC_ERROR("ddcutil not built with USB support");
      psc = DDCRC_INTERNAL_ERROR;
#endif
      ddc_excp = errinfo_new(psc, __func__, "Error setting feature value");
   }
   else {
      // TODO: clean up function signatures
      // pointless wrapping in a Buffer just to unwrap
      Buffer * new_value = buffer_new_with_value(bytes, bytect, __func__);

      ddc_excp = multi_part_write_with_retry(dh, feature_code, new_value);
      psc = (ddc_excp) ? ddc_excp->status_code : 0;

      buffer_free(new_value, __func__);
   }

   if ( psc == DDCRC_RETRIES )
      DBGTRC_NOPREFIX(debug, TRACE_GROUP, "Try errors: %s", errinfo_causes_string(ddc_excp));
   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   return ddc_excp;
}


/** Sets the setvcp verification setting for the current thread.
 *
 *  If enabled, setvcp will read the feature value from the monitor after
 *  writing it, to ensure the monitor has actually changed the feature value.
 *
 *  @param onoff  **true** for enabled, **false** for disabled.
 *  @return prior setting
 */
bool
ddc_set_verify_setvcp(bool onoff) {
   bool debug = false;
   DBGMSF(debug, "Setting verify_setvcp = %s", sbool(onoff));

   Thread_Vcp_Settings * settings = get_thread_vcp_settings();
   bool old_value = settings->verify_setvcp;
   settings->verify_setvcp = onoff;
   return old_value;
}


/** Gets the current setvcp verification setting for the current thread.
 *
 *  @return **true** if setvcp verification enabled\n
 *          **false** if not
 */
bool
ddc_get_verify_setvcp() {
   Thread_Vcp_Settings * settings = get_thread_vcp_settings();
   return settings->verify_setvcp;
}


static bool
is_rereadable_feature(
      Display_Handle * dh,
      DDCA_Vcp_Feature_Code opcode)
{
   bool debug = false;
   DBGMSF(debug, "Starting opcode = 0x%02x", opcode);

   // readable features that should not be read after write
   DDCA_Vcp_Feature_Code unrereadable_features[] = {
         0x02,        // new control value
         0x03,        // soft controls
         0x60,        // input source - for some monitors it is meaningful to read the new 
                      //                value, others won't respond if set to a different input
   };

   bool result = true;
   for (int ndx = 0; ndx < ARRAY_SIZE(unrereadable_features); ndx++) {
      if ( unrereadable_features[ndx] == opcode ) {
          result = false;
          DBGMSF(debug, "Unreadable opcode 0x%02x", opcode);
          break;
      }
   }

   if (result) {
      Display_Feature_Metadata * dfm =
            dyn_get_feature_metadata_by_dh(opcode, dh, /*with_default*/false);
      // if not found, assume readable  ??
      if (dfm) {
         result = dfm->feature_flags & DDCA_READABLE;
         dfm_free(dfm);
      }
   }

   DBGMSF(debug, "Returning: %s", sbool(result));
   return result;
}


static bool
is_unreadable_sl_value(
      DDCA_Vcp_Feature_Code opcode,
      Byte                  sl_value)
{
   bool debug = false;
   DBGMSF(debug, "opcode=0x%02x, sl_value=0x%02x", opcode, sl_value);

   bool result = false;
   switch(opcode) {
   case 0xd6:
      if (sl_value == 5)        // turn off display, trying to read from it will fail
         result = true;
      break;
   default:
      break;
   }

   DBGMSF(debug, "Done.     Returning: %s", sbool(result));
   return result;
}


static bool
single_vcp_value_equal(
      DDCA_Any_Vcp_Value * vrec1,
      DDCA_Any_Vcp_Value * vrec2)
{
   assert(vrec1 && vrec2);  // no implementation for degenerate cases
   bool debug = false;

   bool result = false;
   if (vrec1->opcode     == vrec2->opcode &&
       vrec1->value_type == vrec2->value_type)
   {
      switch(vrec1->value_type) {
      case(DDCA_NON_TABLE_VCP_VALUE):
            // only check SL byte which would be set for any VCP, monitor
      result = (vrec1->val.c_nc.sl == vrec2->val.c_nc.sl);
            break;
      case(DDCA_TABLE_VCP_VALUE):
            result = (vrec1->val.t.bytect == vrec2->val.t.bytect) &&
                     (memcmp(vrec1->val.t.bytes, vrec2->val.t.bytes, vrec1->val.t.bytect) == 0 );
      }
   }

   DBGMSF(debug, "Returning: %s", sbool(result));
   return result;
}


// TODO: Consider wrapping set_vcp_value() in set_vcp_value_with_retry(), which would
// retry in case verification fails

/** Sets a VCP feature value.
 *
 *  @param  dh            display handle for open display
 *  @param  vrec          pointer to value record
 *  @param  newval_loc    if non-null, address at which to return verified value
 *  @return NULL if success, pointer to #Error_Info if failure
 *
 *  If write verification is turned on, reads the feature value after writing it
 *  to ensure the display has actually changed the value.
 *
 * The caller is responsible for freeing the value returned at **newval_loc**.
 *  @remark
 *  If verbose messages are in effect, writes detailed messages to the current
 *  stdout device.
 */
Error_Info *
ddc_set_vcp_value(
      Display_Handle *    dh,
      DDCA_Any_Vcp_Value *  vrec,
      DDCA_Any_Vcp_Value ** newval_loc)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "");

   FILE * verbose_msg_dest = fout();
   if ( get_output_level() < DDCA_OL_VERBOSE && !debug )
      verbose_msg_dest = NULL;

   Error_Info * ddc_excp = NULL;
   if (newval_loc)
      *newval_loc = NULL;
   if (vrec->value_type == DDCA_NON_TABLE_VCP_VALUE) {
      ddc_excp = ddc_set_nontable_vcp_value(dh, vrec->opcode, VALREC_CUR_VAL(vrec));
   }
   else {
      assert(vrec->value_type == DDCA_TABLE_VCP_VALUE);
      ddc_excp = set_table_vcp_value(dh, vrec->opcode, vrec->val.t.bytes, vrec->val.t.bytect);
   }

   if (!ddc_excp && ddc_get_verify_setvcp()) {
      Public_Status_Code psc = 0;
      if ( is_rereadable_feature(dh, vrec->opcode) &&
           ( vrec->value_type != DDCA_NON_TABLE_VCP_VALUE ||
             !is_unreadable_sl_value(vrec->opcode, vrec->val.c_nc.sl)
           )
         )
      {
         f0printf(verbose_msg_dest, "Verifying that value of feature 0x%02x successfully set...\n", vrec->opcode);
         DDCA_Any_Vcp_Value * newval = NULL;
         ddc_excp = ddc_get_vcp_value(
             dh,
             vrec->opcode,
             vrec->value_type,
             &newval);
         psc = (ddc_excp) ? ddc_excp->status_code : 0;
         if (ddc_excp) {
            f0printf(verbose_msg_dest, "(%s) Read after write failed. get_vcp_value() returned: %s\n",
                           __func__, psc_desc(psc));
            if (psc == DDCRC_RETRIES)
               f0printf(verbose_msg_dest, "(%s)    Try errors: %s\n", __func__, errinfo_causes_string(ddc_excp));
            // psc = DDCRC_VERIFY;
         }
         else {
            assert(vrec && newval);    // silence clang complaint
            // dbgrpt_ddca_single_vcp_value(vrec, 2);
            // dbgrpt_ddca_single_vcp_value(newval, 3);

            if (! single_vcp_value_equal(vrec,newval)) {
               ddc_excp = errinfo_new(DDCRC_VERIFY, __func__, "Current value does not match value set");
               f0printf(verbose_msg_dest, "Current value does not match value set.\n");
            }
            else {
               f0printf(verbose_msg_dest, "Verification succeeded\n");
            }
            if (newval_loc)
               *newval_loc = newval;
            else
               free_single_vcp_value(newval);
         }
      }
      else {
         if (!is_rereadable_feature(dh, vrec->opcode) )
            f0printf(verbose_msg_dest, "Feature 0x%02x does not support verification\n", vrec->opcode);
         else
            f0printf(verbose_msg_dest, "Feature 0x%02x, value 0x%02x does not support verification\n",
                                       vrec->opcode,
                                       vrec->val.c_nc.sl);
      }
   }

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   return ddc_excp;
}


/** Possibly returns a mock value for a non-table feature
 *
 *  @param  feature_code  VCP Feature Code
 *  @param  resp_loc      where to return pointer to pseudo-response,
 *                        NULL if no pseudo-response
 *  @return pseudo error information, if simulating a failure
 *
 *  @remark
 *  If return NULL and *resp_loc == NULL, then no pseudo response was generated
 */
Error_Info *
mock_get_nontable_vcp_value(
      DDCA_Vcp_Feature_Code   feature_code,
      Parsed_Nontable_Vcp_Response** resp_loc)
{
   bool debug = false;
   DBGMSF(debug, "Starting. feature_code = 0x%02x, resp_loc = %p",
                 feature_code, resp_loc);
   Error_Info * pseudo_errinfo = NULL;
   *resp_loc = NULL;

#ifdef TEST_X72
   if (feature_code == 0x72) {
      Parsed_Nontable_Vcp_Response * resp = calloc(1, sizeof(Parsed_Nontable_Vcp_Response));
      resp->vcp_code = feature_code;
      resp->valid_response = true;
      resp->supported_opcode = true;
      resp->mh = 0;
      resp->ml = 0;
      resp->sh = 0x78;
      resp->sl = 0x00;
      resp->max_value = resp->mh << 8 | resp->ml;
      resp->cur_value = resp->sh << 8 | resp->sl;

      *resp_loc = resp;
   }
#endif

#ifdef TEST_EIO
   if (feature_code == 0x45) {
      pseudo_errinfo = errinfo_new(-EIO, __func__, "Pseudo EIO error");
   }
#endif

   if (debug) {
      DBGMSG("Feature 0x%02x, *resp_loc = %p, returning: %s",
          feature_code,
          *resp_loc,
          errinfo_summary(pseudo_errinfo) );
      if (*resp_loc)
         dbgrpt_interpreted_nontable_vcp_response(*resp_loc, 2);
   }
   return pseudo_errinfo;
}


//
// Get VCP values
//

/** Gets the value for a non-table feature.
 *
 *  @param  dh                   handle for open display
 *  @param  feature_code         VCP feature code
 *  @param  parsed_response_loc  where to return parsed response
 *  @return NULL if success, pointer to #Error_Info if failure
 *
 * It is the responsibility of the caller to free the parsed response.
 *
 * The value pointed to by parsed_response_loc is non-null iff the returned value is null.
 */
Error_Info *
ddc_get_nontable_vcp_value(
       Display_Handle *               dh,
       DDCA_Vcp_Feature_Code          feature_code,
       Parsed_Nontable_Vcp_Response** parsed_response_loc)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "dh=%s, Reading feature 0x%02x", dh_repr(dh), feature_code);

   Error_Info * excp = NULL;
   Parsed_Nontable_Vcp_Response * parsed_response = NULL;
   *parsed_response_loc = NULL;

   Error_Info * mock_errinfo = mock_get_nontable_vcp_value(feature_code, parsed_response_loc);
   if (mock_errinfo || *parsed_response_loc) {
      DBGMSF(debug, "Returning mock response for feature 0x%02x", feature_code);
      return mock_errinfo;
   }

   DDC_Packet * request_packet_ptr  = NULL;
   DDC_Packet * response_packet_ptr = NULL;
   request_packet_ptr = create_ddc_getvcp_request_packet(
                           feature_code, "ddc_get_nontable_vcp_value:request packet");
   // dump_packet(request_packet_ptr);

   Byte expected_response_type = DDC_PACKET_TYPE_QUERY_VCP_RESPONSE;
   Byte expected_subtype = feature_code;
   // int max_read_bytes  = 20;    // actually 3 + 8 + 1, or is it 2 + 8 + 1?
   // expected response size:
   //  (src addr == x6e) (length) (response contents) (checkbyte)
   //  1               + 1      + 8                 + 1           == 11
   //  alternative is DDC Null Response, which is shorter
   //  N. response does not include initial destination address byte of DDC/CI spec
   int max_read_bytes = 11;

   excp = ddc_write_read_with_retry(
           dh,
           request_packet_ptr,
           max_read_bytes,
           expected_response_type,
           expected_subtype,
           false,                       // all_zero_response_ok
           &response_packet_ptr
        );
   ASSERT_IFF(excp, !response_packet_ptr);
//   assert( (!excp && response_packet_ptr) || (excp && !response_packet_ptr));
   if (debug || IS_TRACING() ) {
      if (excp)
         DBGTRC_NOPREFIX(debug, TRACE_GROUP,
                "ddc_write_read_with_retry() returned %s, response_packet_ptr=%p",
                psc_desc(ERRINFO_STATUS(excp)), response_packet_ptr);
   }

   if (!excp) {
      assert(response_packet_ptr);
      // dump_packet(response_packet_ptr);
      Public_Status_Code psc = get_interpreted_vcp_code(response_packet_ptr, true /* make_copy */, &parsed_response);   // ???
      if (psc == 0) {
#ifdef NO_LONGER_NEEDED
         if (parsed_response->vcp_code != feature_code) {
            DBGMSG("!!! WTF! requested feature_code = 0x%02x, but code in response is 0x%02x",
                   feature_code, parsed_response->vcp_code);
            call_tuned_sleep_i2c(SE_POST_READ);
            goto retry;
         }
#endif

         if (!parsed_response->valid_response)  {
            excp = errinfo_new(DDCRC_DDC_DATA, __func__, "Invalid getvcp response");  // was DDCRC_INVALID_DATA
         }
         else if (!parsed_response->supported_opcode) {
            excp = errinfo_new(DDCRC_REPORTED_UNSUPPORTED, __func__, "Unsupported feature");
            if (!value_bytes_zero(parsed_response)) {
               // for explorering
               DBGMSG("supported_opcode == false, but not all value bytes 0");
            }
         }
         else if (value_bytes_zero(parsed_response) &&
               (dh->dref->flags & DREF_DDC_USES_MH_ML_SH_SL_ZERO_FOR_UNSUPPORTED) )
         {
            // just a messages for now
            DBGMSG("all value bytes 0, supported_opcode == true,"
                   " setting DDCRC_DETERMINED_UNSUPPORTED)");
            excp = errinfo_new(DDCRC_DETERMINED_UNSUPPORTED, __func__, "MH=ML=SH=SL=0");
         }

         if (excp) {
            free(parsed_response);
            parsed_response = NULL;
         }
      }
      else {
         excp = errinfo_new(psc, __func__, NULL);
      }
   }

   if (request_packet_ptr)
      free_ddc_packet(request_packet_ptr);
   if (response_packet_ptr)
      free_ddc_packet(response_packet_ptr);

   ASSERT_IFF(excp, !parsed_response); // needed to avoid clang warning
   if (!excp) {
      DBGTRC_NOPREFIX(debug, TRACE_GROUP, "Success reading feature x%02x. *ppinterpreted_code=%p",
                                      feature_code, parsed_response);
      DBGTRC_NOPREFIX(debug, TRACE_GROUP,
                      "mh=0x%02x, ml=0x%02x, sh=0x%02x, sl=0x%02x, max value=%d, cur value=%d",
                      parsed_response->mh, parsed_response->ml,
                      parsed_response->sh, parsed_response->sl,
                      (parsed_response->mh<<8) | parsed_response->ml,
                      (parsed_response->sh<<8) | parsed_response->sl);
   }
   *parsed_response_loc = parsed_response;

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, excp, "");
   return excp;
}


/** Gets the value of a table feature in a newly allocated Buffer struct.
 *  It is the responsibility of the caller to free the Buffer.
 *
 *  @param  dh              display handle
 *  @param  feature_code    VCP feature code
 *  @param  table_bytes_loc location at which to save address of newly allocated Buffer
 *  @return NULL if success, pointer to #Error_Info if failure
 */
Error_Info *
ddc_get_table_vcp_value(
       Display_Handle *       dh,
       Byte                   feature_code,
       Buffer**               table_bytes_loc)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "Reading feature 0x%02x", feature_code);

   Error_Info * ddc_excp = NULL;
   DDCA_Output_Level output_level = get_output_level();
   Buffer * paccumulator =  NULL;

   ddc_excp = multi_part_read_with_retry(
            dh,
            DDC_PACKET_TYPE_TABLE_READ_REQUEST,
            feature_code,
            true,                      // all_zero_response_ok
            &paccumulator);
   if (debug || ddc_excp) {
      DBGTRC_NOPREFIX(debug, TRACE_GROUP,
             "multi_part_read_with_retry() returned %s", psc_desc(ddc_excp->status_code));
   }

   if (!ddc_excp) {
      *table_bytes_loc = paccumulator;
      if (output_level >= DDCA_OL_VERBOSE) {
         DBGMSG("Bytes returned on table read:");
         dbgrpt_buffer(paccumulator, 1);
      }
   }
   // lowest level at which this check can be done, multi_part_read_with_retry() doesn't
   // know it's being called for table value
   else if (ddc_excp->status_code == DDCRC_NULL_RESPONSE ||
            ddc_excp->status_code == DDCRC_ALL_RESPONSES_NULL)
   {
      Error_Info * wrapped_exception = ddc_excp;
      ddc_excp = errinfo_new_with_cause(
            DDCRC_DETERMINED_UNSUPPORTED, wrapped_exception, __func__, "DDC NULL Message");
   }

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "*table_bytes_loc=%p", *table_bytes_loc);
   return ddc_excp;
}


/** Gets the value of a VCP feature.
 *
 * @param  dh              handle for open display
 * @param  feature_code    feature code id
 * @param  call_type       indicates whether table or non-table
 * @param  valrec_loc      location where to return newly allocated #Single_Vcp_Value
 * @return NULL if success, pointer to #Error_Info if failure
 *
 * The value pointed to by pvalrec is non-null iff the return value is null
 *
 * The caller is responsible for freeing the value returned at **valrec_loc**.
 */
Error_Info *
ddc_get_vcp_value(
       Display_Handle *       dh,
       Byte                   feature_code,
       DDCA_Vcp_Value_Type    call_type,
       DDCA_Any_Vcp_Value **  valrec_loc)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "Reading feature 0x%02x, dh=%s, dh->fd=%d",
            feature_code, dh_repr(dh), dh->fd);

   Error_Info * ddc_excp = NULL;
   Buffer * buffer = NULL;
   Parsed_Nontable_Vcp_Response * parsed_nontable_response = NULL;  // vs interpreted ..
   DDCA_Any_Vcp_Value * valrec = NULL;

   // why are we coming here for USB?
   if (dh->dref->io_path.io_mode == DDCA_IO_USB) {
#ifdef USE_USB
      DBGMSF(debug, "USB case");
      Public_Status_Code psc = 0;

      switch (call_type) {

          case (DDCA_NON_TABLE_VCP_VALUE):
                psc = usb_get_nontable_vcp_value(
                      dh,
                      feature_code,
                      &parsed_nontable_response);    //
                if (psc == 0) {
                   valrec = create_nontable_vcp_value(
                               feature_code,
                               parsed_nontable_response->mh,
                               parsed_nontable_response->ml,
                               parsed_nontable_response->sh,
                               parsed_nontable_response->sl);
                   free(parsed_nontable_response);
                }
                else
                   ddc_excp = errinfo_new(psc, __func__, NULL);
                break;

          case (DDCA_TABLE_VCP_VALUE):
                ddc_excp = errinfo_new(DDCRC_UNIMPLEMENTED, __func__,
                                        "Table features not supported for USB connection");
                break;
          }
#else
      PROGRAM_LOGIC_ERROR("ddcutil not built with USB support");
#endif
   }
   else {
      switch (call_type) {

      case (DDCA_NON_TABLE_VCP_VALUE):
            ddc_excp = ddc_get_nontable_vcp_value(
                          dh,
                          feature_code,
                          &parsed_nontable_response);
            if (!ddc_excp) {
               valrec = create_nontable_vcp_value(
                           feature_code,
                           parsed_nontable_response->mh,
                           parsed_nontable_response->ml,
                           parsed_nontable_response->sh,
                           parsed_nontable_response->sl);
               free(parsed_nontable_response);
            }
            break;

      case (DDCA_TABLE_VCP_VALUE):
            ddc_excp = ddc_get_table_vcp_value(
                    dh,
                    feature_code,
                    &buffer);
            if (!ddc_excp) {
               valrec = create_table_vcp_value_by_buffer(feature_code, buffer);
               buffer_free(buffer, __func__);
            }
            break;
      }

   } // non USB

   *valrec_loc = valrec;

   ASSERT_IFF(!ddc_excp,*valrec_loc);
   DBGTRC_RET_ERRINFO_STRUCT(debug, TRACE_GROUP, ddc_excp, valrec_loc, dbgrpt_single_vcp_value);
   return ddc_excp;
}


void init_ddc_vcp() {
   RTTI_ADD_FUNC(ddc_save_current_settings);
   RTTI_ADD_FUNC(ddc_set_nontable_vcp_value);
   RTTI_ADD_FUNC(set_table_vcp_value);
   RTTI_ADD_FUNC(ddc_set_vcp_value);
   RTTI_ADD_FUNC(ddc_get_nontable_vcp_value);
   RTTI_ADD_FUNC(ddc_get_table_vcp_value);
   RTTI_ADD_FUNC(ddc_get_vcp_value);
}

