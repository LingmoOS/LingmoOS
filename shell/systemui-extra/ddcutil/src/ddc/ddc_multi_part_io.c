/** \file ddc_multi_part_io.c
 *
 * Handle multi-part DDC reads and writes used for Table features and
 * Capabilities.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

/** \cond */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "public/ddcutil_types.h"

#include "util/string_util.h"
/** \endcond */

#include "base/core.h"
#include "base/ddc_errno.h"
#include "base/ddc_packets.h"
#include "base/execution_stats.h"
#include "base/parms.h"
#include "base/rtti.h"
#include "base/tuned_sleep.h"

#include "ddc/ddc_packet_io.h"
#include "ddc/ddc_try_stats.h"

#include "ddc/ddc_multi_part_io.h"


// Trace management
static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_DDC;


/** Makes one attempt to read the entire capabilities string or table feature value
*
* @param  dh             display handle for open i2c or adl device
* @param  request_type   DDC_PACKET_TYPE_CAPABILITIES_REQUEST or DDC_PACKET_TYPE_TABLE_REQD_REQUEST
* @param  request_subtype  VCP feature code for table read, ignore for capabilities
* @param  all_zero_response_ok  if true, an all zero response is not regarded
*         as an error
* @param  accumulator    buffer in which to return result (already allocated)
*
* @return @Error_Info struct with error detail, NULL if no error
*/
static Error_Info *
try_multi_part_read(
      Display_Handle * dh,
      Byte             request_type,
      Byte             request_subtype,
      bool             all_zero_response_ok,
      Buffer *         accumulator)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP,
          "request_type=0x%02x, request_subtype=x%02x, all_zero_response_ok=%s, accumulator=%p",
          request_type, request_subtype, sbool(all_zero_response_ok), accumulator);

   const int MAX_FRAGMENT_SIZE = 32;
   const int readbuf_size = 6 + MAX_FRAGMENT_SIZE + 1;

   Public_Status_Code psc = 0;
   Error_Info * excp = NULL;

   DDC_Packet * request_packet_ptr  = NULL;
   DDC_Packet * response_packet_ptr = NULL;
   request_packet_ptr = create_ddc_multi_part_read_request_packet(
                           request_type,
                           request_subtype,
                           0,
                           "try_multi_part_read");
   buffer_set_length(accumulator,0);
   int  cur_offset = 0;
   bool complete   = false;
   while (!complete && !excp) {         // loop over fragments
      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "Top of fragment loop");

      int fragment_size;
      update_ddc_multi_part_read_request_packet_offset(request_packet_ptr, cur_offset);
      response_packet_ptr = NULL;
      Byte expected_response_type = (request_type == DDC_PACKET_TYPE_CAPABILITIES_REQUEST)
                                       ? DDC_PACKET_TYPE_CAPABILITIES_RESPONSE
                                       : DDC_PACKET_TYPE_TABLE_READ_RESPONSE;
      Byte expected_subtype = request_subtype;     // 0x00 for capabilities, VCP feature code for table read
      excp = ddc_write_read_with_retry(
           dh,
           request_packet_ptr,
           readbuf_size,
           expected_response_type,
           expected_subtype,
           all_zero_response_ok,
           &response_packet_ptr
          );
      psc = (excp) ? excp->status_code : 0;
      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
             "ddc_write_read_with_retry() request_type=0x%02x, request_subtype=0x%02x, returned %s",
             request_type, request_subtype, errinfo_summary(excp));
      TUNED_SLEEP_WITH_TRACE(dh, SE_POST_CAP_TABLE_SEGMENT, NULL);

      if (excp) {
      // if (psc != 0) {
         if (response_packet_ptr)
            free_ddc_packet(response_packet_ptr);
         continue;
      }
      assert(response_packet_ptr);
      assert(!excp && psc == 0);

      if ( IS_TRACING_BY_FUNC_OR_FILE() || debug ) {
         DBGMSG("After try_write_read():");
         dbgrpt_interpreted_multi_read_fragment(response_packet_ptr->parsed.multi_part_read_fragment, 0);
      }

      Interpreted_Multi_Part_Read_Fragment * aux_data_ptr =
         response_packet_ptr->parsed.multi_part_read_fragment;

      int display_current_offset = aux_data_ptr->fragment_offset;
      if (display_current_offset != cur_offset) {
         DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
                "display_current_offset %d != cur_offset %d", display_current_offset, cur_offset);
         psc = DDCRC_MULTI_PART_READ_FRAGMENT;
         excp = errinfo_new(psc, __func__, NULL);
         COUNT_STATUS_CODE(psc);
      }
      else {
         DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "display_current_offset = %d matches cur_offset", display_current_offset);

         fragment_size = aux_data_ptr->fragment_length;         // ***
         DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "fragment_size = %d", fragment_size);
         if (fragment_size == 0) {
            complete = true;   // redundant
         }
         else {
            buffer_append(accumulator, aux_data_ptr->bytes, fragment_size);
            cur_offset = cur_offset + fragment_size;
            if ( IS_TRACING_BY_FUNC_OR_FILE() || debug ) {
               DBGMSG("Currently assembled fragment: |%.*s|", accumulator->len, accumulator->bytes);
               DBGMSG("cur_offset = %d", cur_offset);
            }
            all_zero_response_ok = false;           // accept all zero response only on first fragment
         }
      }
      free_ddc_packet(response_packet_ptr);

   } // while loop assembling fragments

   free_ddc_packet(request_packet_ptr);

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, excp, "");
   return excp;
}


