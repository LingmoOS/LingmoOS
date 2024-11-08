/* @file feature_metadata.c
 *
 * Functions for external and internal representation of
 * display-specific feature metadata.
 */

// Copyright (C) 2018-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

/** \cond */
#include <assert.h>
#include <glib-2.0/glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/glib_util.h"
#include "util/report_util.h"
/** \endcond */

#include "base/displays.h"
#include "base/rtti.h"
#include "base/vcp_version.h"

#include "base/feature_metadata.h"

// #include "base/dynamic_features.h"   // oops for dbgreport_feature_metadata()


/** Thread safe function that returns a string representation of a #Nontable_Vcp_Value
 *  suitable for diagnostic messages. The returned value is valid until the
 *  next call to this function on the current thread.
 *
 *  \param  vcp_value  pointer to #Nontable_Vcp_Value
 *  \return string representation of value
 */
char * nontable_vcp_value_repr_t(Nontable_Vcp_Value * vcp_value) {
   static GPrivate  nontable_value_repr_key = G_PRIVATE_INIT(g_free);

   char * buf = get_thread_fixed_buffer(&nontable_value_repr_key, 100);
   if (vcp_value)
      g_snprintf(buf, 100,
          "Nontable_Vcp_Value[vcp_code: 0x%02x, max=%d, cur=%d, mh=0x%02x, ml=0x%02x, sh=0x%02x, sl=0x%02x]",
          vcp_value->vcp_code, vcp_value->max_value, vcp_value->cur_value,
          vcp_value->mh, vcp_value->ml, vcp_value->sh, vcp_value->sl);
   else
      strcpy(buf, "Nontable_Vcp_Value[NULL]");
   return buf;
}



// Feature flags

/** Creates a string representation of DDCA_Feature_Flags bitfield.
 *
 *  @param  flags      feature characteristics
 *  @return string representation, valid until the next call
 *          of this function in the current thread, do not free
 *
 *  @remark
 *  Currently used only in debug code.  (11/2018)
 */
char *
interpret_feature_flags_t(DDCA_Version_Feature_Flags flags) {
   static GPrivate  buf_key = G_PRIVATE_INIT(g_free);
   char * buffer = get_thread_fixed_buffer(&buf_key, 100);

   g_snprintf(buffer, 100, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
       flags & DDCA_RO               ? "Read-Only, "                   : "",
       flags & DDCA_WO               ? "Write-Only, "                  : "",
       flags & DDCA_RW               ? "Read-Write, "                  : "",
       flags & DDCA_STD_CONT         ? "Continuous (standard), "       : "",
       flags & DDCA_COMPLEX_CONT     ? "Continuous (complex), "        : "",
       flags & DDCA_SIMPLE_NC        ? "Non-Continuous (simple), "     : "",
       flags & DDCA_COMPLEX_NC       ? "Non-Continuous (complex), "    : "",
       flags & DDCA_NC_CONT          ? "Non-Continuous with continuous subrange, " :"",
       flags & DDCA_WO_NC            ? "Non-Continuous (write-only), " : "",
       flags & DDCA_NORMAL_TABLE     ? "Table (readable), "            : "",
       flags & DDCA_WO_TABLE         ? "Table (write-only), "          : "",
       flags & DDCA_DEPRECATED       ? "Deprecated, "                  : "",
       flags & DDCA_USER_DEFINED     ? "User-defined, "                : "",
       flags & DDCA_SYNTHETIC        ? "Synthesized metadata, "        : "",


       // Should never occur in DDCA_Version_Feature_Flags:
       flags & DDCA_PERSISTENT_METADATA ? "Persistent metadata, "         : "",
       flags & DDCA_SYNTHETIC_VCP_FEATURE_TABLE_ENTRY ? "Synthesized VFTE, "          : ""
       );
   // remove final comma and blank
   if (strlen(buffer) > 0)
      buffer[strlen(buffer)-2] = '\0';

   return buffer;
}


// SL value tables

/** Returns the number of entries in a feature value table, including the
 *  final terminating entry.
 *
 *  @param  table  feature value table
 *  @return number of entries
 */
static int
sl_value_table_size(DDCA_Feature_Value_Entry * table) {
   int ct = 0;
   if (table) {
      DDCA_Feature_Value_Entry * cur = table;
      while (true) {
         ct++;
         if (!cur->value_name)
            break;
         cur++;
      }
   }
   return ct;
}


/** Emit a debugging report of a feature value table.
 *
 *  @param table  pointer to first #DDCA_Feature_Value_Entry in table
 *  @param title  name of table
 *  @param depth  logical indentation depth
 */
