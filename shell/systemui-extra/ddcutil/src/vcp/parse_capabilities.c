/** @file parse_capabilities.c
 *  Parse the capabilities string returned by DDC, query the parsed data structure.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

/** \cond */
#include <assert.h>
#include <glib-2.0/glib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/** \endcond */

#include "util/report_util.h"
#include "util/string_util.h"

#include "base/core.h"
#include "base/ddc_command_codes.h"
#include "base/displays.h"
#include "base/rtti.h"
#include "base/vcp_version.h"

#include "vcp/parsed_capabilities_feature.h"
#include "vcp/vcp_feature_codes.h"

#include "vcp/parse_capabilities.h"


static DDCA_Trace_Group  TRACE_GROUP = DDCA_TRC_NONE;

#undef CAPABILITIES_TESTS
// #define CAPABILITIES_TESTS

#ifdef CAPABILITIES_TESTS
// not made static to avoid warning about unused variable
char* test_cap_strings[] = {
      // GSM LG Ultra HD
      "(prot(monitor)type(LED)model(25UM65)cmds(01 02 03 0C E33 F3)"
      "vcp(0203(10 00)0405080B0C101214(05 07 08 0B) 16181A5260(3033 04)6C6E70"
      "87ACAEB6C0C6C8C9D6(01 04)DFE4E5E6E7E8E9EAEBED(00 10 20 40)EE(00 01)"
      "FE(01 02 03)FF)mswhql(1)mccs_ver(2.1))",
      // Asus PB287
      "(prot(monitor) type(LCD)model LCDPB287 cmds(01 02 03 07 0C F3) "
      "vcp(02 04 05 08 0B 0C 10 12 14(05 06 08 0B) 16 18 1A 60(11 12 0F) "
      "62 6C 6E 70 8D(01 02) A8 AC AE B6 C6 C8 C9 D6(01 04) DF) "
      "mccs_ver(2.1)asset_eep(32)mpu(01)mswhql(1))",

};
#endif

Value_Name_Table capabilities_validity_names = {
   VN(CAPABILITIES_VALID),
   VN(CAPABILITIES_USABLE),
   VN(CAPABILITIES_INVALID),
   VN_END
};

char * capabilities_validity_name(Parsed_Capabilities_Validity validity) {
   return vnt_name(capabilities_validity_names, validity);
}


void dbgrpt_parsed_capabilities(Parsed_Capabilities * pcaps, int depth) {
   int d1 = depth+1;
   int d2 = depth+2;
   rpt_structure_loc("Parsed_Capabilities", pcaps, depth);
   if (pcaps) {
       rpt_vstring(d1, "raw value:               %s",       pcaps->raw_value);
       rpt_vstring(d1, "raw_value_synthesized:   %s",  sbool(pcaps->raw_value_synthesized));
       rpt_vstring(d1, "model:                   %s", pcaps->model);
       rpt_vstring(d1, "mccs version string:     %s", pcaps->mccs_version_string);
       rpt_vstring(d1, "parsed_mccs_version:     %d.%d = %s",
                                                   pcaps->parsed_mccs_version.major,
                                                   pcaps->parsed_mccs_version.minor,
                                                   format_vspec(pcaps->parsed_mccs_version) );

       rpt_vstring(d1, "raw_cmds_segment_seen:   %s",  sbool(pcaps->raw_cmds_segment_seen));
       rpt_vstring(d1, "raw_cmds_segment_valid:  %s", sbool(pcaps->raw_cmds_segment_valid) );
       char * t = (pcaps->commands) ? bva_as_string(pcaps->commands, /*as_hex=*/true, " ") : NULL;
        rpt_vstring(d1, "commands:                %s", (t) ? t : "NULL");
        if (t)
           free(t);

       rpt_vstring(d1, "raw_vcp_features_seen:   %s", sbool(pcaps->raw_vcp_features_seen));
       rpt_vstring(d1, "vcp_features.len:        %d", pcaps->vcp_features->len);

       rpt_vstring(d1, "caps_validity:           %s", capabilities_validity_name(pcaps->caps_validity));

       if (!pcaps->messages || pcaps->messages->len == 0)
          rpt_label(d1, "No messages");
       else {
          rpt_label(d1, "Messages:");
          for (int ndx = 0; ndx < pcaps->messages->len; ndx++)
             rpt_vstring(d2, "%s", (char *) g_ptr_array_index(pcaps->messages, ndx));
       }
   }
}