/** Gets the DDC capabilities string for a monitor, performing retries if necessary.
 *  Also used for VCP features of type Table.
*
*  @param  dh                    handle of open display
*  @param  request_type
*  @param  request_subtype       VCP function code for table read, ignore for capabilities
*  @param  all_zero_response_ok  if true, zero response is not an error
*  @param  buffer_loc            address at which to return newly allocated #Buffer in which
*                                result is returned
*  @retval  NULL    success
*  @retval  #Ddc_Error containing status DDCRC_UNSUPPORTED does not support Capabilities Request
*  @retval  #Ddc_Error containing status DDCRC_TRIES  maximum retries exceeded:
*
*  *buffer_loc is set iff returned value is NULL
*/
Error_Info *
multi_part_read_with_retry(
      Display_Handle * dh,
      Byte             request_type,
      Byte             request_subtype,   // VCP feature code for table read, ignore for capabilities
      bool             all_zero_response_ok,
      Buffer**         buffer_loc)
{
   bool debug = false;
   Retry_Op_Value max_multi_part_read_tries = try_data_get_maxtries2(MULTI_PART_READ_OP);
   DBGTRC_STARTING(debug, TRACE_GROUP,
          "request_type=0x%02x, request_subtype=0x%02x, all_zero_response_ok=%s"
          ", max_multi_part_read_tries=%d",
          request_type, request_subtype, sbool(all_zero_response_ok), max_multi_part_read_tries);

   Public_Status_Code rc = -1;   // dummy value for first call of while loop
   Error_Info * ddc_excp = NULL;
   Error_Info * try_errors[MAX_MAX_TRIES];

   int tryctr = 0;
   bool can_retry = true;
   Buffer * accumulator = buffer_new(2048, "multi part read buffer");

   while (tryctr < max_multi_part_read_tries && rc < 0 && can_retry) {
      DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE,
             "Start of while loop. try_ctr=%d, max_multi_part_read_tries=%d",
             tryctr, max_multi_part_read_tries);

      ddc_excp = try_multi_part_read(
              dh,
              request_type,
              request_subtype,
              all_zero_response_ok,
              accumulator);
      try_errors[tryctr] = ddc_excp;
      rc = (ddc_excp) ? ddc_excp->status_code : 0;

      if (rc == DDCRC_NULL_RESPONSE || rc == DDCRC_ALL_RESPONSES_NULL) {
         // generally means this, but could conceivably indicate a protocol error.
         // try multiple times to ensure it's really unsupported?

         // just pass DDCRC_NULL_RESPONSE up the chain
         // rc = DDCRC_DETERMINED_UNSUPPORTED;
         // COUNT_STATUS_CODE(rc);   // double counting?

         can_retry = false;
      }
      else if (rc == DDCRC_READ_ALL_ZERO) {
         can_retry = false;

         // just pass DDCRC_READ_ALL_ZERO up the chain:
         // rc = DDCRC_DETERMINED_UNSUPPORTED;    // ??
         // COUNT_STATUS_CODE(rc);   // double counting?
      }
      else if (rc == DDCRC_ALL_TRIES_ZERO) {
         can_retry = false;

         // just pass it up
         // rc = DDCRC_DETERMINED_UNSUPPORTED;    // ??
         // COUNT_STATUS_CODE(rc);   // double counting?
      }
      else if (rc == -EBADF) {
         // DBGMSG("EBADF");
         can_retry = false;
      }

      tryctr++;
   }
   assert( (rc<0 && ddc_excp) || (rc==0 && !ddc_excp) );
   DBGTRC_NOPREFIX(debug, DDCA_TRC_NONE, "After try loop. tryctr=%d, rc=%d. ddc_excp=%s",
                            tryctr, rc, errinfo_summary(ddc_excp));

   if (debug) {
      for (int ndx = 0; ndx < tryctr; ndx++) {
         DBGMSG("try_errors[%d] = %s", ndx, errinfo_summary(try_errors[ndx]));
      }
   }

   if (rc < 0) {
      buffer_free(accumulator, "capabilities buffer, error");
      accumulator = NULL;
      if (tryctr >= max_multi_part_read_tries)
         rc = DDCRC_RETRIES;
      ddc_excp = errinfo_new_with_causes(rc, try_errors, tryctr, __func__, NULL);

      if (rc != try_errors[tryctr-1]->status_code)
         COUNT_STATUS_CODE(rc);     // new status code, count it
   }
   else {
      for (int ndx = 0; ndx < tryctr-1; ndx++) {
         // errinfo_free(try_errors[ndx]);
         ERRINFO_FREE_WITH_REPORT(try_errors[ndx], debug || IS_TRACING() || report_freed_exceptions);
      }
   }

   // if counts for DDCRC_ALL_TRIES_ZERO?
   try_data_record_tries2(MULTI_PART_READ_OP, rc, tryctr);

   *buffer_loc = accumulator;
   ASSERT_IFF(ddc_excp, !*buffer_loc);
   DBGTRC_RET_ERRINFO_STRUCT(debug, TRACE_GROUP, ddc_excp, buffer_loc, buffer_rpt);