void
dbgrpt_sl_value_table(DDCA_Feature_Value_Entry * table, char * title, int depth) {
   int d1 = depth+1;
   if (table) {
      rpt_vstring(depth, "%s table at %p", title, table);
      if (table) {
         rpt_vstring(depth, "Members: ");
         DDCA_Feature_Value_Entry * cur = table;
         while (cur->value_name) {
            rpt_vstring(d1, "0x%02x -> %s", cur->value_code, cur->value_name);
            cur++;
         }
      }
   }
   else {
      rpt_vstring(depth, "%s table:   NULL", title);
   }
}


#ifdef UNUSED_VARIANT
void dbgrpt_sl_value_table_dup(DDCA_Feature_Value_Entry * table, int depth) {
   rpt_vstring(depth, "DDCA_Feature_Value_Table at %p", table);
   if (table) {
      DDCA_Feature_Value_Entry * cur = table;
      while (cur->value_code != 0x00 || cur->value_name) {
         rpt_vstring(depth+1, "0x%02x - %s", cur->value_code, cur->value_name);
         cur++;
      }
   }
}
#endif


/** Make a deep copy of a feature value table
 *
 *  @param oldtable  pointer to first #DDCA_Feature_Value_Entry in table to copy
 *  @return copied table.
 */
DDCA_Feature_Value_Entry *
copy_sl_value_table(DDCA_Feature_Value_Entry * oldtable)
{
   bool debug = false;
   DBGMSF(debug, "Starting. oldtable=%p, size=%d", oldtable, sl_value_table_size(oldtable));
   DDCA_Feature_Value_Entry * newtable = NULL;
   if (oldtable) {
      int oldsize = sl_value_table_size(oldtable);
      newtable = calloc(oldsize, sizeof(DDCA_Feature_Value_Entry));
      DDCA_Feature_Value_Entry * oldentry = oldtable;
      DDCA_Feature_Value_Entry * newentry = newtable;
      int ct = 0;
      while(true) {
         ct++;
         newentry->value_code = oldentry->value_code;
         if (oldentry->value_name)
            newentry->value_name = strdup(oldentry->value_name);
         else
            break;
         oldentry++;
         newentry++;
      };
      DBGMSF(debug, "Copied %d entries, including terminating entry", ct);
   }

   DBGMSF(debug, "Done. Returning: %p", newtable);
   return newtable;
}


/** Free a feature value table.
 *
 *  @param table pointer to first entry in table
 */
void
free_sl_value_table(DDCA_Feature_Value_Entry * table) {
   bool debug = false;
   DBGMSF(debug, "Starting. table=%p", table);
   DBGMSF(debug, "table size = %d", sl_value_table_size(table));

   if (table) {
      DDCA_Feature_Value_Entry * cur = table;
      while(true) {
#ifdef OUT
         if (debug) {
            DBGMSG("cur=%p", cur);
            DBGMSG("cur->value_code = 0x%02x", cur->value_code);
            DBGMSG("cur->value_name = %p", cur->value_name);
            DBGMSG("cur->value_name -> %s", cur->value_name);
         }
#endif
         if (cur->value_name) {
            DBGMSF(debug, "Freeing: %p", cur->value_name);
            free(cur->value_name);
         }
         else
            break;
         cur++;
      }
      free(table);
   }

   DBGMSF(debug, "Done");
}


/* Given a hex value to be interpreted and an array of value table entries,
 * return the explanation string for value.
 *
 * Arguments:
 *    value_entries   array of Feature_Value_Entry
 *    value_id        value to look up
 *
 * Returns:
 *    explanation string from the Feature_Value_Entry found (do not free)
 *    NULL if not found
 */
char *
sl_value_table_lookup(
      DDCA_Feature_Value_Entry * value_entries,
      Byte                       value_id)
{
   // DBGMSG("Starting. pvalues_for_feature=%p, value_id=0x%02x", pvalues_for_feature, value_id);
   char * result = NULL;
   DDCA_Feature_Value_Entry *  cur_value = value_entries;
   while (cur_value->value_name != NULL) {
      // DBGMSG("value_code=0x%02x, value_name = %s", cur_value->value_code, cur_value->value_name);
      if (cur_value->value_code == value_id) {
         result = cur_value->value_name;
         // DBGMSG("Found");
         break;
      }
      cur_value++;
   }
   return result;
}


// DDCA_Feature_Metadata

/** Output a debug report of a #DDCA_Feature_Metadata instance
 *
 *  @param  md     instance to report
 *  @param  depth  logical indentation depth
 */