/** Frees a Parsed_Capabilities record
 *
 * @param pcaps  pointer to #Parsed_Capabilities struct
 */
void free_parsed_capabilities(Parsed_Capabilities * pcaps) {
   bool debug = false;
   DBGMSF(debug, "Starting. pcaps=%p", pcaps);

   assert( pcaps );
   assert( memcmp(pcaps->marker, PARSED_CAPABILITIES_MARKER, 4) == 0);

   free(pcaps->raw_value);
   free(pcaps->mccs_version_string);
   free(pcaps->model);

   if (pcaps->commands)
      bva_free(pcaps->commands);

   if (pcaps->vcp_features) {
      DBGMSF(debug, "vcp_features->len = %d", pcaps->vcp_features->len);
      int ndx;
      for (ndx=pcaps->vcp_features->len-1; ndx >=0; ndx--) {
         Capabilities_Feature_Record * vfr =
               g_ptr_array_index(pcaps->vcp_features, ndx);
         // report_feature(vfr);
         free_capabilities_feature_record(vfr);
         g_ptr_array_remove_index(pcaps->vcp_features, ndx);
      }
      g_ptr_array_free(pcaps->vcp_features, true);
      if (pcaps->messages)
         g_ptr_array_free(pcaps->messages, true);
   }

   pcaps->marker[3] = 'x';
   free(pcaps);
}


//
// *** Utility Functions ***
//

/* Point to the first non-space character in a string.
 *
 * @param  s   pointer to string
 * \parsm  len length of string
 * @return pointer to first non-space character of string,
 *         end of string if not found
 */
char * ltrim(char * s, int len) {
   while (len > 0 && *s == ' ') {
      len--;
      s++;
   }
   return s;
}


/** Finds the matching closing parenthesis for the
 *  current open parenthesis.
 *
 *  @param start    first character to examine (must be '(')
 *  @param end      points to end of string, i.e. the byte
 *                  after the last character to examine
 *  @return pointer pointer to closing parenthesis,
 *          end if closing parenthesis not found
 */

static char * find_closing_paren(
      char * start,
      char * end)
{
   assert( *start == '(');
   char * pos = start+1;
   int depth = 1;
   while (pos < end && depth > 0) {
      if (*pos == '(')
         depth++;
      else if (*pos == ')')
         depth--;
      pos++;
   }
   if (depth == 0)
      pos--;
   return pos;
}


Parsed_Capabilities_Validity
update_validity(
   Parsed_Capabilities_Validity validity,
   Parsed_Capabilities_Validity cur_validity)
{
   // could be clever using the ordering of the numeric value of enum
   // but this is robust against enum changes
   Parsed_Capabilities_Validity result = CAPABILITIES_USABLE;
   if (validity == CAPABILITIES_INVALID || cur_validity == CAPABILITIES_INVALID)
      result = CAPABILITIES_INVALID;
   else if (validity == CAPABILITIES_VALID && cur_validity == CAPABILITIES_VALID)
      result = CAPABILITIES_VALID;
   return result;
}


//
// Parsing
//

/* Capabilities string format:
     Parenthesized expression
     containing sequence of "segments"
     each segment consists of a segment name, followed by a parenthesized value
 */


//
// cmds() segment
//

/** Parses the value of the cmds segment, which is a list of 2 character values
 *  separated by spaces.
 *
 * @param   start    start of values
 * @param   len      segment length
 * @param   messages accumulates error messages
 * @return  #Byte_Value_Array indicating command values seen,
 *          NULL if a parsing error
 *
 *  @remark
 *  Alternatively, return a ByteBitFlag value, or pass a
 *  preallocted ByteBitFlag instance
 *
 *  @remark
 *  On every monitor tested, the values are separated by spaces.
 *  However, per the Access Bus spec, Section 7, values need not be separated by spaces,
 *  e.g. 010203 is valid
 */