#ifdef OLD
   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "*buffer_loc=%p", *buffer_loc);
   if (IS_DBGTRC(debug, TRACE_GROUP) && *buffer_loc)
     buffer_rpt(*buffer_loc, 2);
#endif
   return ddc_excp;
}


/** Makes one attempt to write an entire VCP Table value
*
*   @param dh             display handle for open i2c or adl device
*   @param vcp_code       VCP feature code
*   @param value_to_set   Table feature value
*
*   @return status code
*/
static Error_Info *
try_multi_part_write(
      Display_Handle * dh,
      Byte             vcp_code,
      Buffer *         value_to_set)
{
   bool debug = false;
   Byte request_type = DDC_PACKET_TYPE_TABLE_WRITE_REQUEST;
   Byte request_subtype = vcp_code;
   DBGTRC_STARTING(debug, TRACE_GROUP,
          "request_type=0x%02x, request_subtype=x%02x, accumulator=%p",
          request_type, request_subtype, value_to_set);

   Public_Status_Code psc = 0;
   Error_Info * ddc_excp = NULL;
   int MAX_FRAGMENT_SIZE = 32;
   int max_fragment_size = MAX_FRAGMENT_SIZE - 4;    // hack
   // const int writebbuf_size = 6 + MAX_FRAGMENT_SIZE + 1;

   DDC_Packet * request_packet_ptr  = NULL;
   int bytes_remaining = value_to_set->len;
   int offset = 0;
   while (bytes_remaining >= 0 && psc == 0) {
      int bytect_to_write = (bytes_remaining <= max_fragment_size)
                                    ? bytes_remaining
                                    : max_fragment_size;
      request_packet_ptr =  create_ddc_multi_part_write_request_packet(
                   DDC_PACKET_TYPE_TABLE_WRITE_REQUEST,
                   vcp_code,       // request_subtype,
                   offset,
                   value_to_set->bytes+offset,
                   bytect_to_write,
                   __func__);
      ddc_excp = ddc_write_only_with_retry(dh, request_packet_ptr);
      psc = (ddc_excp) ? ddc_excp->status_code : 0;
      free_ddc_packet(request_packet_ptr);
      assert( (!ddc_excp && psc == 0) || (ddc_excp && psc!=0) );

      if (!ddc_excp) {
         if (bytect_to_write == 0)   // if just wrote final empty segment to indicate done
            break;
         offset += bytect_to_write;
         bytes_remaining -= bytect_to_write;
      }
   }

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   assert( (ddc_excp && psc<0) || (!ddc_excp && psc==0) );
   return ddc_excp;
}