void
dbgrpt_ddca_feature_metadata(
      DDCA_Feature_Metadata * md,
      int                     depth)
{
   int d0 = depth;
   int d1 = depth+1;
   rpt_structure_loc("DDCA_Feature_Metadata", md, depth);
   rpt_vstring(d0, "Feature code:      0x%02x", md->feature_code);
   rpt_vstring(d1, "MCCS version:      %d.%d",  md->vcp_version.major, md->vcp_version.minor);
   rpt_vstring(d1, "Feature name:      %s",     md->feature_name);
   rpt_vstring(d1, "Description:       %s",     md->feature_desc);
   rpt_vstring(d1, "Feature flags:     0x%04x", md->feature_flags);
   rpt_vstring(d1, "Interpreted flags: %s", interpret_feature_flags_t(md->feature_flags));
   dbgrpt_sl_value_table(md->sl_values, "Feature values", d1);
   dbgrpt_sl_value_table(md->latest_sl_values, "Latest feature values", d1);
}


/** Frees a #DDCA_Feature_Metadata instance.
 *  Should never be called for permanent instances that are part of user defined
 *  feature records.
 *
 *  @param metadata  pointer to instance
 */
void
free_ddca_feature_metadata(DDCA_Feature_Metadata * metadata) {
   bool debug = false;
   DBGMSF(debug, "Executing. metadata = %p", metadata);
   if ( metadata && memcmp(metadata->marker, DDCA_FEATURE_METADATA_MARKER, 4) == 0) {
      if (debug)
         dbgrpt_ddca_feature_metadata(metadata, 2);
      assert(!(metadata->feature_flags & DDCA_PERSISTENT_METADATA));
      if (!(metadata->feature_flags & DDCA_PERSISTENT_METADATA)) {
         free(metadata->feature_name);
         free(metadata->feature_desc);
         free_sl_value_table(metadata->sl_values);
         // free_sl_value_table(metadata->latest_sl_values);
      }
      metadata->marker[3] = 'x';
   }
}


//
// Display_Feature_Metadata (used internally)
//

/** Emits a debug report on a #Display_Feature_Metadata instance.
 *  The report is written to the current report destination.
 *
 *  @param  meta   pointer to instance
 *  @param  depth  logical indentation depth
 */
void
dbgrpt_display_feature_metadata(
      Display_Feature_Metadata * meta,
      int                        depth)
{
   rpt_vstring(depth, "Display_Feature_Metadata at %p", meta);
   if (meta) {
      assert(memcmp(meta->marker, DISPLAY_FEATURE_METADATA_MARKER, 4) == 0);
      int d1 = depth+1;
      rpt_vstring(d1, "display_ref:     %s", dref_repr_t(meta->display_ref));
      rpt_vstring(d1, "feature_code:    0x%02x", meta->feature_code);
      rpt_vstring(d1, "vcp_version:     %d.%d = %s",
                      meta->vcp_version.major, meta->vcp_version.minor, format_vspec(meta->vcp_version));
      rpt_vstring(d1, "feature_name:    %s", meta->feature_name);
      rpt_vstring(d1, "feature_desc:    %s", meta->feature_desc);
      char * s = interpret_feature_flags_t(meta->feature_flags);
      rpt_vstring(d1, "flags:           0x%04x = %s", meta->feature_flags, s);
      dbgrpt_sl_value_table(meta->sl_values, "Feature values", d1);
      // dbgrpt_sl_value_table(meta->latest_sl_values, "Latest feature values", d1);
      rpt_vstring(d1, "nontable_formatter:           %p - %s",
                      meta->nontable_formatter,
                      rtti_get_func_name_by_addr(meta->nontable_formatter)) ;
      rpt_vstring(d1, "nontable_formatter_sl:        %p - %s",
                      meta->nontable_formatter_sl,
                      rtti_get_func_name_by_addr(meta->nontable_formatter_sl));
      rpt_vstring(d1, "nontable_formatter_universal: %p - %s",
                      meta->nontable_formatter_universal,
                      rtti_get_func_name_by_addr(meta->nontable_formatter_universal)) ;   // the future
      rpt_vstring(d1, "table_formatter:              %p - %s",
                      meta->table_formatter,
                      rtti_get_func_name_by_addr(meta->table_formatter));
   }
}


/** Frees a #Display_Feature_Metadata instance.
 *
 *  @param meta pointer to instance
 */
void
dfm_free(
      Display_Feature_Metadata * meta)
{
   bool debug = false;
   DBGMSF(debug, "Executing. meta=%p", meta);
   if (debug)
      dbgrpt_display_feature_metadata(meta, 2);
   if (meta) {
      assert(memcmp(meta->marker, DISPLAY_FEATURE_METADATA_MARKER, 4) == 0);
      meta->marker[3] = 'x';
      free(meta->feature_name);
      free(meta->feature_desc);
      free_sl_value_table(meta->sl_values);
      // free_sl_value_table(meta->latest_sl_values);
      free(meta);
   }
}