static Byte_Value_Array parse_cmds_segment(
      char *      start,
      int         len,
      GPtrArray * messages)
{
   bool debug = false;
   DBGMSF(debug, "Starting. start=%p, len=%d", start, len);

   Byte_Value_Array cmd_ids = bva_create();
   bool ok = store_bytehex_list(start, len, cmd_ids, bva_appender);
   // ok = false;   // force failure for testing
   if (!ok) {
      char * s = g_strdup_printf("Error processing commands list: %.*s", len, start);
      g_ptr_array_add(messages, s);
   }
   if (!ok) {
      bva_free(cmd_ids);
      cmd_ids = NULL;
   }

   DBGMSF(debug, "Done.     Returning %p", cmd_ids);
   return cmd_ids;
}


//
// vcp segment
//

#ifdef FUTURE
void parse_vcp_values(char * start, int len, GPtrArray* messages) {
}
#endif


//
// vcp() segment
//

typedef
struct {
   char * code_start;
   int    code_len;
   char * values_start;
   int    values_len;
   char * remainder_start;
   int    remainder_len;
   bool   valid;
} Vcp_Feature_Segment;


/** Parse the value of a vcp() segment..
 *
 *  @param  start    offset to start of segment
 *  @param  len      length of segment
 *  @param  messages accumulates error messages
 *
 *  A VCP contains either the feature code in hex, or the feature code followed
 *  by a parenthesized list of values (in hex).
 *
 *  @returns GPtrArray of Capabilities_Feature_Record *
 *
 *  The returned pointer is never NULL.  The **GPtrArray** it points to may
 *  contain 0 pointers.
 */
static Parsed_Capabilities_Validity
parse_vcp_segment(
      char *       start,
      int          len,
      GPtrArray *  vcp_array,
      GPtrArray *  messages)
{
   bool debug = false;
   DBGMSF(debug, "Starting.  len = %d, start=%p -> %.*s", len, start, len, start);
   // Vcp_Code_Table_Entry * vcp_entry;    // future?

   Parsed_Capabilities_Validity result = CAPABILITIES_VALID;

   char * pos = start;
   char * end = start + len;
   DBGMSF(debug, "Parsing: |%.*s|", len, start);
   Byte   cur_feature_id = 0x00; // initialization logically unnecessary, but o.w. get warning
   bool   valid_feature;
   int    value_len = 0;         // initialization logically unnecessary, but o.w. get warning
   char * value_start = NULL;    // ditto
   while (pos < end) {
      valid_feature = false;
      // strip leading blanks
      while(*pos == ' ' && pos < end) pos++;
      if (pos == end)
         break;

      char * st = pos;
      while (*pos != ' ' && *pos != '(' && pos < end) pos++;
      int len = pos-st;
      DBGMSF(debug, "Found: Feature code subsegment: %.*s", len, st);
      // If len > 2, feature codes not separated by blanks.  Take just the first 2 characters
      if (len > 2) {
         pos = st + 2;
         len = 2;
      }
      bool feature_code_ok = false;
      if (len == 2) {
         // cur_feature_id = hhc_to_byte(st);   // what if invalid hex?
         feature_code_ok = hhc_to_byte_in_buf(st, &cur_feature_id);
         if (feature_code_ok) {
            valid_feature = true;
            value_start = NULL;
            value_len   = 0;
         }
      }
      if (!feature_code_ok) {
         char * s = g_strdup_printf("Feature %.*s (Invalid code)",1,st);
         g_ptr_array_add(messages, s);
         // f0printf(ferr(), "Feature: %.*s (invalid code)\n", 1, st);
         if (result == CAPABILITIES_VALID)
            result = CAPABILITIES_USABLE;
      }

      if (*pos == '(') {
         // find matching )
         char * value_end = find_closing_paren(pos, end);
         if (value_end == end) {
            g_ptr_array_add(messages, strdup("Value parse terminated without closing parenthesis") );
            // TODO: recover from error, this is bad data from the monitor
            result = CAPABILITIES_INVALID;
            goto bye;  // Error is fatal
         }
         value_start = pos+1;
         value_len = value_end - (pos + 1);
         // printf("  Values: %.*s\n", value_len, value_start);
         pos = value_end + 1;   // point to character after closing paren
      }

      if (valid_feature) {
         Capabilities_Feature_Record * vfr =
               parse_capabilities_feature(cur_feature_id, value_start, value_len, messages);
         // if (debug) {
         //    DDCA_MCCS_Version_Spec dummy_version = {0,0};
         //    report_capabilities_feature(vfr, dummy_version, 1);
         // }
         if (!vfr->valid_values && result == CAPABILITIES_VALID)
            result = CAPABILITIES_USABLE;
         g_ptr_array_add(vcp_array, vfr);
      }
   }
bye:
   if (result != CAPABILITIES_VALID)
      DBGMSF(debug, "feature = 0x%02x, %s", cur_feature_id, capabilities_validity_name(result) );

   return result;
}