/** Writes a VCP table feature, with retry.
 *
 * @param  dh display handle
 * @param vcp_code  VCP feature code to write
 * @param value_to_set bytes of Table feature
 * @return  NULL if success, pointer to #Ddc_Error if failure
 */
Error_Info *
multi_part_write_with_retry(
     Display_Handle * dh,
     Byte             vcp_code,
     Buffer *         value_to_set)
{
   Retry_Op_Value max_multi_part_write_tries = try_data_get_maxtries2(MULTI_PART_WRITE_OP);
   bool debug = false;
   if (IS_TRACING())
      puts("");
   DBGTRC_STARTING(debug, TRACE_GROUP, "dh=%s, vcp_code=0x%02x",
                              dh_repr(dh), vcp_code);

   Public_Status_Code rc = -1;   // dummy value for first call of while loop
   Error_Info * ddc_excp = NULL;

   Error_Info *         try_errors[MAX_MAX_TRIES];

   int tryctr = 0;
   bool can_retry = true;

   while (tryctr < max_multi_part_write_tries && rc < 0 && can_retry) {
      DBGTRC_NOPREFIX(debug, TRACE_GROUP,
             "Start of while loop. try_ctr=%d, max_multi_part_write_tries=%d",
             tryctr, max_multi_part_write_tries);

      ddc_excp = try_multi_part_write(
              dh,
              vcp_code,
              value_to_set);
      try_errors[tryctr] = ddc_excp;
      rc = (ddc_excp) ? ddc_excp->status_code : 0;
      assert( (ddc_excp && rc<0) || (!ddc_excp && rc==0) );

      // TODO: What rc values set can_retry = false?

      tryctr++;
   }
   assert( (ddc_excp && rc < 0) || (!ddc_excp && rc==0) );

   if (rc < 0) {
      if (can_retry)
         rc = DDCRC_RETRIES;
      ddc_excp= errinfo_new_with_causes(rc, try_errors, tryctr, __func__, NULL);

      if (rc != try_errors[tryctr-1]->status_code)
         COUNT_STATUS_CODE(rc);     // new status code, count it
   }
   else {
      for (int ndx=0; ndx<tryctr; ndx++) {
         // errinfo_free(try_errors[ndx]);
         ERRINFO_FREE_WITH_REPORT(try_errors[ndx], debug || IS_TRACING() || report_freed_exceptions);
      }
   }

   DBGTRC_RET_ERRINFO(debug, TRACE_GROUP, ddc_excp, "");
   return ddc_excp;
}


static inline void init_ddc_multi_part_io_func_name_table() {
#define ADD_FUNC(_NAME) rtti_func_name_table_add(_NAME, #_NAME);
   ADD_FUNC(try_multi_part_read);
   ADD_FUNC(multi_part_read_with_retry);
#undef ADD_FUNC
}


void init_ddc_multi_part_io() {
   init_ddc_multi_part_io_func_name_table();
}