/** Common allocation and basic initialization for #Display_Feature_Metadata.
 *
 *  @param feature_code
 *  @return newly allocated #Display_Feature_Metadata
 *
 *  @remark sets marker and feature_code fields, all other fields to 0
 */
Display_Feature_Metadata *
dfm_new(
      DDCA_Vcp_Feature_Code feature_code)
{
   Display_Feature_Metadata * result = calloc(1, sizeof(Display_Feature_Metadata));
   memcpy(result->marker, DISPLAY_FEATURE_METADATA_MARKER, 4);
   result->feature_code = feature_code;
   return result;
}

#ifdef UNUSED
void dfm_set_feature_name(Display_Feature_Metadata * meta, const char * feature_name)
{
   assert(meta);
   if (meta->feature_name)
      free(meta->feature_name);
   meta->feature_name = strdup(feature_name);
}

void dfm_set_feature_desc(Display_Feature_Metadata * meta, const char * feature_desc)
{
   assert(meta);
   if (meta->feature_desc)
      free(meta->feature_desc);
   meta->feature_desc = strdup(feature_desc);
}
#endif


/** Converts a #Display_Feature_Metadata to a DDCA_Feature_Metadata
 *
 *  @param ddca_meta  instance to convert
 *  @result newly created #DDCA_Feature_Metadata
 *
 *  @remark
 *  It is the responsibility of the caller to free the returned instance
 */
DDCA_Feature_Metadata *
dfm_to_ddca_feature_metadata(
      Display_Feature_Metadata * dfm)
{
   bool debug = false;
   DBGMSF(debug, "Starting. dfm=%p", dfm);
   if (debug)
      dbgrpt_display_feature_metadata(dfm, 2);

   DDCA_Feature_Metadata * ddca_meta = calloc(1, sizeof(DDCA_Feature_Metadata));
   memcpy(ddca_meta->marker, DDCA_FEATURE_METADATA_MARKER, 4);
   ddca_meta->feature_code  = dfm->feature_code;
   ddca_meta->vcp_version   = dfm->vcp_version;
   ddca_meta->feature_flags = dfm->feature_flags;
   ddca_meta->feature_name = (dfm->feature_name) ? strdup(dfm->feature_name) : NULL;
   ddca_meta->feature_desc = (dfm->feature_desc) ? strdup(dfm->feature_desc) : NULL;
   DBGMSF(debug, "** dfm->sl_values = %p", dfm->sl_values);
   ddca_meta->sl_values = copy_sl_value_table(dfm->sl_values);
   // ddca_meta->feature_flags |= DDCA_SYNTHETIC_DDCA_FEATURE_METADATA;
   // ddca_meta->latest_sl_values = copy_sl_value_table(dfm->latest_sl_values);

   DBGMSF_RET_STRUCT(debug, DDCA_Feature_Metadata, dbgrpt_ddca_feature_metadata, ddca_meta);
   return ddca_meta;
}


/** Converts a #DDCA_Feature_Metadata to a #Display_Feature_Metadata.
 *
 *  @param ddca_meta  instance to convert
 *  @result newly created #Display_Feature_Metadata
 *
 *  @remark
 *  It is the responsibility of the caller to free the returned instance
 */
Display_Feature_Metadata *
dfm_from_ddca_feature_metadata(
      DDCA_Feature_Metadata * ddca_meta)
{
   bool debug = false;
   DBGMSF(debug, "Starting");
   assert(ddca_meta);
   assert(memcmp(ddca_meta->marker, DDCA_FEATURE_METADATA_MARKER, 4) == 0);

   Display_Feature_Metadata * dfm = dfm_new(ddca_meta->feature_code);
   dfm->display_ref = NULL;
   dfm->feature_desc = (ddca_meta->feature_desc) ? strdup(ddca_meta->feature_desc) : NULL;
   dfm->feature_name = (ddca_meta->feature_name) ? strdup(ddca_meta->feature_name) : NULL;
   // dfm->feature_flags = ddca_meta->feature_flags & ~DDCA_SYNTHETIC_DDCA_FEATURE_METADATA;
   dfm->feature_flags = ddca_meta->feature_flags & ~DDCA_PERSISTENT_METADATA;
   dfm->nontable_formatter = NULL;
   dfm->nontable_formatter_sl = NULL;
   dfm->table_formatter = NULL;
   dfm->vcp_version =  DDCA_VSPEC_UNQUERIED;
   dfm->sl_values = copy_sl_value_table(ddca_meta->sl_values);      // OR DUPLICATE?
   // dfm->latest_sl_values = copy_sl_value_table(ddca_meta->latest_sl_values);
   DBGMSF(debug, "Done. dfm=%p");
   return dfm;
}