#ifdef IN_PROGRESS

/** Process the VCP segment
 *
 *  @param  start    offset to start of segment
 *  @param  len      length of segment
 *  @param  messages accumulates error messages
 *
 *  @returns GPtrArray of Capabilities_Feature_Record *
 *
 *  The returned pointer is never NULL.  The **GPtrArray** it points to may
 *  contain 0 pointers.
 */
 Parsed_Capabilities_Validity
parse_vcp_segment_new(
      char *       start,
      int          len,
      GPtrArray *  vcp_array,
      GPtrArray *  messages)
{
   bool debug = false;
   DBGMSF(debug, "Starting.  len = %d, start=%p -> %.*s", len, start, len, start);
   // Vcp_Code_Table_Entry * vcp_entry;    // future?

   Parsed_Capabilities_Validity validity = CAPABILITIES_VALID;
   Vcp_Feature_Segment* segment = NULL;
   char * pos = start;
   while( (segment = next_vcp_feature_segment(pos, len, messages)) ) {
      // start debug code
      if (!segment->valid) {
         // issue msg?
         break;
      }
      DBGMSG("call a function to process the segment");
      Parsed_Capabilities_Validity cur_validity = parse_single_feature(segment, vcp_array, messages);
      validity = update_validity(validity, cur_validity);
      pos = segment->remainder_start;
      len = segment->remainder_len;
   }

   DBGMSG("Begin regular parsing");

   Parsed_Capabilities_Validity result = CAPABILITIES_VALID;

   // char *
   pos = start;
   char * end = start + len;
   DBGMSF(debug, "Parsing: |%.*s|", len, start);
   Byte   cur_feature_id = 0x00; // initialization logically unnecessary, but o.w. get warning
   bool   valid_feature;
   int    value_len = 0;         // initialization logically unnecessary, but o.w. get warning
   char * value_start = NULL;    // ditto
   while (pos < end) {
      valid_feature = false;
      // strip leading blanks
      while(*pos == ' ' && pos < end) pos++;
      if (pos == end)
         break;

      char * st = pos;
      while (*pos != ' ' && *pos != '(' && pos < end) pos++;
      int len = pos-st;
      DBGMSF(debug, "Found: Feature code subsegment: %.*s", len, st);
      // If len > 2, feature codes not separated by blanks.  Take just the first 2 characters
      if (len > 2) {
         pos = st + 2;
         len = 2;
      }
      bool feature_code_ok = false;
      if (len == 2) {
         // cur_feature_id = hhc_to_byte(st);   // what if invalid hex?
         feature_code_ok = hhc_to_byte_in_buf(st, &cur_feature_id);
         if (feature_code_ok) {
            valid_feature = true;
            value_start = NULL;
            value_len   = 0;
         }
      }
      if (!feature_code_ok) {
         char * s = g_strdup_printf("Feature %.*s (Invalid code)",1,st);
         g_ptr_array_add(messages, s);
         // f0printf(ferr(), "Feature: %.*s (invalid code)\n", 1, st);
         if (result == CAPABILITIES_VALID)
            result = CAPABILITIES_USABLE;
      }

      if (*pos == '(') {
         // find matching )
         char * value_end = find_closing_paren(pos, end);
         if (value_end == end) {
            g_ptr_array_add(messages, "Value parse terminated without closing parenthesis" );
            // TODO: recover from error, this is bad data from the monitor
            result = CAPABILITIES_INVALID;
            goto bye;  // Error is fatal
         }
         value_start = pos+1;
         value_len = value_end - (pos + 1);
         // printf("  Values: %.*s\n", value_len, value_start);
         pos = value_end + 1;   // point to character after closing paren
      }

      if (valid_feature) {
         Capabilities_Feature_Record * vfr =
               parse_capabilities_feature(cur_feature_id, value_start, value_len, messages);
         // if (debug) {
         //    DDCA_MCCS_Version_Spec dummy_version = {0,0};
         //    report_capabilities_feature(vfr, dummy_version, 1);
         // }
         if (!vfr->valid_values && result == CAPABILITIES_VALID)
            result = CAPABILITIES_USABLE;
         g_ptr_array_add(vcp_array, vfr);
      }
   }
bye:
   if (result != CAPABILITIES_VALID)
      DBGMSG("feature = 0x%02x, %s", cur_feature_id, capabilities_validity_name(result) );

   return result;
}
#endif


//
// Top level functions for parsing a capabilities string
//

// A top level segment of the capabilities string
// Has the form " name(value)", e.g. "commands(01 02 04 08)"

typedef
struct {
   char * name_start;
   int    name_len;
   char * value_start;
   int    value_len;
   char * remainder_start;
   int    remainder_len;
} Capabilities_Segment;

// implement if necessary:
// void dbgrpt_capabilities_segment(Capabilities_Segment * segment, int depth) {


/** Extract the next top level segment of the capabilities string.
 *
 *  @param  start   current position in the capabilities string
 *  @param  len     length of remainder of capabilities string
 *  @return pointer to newly allocated Capabilities_Segment describing the segment
 *          It is the responsibility of the caller to free the returned struct,
 *          BUT NOT THE LOCATIONS IT ADDRESSES
 */
static Capabilities_Segment *
next_capabilities_segment(char * start, int len, GPtrArray* messages, char * capabilities_staring_start)
{
   bool debug = false;
   DBGMSF(debug, "Starting. len=%d, start=%p -> |%.*s|", len, start, len, start);
   const char * global_start = capabilities_staring_start; // was: start;
   char * end = start+len;
   Capabilities_Segment * segment = calloc(1, sizeof(Capabilities_Segment));
   // n. Apple Cinema Display precedes segment name with blank
   char * trimmed_start = ltrim(start, len);
   int    trimmed_len   = len - (trimmed_start - start) ;
   char * errmsg = NULL;
   // DBGMSG("trimmed_len=%ld, trimmed_start=%p -> |%.*s|",
   //        trimmed_len, trimmed_start, trimmed_len, trimmed_start);
   if (trimmed_len == 0) {
      // just indicate there's no more to do
      free(segment);
      segment = NULL;
      goto bye;
   }
   char * pos = trimmed_start;


#define REQUIRE(_condition, _msg, _position)  \
   if (!(_condition)) { \
      g_ptr_array_add(messages, g_strdup_printf("%s at offset %jd", \
                      _msg, (intmax_t)( _position-global_start)) ); \
      segment->name_start = segment->value_start = segment->remainder_start = NULL; \
      segment->name_len   = segment->value_len   = segment->remainder_len   = 0; \
      goto bye; \
   }

   REQUIRE( *trimmed_start != '(' , "Missing segment name", pos);
   segment->name_start = trimmed_start;
   while (pos < end && *pos != '(' && *pos != ' ') {pos++;}  // name stops with either left paren or space
   REQUIRE( pos < end, "Nothing follows segment name", pos);
   segment->name_len = pos-trimmed_start;
   while ( pos < end && *pos == ' ' ) { pos++; }   // blanks following segment name

   errmsg = g_strdup_printf("Nothing follows segment name (2) %.*s",
                            segment->name_len, segment->name_start);
   REQUIRE( pos < end, errmsg, pos);
   free(errmsg); errmsg = NULL;

   // assert(*pos == '(');
   DBGMSF(debug, "pos=%p, trimmed_start=%p", pos, trimmed_start);
   segment->name_len = pos - trimmed_start;
   DBGMSF(debug, "start=%p, len=%d, trimmed_start=%p", start, len, trimmed_start);
   DBGMSF(debug, "name_len = %d, name_start = %p -> %.*s", segment->name_len, segment->name_start,
                                                   segment->name_len, segment->name_start);
   errmsg = g_strdup_printf("Missing parenthesized value for segment %.*s",
                                   segment->name_len, segment->name_start);
   REQUIRE(*pos == '(', errmsg, pos);
   free(errmsg); errmsg = NULL;

   segment->value_start = pos+1;
   pos =find_closing_paren(pos, end);

   errmsg = g_strdup_printf("No closing parenthesis for segment %.*s",
                            segment->name_len, segment->name_start);
   REQUIRE(pos < end, errmsg, pos);
   free(errmsg); errmsg=NULL;

   segment->value_len = pos - segment->value_start;
   REQUIRE(segment->value_len > 0, "zero length value", pos);

   segment->remainder_start = pos+1;
   segment->remainder_len = end - (pos+1);

   // printf("name:      |%.*s|\n", segment->name_len, segment->name_start);
   // printf("value:     |%.*s|\n", segment->value_len, segment->value_start);
   // printf("remainder: |%.*s|\n", segment->remainder_len, segment->remainder_start);

bye:
   if (errmsg)
      free(errmsg);
   DBGMSF(debug, "Returning: %p", segment);
   return segment;
}


/** Parses the entire capabilities string
 *
 *  @param  buf_start   starting address of string
 *  @param  buf_len     length of string (not including trailing null)
 *
 *  @return pointer to newly allocated ParsedCapabilities structure
 */
Parsed_Capabilities * parse_capabilities(
      char * buf_start,
      int    buf_len)
{
   assert(buf_start);
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "buf_len=%d, buf_start=%p->|%.*s|",
                                       buf_start, buf_len, buf_len, buf_start);
   if ( IS_DBGTRC(debug, TRACE_GROUP) ) {
      rpt_hex_dump((Byte*)buf_start, buf_len, 1);
   }
   
   // right trim white space
   while ( buf_len > 0 && *(buf_start+buf_len-1) == ' ') {
      buf_len--;
   }

   char * capabilities_string_start = buf_start;
   Parsed_Capabilities* pcaps = calloc(1, sizeof(Parsed_Capabilities));
   memcpy(pcaps->marker, PARSED_CAPABILITIES_MARKER, 4);

   // Explicitly initialize all fields as documentation
   pcaps->raw_value = chars_to_string(buf_start, buf_len);
   pcaps->raw_value_synthesized = false;   // set by user of function
   pcaps->mccs_version_string  = NULL;
   pcaps->parsed_mccs_version = DDCA_VSPEC_UNQUERIED;
   pcaps->raw_cmds_segment_seen = false;
   pcaps->commands = NULL;
   pcaps->raw_vcp_features_seen = false;
   pcaps->raw_cmds_segment_valid = false;
   pcaps->caps_validity = CAPABILITIES_VALID;
   pcaps->vcp_features = g_ptr_array_sized_new(40);
   pcaps->messages = g_ptr_array_new();

   // DBGMSG("Initial buf_len=%d, buf_start=%p -> |%.*s|", buf_len, buf_start, buf_len, buf_start);
   // Apple Cinema display violates spec, does not surround capabilities string with parens
   if (buf_start[0] == '(') {
      if (buf_start[buf_len-1] != ')') {
            g_ptr_array_add(pcaps->messages,
                            strdup("Capabilities string lacks closing parenthesis"));
            pcaps->caps_validity = CAPABILITIES_INVALID;
            goto bye;
      }

      // trim starting and ending parens
      buf_start = buf_start+1;
      buf_len = buf_len -2;
   }
   // DBGMSG("Adjusted buf_len=%d, buf_start=%p -> |%.*s|", buf_len, buf_start, buf_len, buf_start);

   while (buf_len > 0) {
      Capabilities_Segment * seg =
         next_capabilities_segment(buf_start, buf_len, pcaps->messages, capabilities_string_start);
      if (seg->name_start == NULL)  {
         // error condition encountered
         pcaps->caps_validity = CAPABILITIES_INVALID;
         free(seg);
         break;
      }
      buf_start = seg->remainder_start;
      buf_len   = seg->remainder_len;

      DBGMSF(debug, "Segment:  |%.*s| -> |%.*s|",
                    seg->name_len,  seg->name_start,  seg->value_len, seg->value_start );

      // cmds segment
      if (seg->name_len == 4 &&  // avoid buffer overflow
          memcmp(seg->name_start, "cmds", seg->name_len) == 0)
      {
         pcaps->raw_cmds_segment_seen = true;
         pcaps->commands = parse_cmds_segment(seg->value_start, seg->value_len, pcaps->messages);
         pcaps->raw_cmds_segment_valid = (pcaps->commands);   // ??
         if (!pcaps->commands) {
            if (pcaps->caps_validity == CAPABILITIES_VALID)
               pcaps->caps_validity = CAPABILITIES_USABLE;
         }
      }

      // vcp segment
      else if (seg->name_len == 3 &&
                (memcmp(seg->name_start, "vcp", seg->name_len) == 0 ||
                 memcmp(seg->name_start, "VCP", seg->name_len) == 0      // hack for Apple Cinema Display
                )
              )
      {
         pcaps->raw_vcp_features_seen = true;
         Parsed_Capabilities_Validity vcp_segment_validity =
               parse_vcp_segment(seg->value_start, seg->value_len, pcaps->vcp_features, pcaps->messages);

         pcaps->caps_validity = update_validity(pcaps->caps_validity, vcp_segment_validity);
      }

      else if (seg->name_len == 8 &&
               memcmp(seg->name_start,
                      "mccs_ver" /* was "mccs_version_string" WHY? */,
                      seg->name_len) == 0)
      {
         pcaps->mccs_version_string = chars_to_string(seg->value_start, seg->value_len);
         DDCA_MCCS_Version_Spec vspec = parse_vspec(pcaps->mccs_version_string);
         // n. will be DDCA_VSPEC_UNQUERIED if no value string, DDCA_VSPEC_UNKNOWN if invalid
         pcaps->parsed_mccs_version = vspec;
         DBGMSF(debug, "parsed version: %s", format_vspec(vspec));
         if ( vcp_version_eq(vspec,DDCA_VSPEC_UNKNOWN)) {
            if (pcaps->caps_validity == CAPABILITIES_VALID)
               pcaps->caps_validity = CAPABILITIES_USABLE;
            char * errmsg = g_strdup_printf("Invalid mccs_ver: \"%s\"", pcaps->mccs_version_string);
            g_ptr_array_add(pcaps->messages, errmsg);
         }
      }

      else if ( seg->name_len == 5 &&
                memcmp(seg->name_start, "model", seg->name_len) == 0 )
      {
         DBGMSF(debug, "model: |%.*s|", seg->value_len, seg->value_start);
         pcaps->model = chars_to_string(seg->value_start, seg->value_len);
         // DBGMSF(debug, "pcaps->model = |%s|", pcaps->model);
      }

      else {
         // additional segment names seen: asset_eep, mpu, mswhql
         DBGMSF(debug, "Ignoring segment: %.*s", seg->name_len, seg->name_start);
      }
      free(seg); // allocated by next_capabilities_segment()
   }

bye:
   DBGTRC_RET_STRUCT(debug, DDCA_TRC_NONE, "Parsed_Capabilities", dbgrpt_parsed_capabilities, pcaps);
   return pcaps;
}


/** Parses a capabilities string
 *
 *  @param  caps   null terminated capabilities string
 *  @return pointer to newly allocated #Parsed_Capabilities structure
 */
Parsed_Capabilities* parse_capabilities_string(
      char * caps)
{
   assert(caps);
#ifdef CAPABILITIES_TESTS
   // Uncomment to enable test
   DBGMSG("Substituting test capabilities string");
   caps = test_cap_strings[1];
#endif

   return parse_capabilities(caps, strlen(caps));
}


//
// Functions to query Parsed_Capabilities
//

/** Returns list of feature ids in a #Parsed_Capabilities structure.
 *
 *  @param pcaps           pointer to #Parsed_Capabilities
 *  @param readable_only   restrict returned list to readable features
 *
 *  @return  #Bit_Set_256  value indicating features found
 */
Bit_Set_256 get_parsed_capabilities_feature_ids(
      Parsed_Capabilities * pcaps,
      bool                  readable_only)
{
   assert(pcaps);
   bool debug = false;
   DBGMSF(debug, "Starting. readable_only=%s, feature count=%d",
                 sbool(readable_only), pcaps->vcp_features->len);

   // Byte_Bit_Flags flags = bbf_create();
   Bit_Set_256 flags = EMPTY_BIT_SET_256;
   if (pcaps->vcp_features) {    // pathological case of 0 length capabilities string
      for (int ndx = 0; ndx < pcaps->vcp_features->len; ndx++) {
         Capabilities_Feature_Record * frec = g_ptr_array_index(pcaps->vcp_features, ndx);
         // DBGMSG("Feature 0x%02x", frec->feature_id);

         bool add_feature_to_list = true;
         if (readable_only) {
            VCP_Feature_Table_Entry * vfte = vcp_find_feature_by_hexid_w_default(frec->feature_id);
            if (!is_feature_readable_by_vcp_version(vfte, pcaps->parsed_mccs_version))
               add_feature_to_list = false;
            if (vfte->vcp_global_flags & DDCA_SYNTHETIC_VCP_FEATURE_TABLE_ENTRY)
               free_synthetic_vcp_entry(vfte);
         }
         if (add_feature_to_list)
            // bbf_insert(flags, frec->feature_id);
            flags = bs256_insert(flags, frec->feature_id);
      }
   }

   DBGMSF(debug, "Returning Bit_Set_256: %s", bs256_to_string(flags, "x", ", ") );
   return flags;
}


/** Checks if a monitor supports table features.
 *
 *  @param   pcaps  pointer to #Parsed_Capabilities (may be null)
 *
 *  @return **true** if a command segment was parsed and both
 *                   Table Read Request and Table Read Reply are declared
 *          **false** otherwise
 */
bool parsed_capabilities_supports_table_commands(Parsed_Capabilities * pcaps) {
   bool result = false;
   if (pcaps && pcaps->raw_cmds_segment_seen && pcaps->commands &&
       bva_contains(pcaps->commands, 0xe2) &&      // Table Read Request
       bva_contains(pcaps->commands, 0xe4)         // Table Read Reply
      )
   {
         result = false;
   }
   return result;
}


//
// *** TESTS ***
//

#ifdef CAPABILITIES_TESTS
//
// Tests
//

#ifdef OLD
void test_segment(char * text) {
   char * start = text;
   int len   = strlen(text);
   Capabilities_Segment * capseg = next_capabilities_segment(start, len);
   free(capseg);      // it's just a test function, but avoid coverity flagging memory leak
}


void test_segments() {
   test_segment("vcp(10 20)");
   test_segment("vcp(10 20)abc");
   test_segment("vcp(10 20 30( asdf ))x");
}
#endif


void test_parse_caps() {
   Parsed_Capabilities * pcaps = parse_capabilities_string("(alpha(adsf)vcp(10 20 30(31 32) ))");
   dbgrpt_parsed_capabilities(pcaps,0);
   free_parsed_capabilities(pcaps);
}
#endif


/** Module initialization */
void init_parse_capabilities() {
   RTTI_ADD_FUNC(parse_capabilities);
}



