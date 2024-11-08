/** @file dyn_feature_set.c
 */

// Copyright (C) 2018-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "util/debug_util.h"
#include "util/report_util.h"

#include "base/displays.h"
#include "base/feature_lists.h"
#include "base/feature_metadata.h"
#include "base/feature_set_ref.h"

#include "vcp/vcp_feature_codes.h"
#include "dynvcp/dyn_feature_set.h"

static DDCA_Trace_Group TRACE_GROUP = DDCA_TRC_UDF;

void dbgrpt_dyn_feature_set(
      Dyn_Feature_Set * fset,
      bool              verbose,
      int               depth)
{
   int d0 = depth;
   int d1 = depth+1;

   rpt_vstring(d0, "Subset: %d (%s)", fset->subset, feature_subset_name(fset->subset));
   rpt_label  (d0, "Members (dfm):");
   for (int ndx=0; ndx < fset->members_dfm->len; ndx++) {
      Display_Feature_Metadata * dfm = g_ptr_array_index(fset->members_dfm,ndx);
      if (verbose)
         dbgrpt_display_feature_metadata(dfm, d1);
      else
         rpt_vstring(d1, "0x%02x - %s", dfm->feature_code, dfm->feature_name);
   }
}


char * dyn_feature_set_repr_t(Dyn_Feature_Set * fset) {
   static GPrivate  dynfs_repr_key = G_PRIVATE_INIT(g_free);

   char * buf = get_thread_fixed_buffer(&dynfs_repr_key, 200);
   snprintf(buf, 100, "[%s,%s]",  feature_subset_name(fset->subset), dref_repr_t(fset->dref));
   return buf;
}


static Display_Feature_Metadata *
dyn_create_dynamic_feature_from_dfr_metadata_dfm(DDCA_Feature_Metadata * dfr_metadata)
{
   bool debug = false;
   DBGMSF(debug, "Starting. id=0x%02x", dfr_metadata->feature_code);
   Display_Feature_Metadata * dfm = dfm_from_ddca_feature_metadata(dfr_metadata);

   if (dfr_metadata->feature_flags & DDCA_SIMPLE_NC) {
      if (dfr_metadata->sl_values)
         dfm->nontable_formatter_sl = dyn_format_feature_detail_sl_lookup;  // HACK
      else
         dfm->nontable_formatter = format_feature_detail_sl_byte;
   }
   else if (dfr_metadata->feature_flags & DDCA_STD_CONT)
      dfm->nontable_formatter = format_feature_detail_standard_continuous;
   else if (dfr_metadata->feature_flags & DDCA_TABLE)
      dfm->table_formatter = default_table_feature_detail_function;
   else
      dfm->nontable_formatter = format_feature_detail_debug_bytes;

   // pentry->vcp_global_flags = DDCA_SYNTHETIC;   // indicates caller should free
   // pentry->vcp_global_flags |= DDCA_USER_DEFINED;
   assert(dfm);
   if (debug || IS_TRACING()) {
      DBGMSF(debug, "Done.  Returning: %p", dfm);
      dbgrpt_display_feature_metadata(dfm, 1);
   }
   return dfm;
}

#ifdef UNUSED
static DDCA_Feature_Metadata *
dyn_create_feature_metadata_from_vcp_feature_table_entry(
      VCP_Feature_Table_Entry * pentry,
      DDCA_MCCS_Version_Spec    vspec)
{
   Display_Feature_Metadata * dfm =
     extract_version_feature_info_from_feature_table_entry(pentry, vspec, /*version_sensitive*/ true);

   if (pentry->vcp_global_flags & DDCA_SYNTHETIC_VCP_FEATURE_TABLE_ENTRY)
      free_synthetic_vcp_entry(pentry);

   DDCA_Feature_Metadata * meta = dfm_to_ddca_feature_metadata(dfm);
   dfm_free(dfm);
   return meta;
}
#endif


#ifdef UNUSED
Display_Feature_Metadata *
dyn_create_dynamic_feature_from_vcp_feature_table_entry_dfm(
      VCP_Feature_Table_Entry * vfte, DDCA_MCCS_Version_Spec vspec)
{
   assert(vfte);
   bool debug = false;
   DBGMSF(debug, "Starting. id=0x%02x", vfte->code);
   // Internal_Feature_Metadata * ifm = calloc(1, sizeof(Internal_Feature_Metadata));
   DDCA_Feature_Metadata * meta = dyn_create_feature_metadata_from_vcp_feature_table_entry(vfte, vspec);
   Display_Feature_Metadata * dfm = dfm_from_ddca_feature_metadata(meta);
   free_ddca_feature_metadata(meta);
   free(meta);

   if (dfm->feature_flags & DDCA_SIMPLE_NC) {
      if (dfm->sl_values)
         dfm->nontable_formatter_sl = dyn_format_feature_detail_sl_lookup;
      else
         dfm->nontable_formatter = format_feature_detail_sl_byte;
   }
   else if (dfm->feature_flags & DDCA_STD_CONT)
      dfm->nontable_formatter = format_feature_detail_standard_continuous;
   else if (dfm->feature_flags & DDCA_TABLE)
      dfm->table_formatter = default_table_feature_detail_function;
   else
      dfm->nontable_formatter = format_feature_detail_debug_bytes;

   // pentry->vcp_global_flags = DDCA_SYNTHETIC;   // indicates caller should free
   // pentry->vcp_global_flags |= DDCA_USER_DEFINED;
   assert(dfm);
   if (debug || IS_TRACING()) {
      DBGMSF(debug, "Done.  Returning: %p", dfm);
      dbgrpt_display_feature_metadata(dfm, 1);
   }
   return dfm;
}
#endif


static Dyn_Feature_Set *
dyn_create_feature_set0(
      VCP_Feature_Subset   subset_id,
      DDCA_Display_Ref     display_ref,
      GPtrArray *          members_dfm)
{
   bool debug = false;
   DBGTRC_STARTING(debug, TRACE_GROUP, "subset_id=%d, number of members=%d",
                              subset_id, (members_dfm) ? members_dfm->len : -1);

   Dyn_Feature_Set * fset = calloc(1,sizeof(Dyn_Feature_Set));
   memcpy(fset->marker, DYN_FEATURE_SET_MARKER, 4);
   fset->subset = subset_id;
   fset->members_dfm = members_dfm;

   DBGTRC_DONE(debug, TRACE_GROUP, "Returning %p", fset);
   return fset;
}


Dyn_Feature_Set *
dyn_create_feature_set(
      VCP_Feature_Subset     subset_id,
      DDCA_Display_Ref       display_ref,
      Feature_Set_Flags      feature_set_flags)
{
   bool debug = false;
   DBGMSF(debug, "Starting. subset_id=%d - %s, dref=%s, feature_setflags=0x%02x - %s",
                  subset_id,
                  feature_subset_name(subset_id),
                  dref_repr_t(display_ref),
                  feature_set_flags,
                  feature_set_flag_names_t(feature_set_flags));

    Dyn_Feature_Set * result = NULL;
    Display_Ref * dref = (Display_Ref *) display_ref;
    assert( dref && memcmp(dref->marker, DISPLAY_REF_MARKER, 4) == 0);

    GPtrArray * members_dfm = g_ptr_array_new();

    if (subset_id == VCP_SUBSET_UDF) {  // all user defined features
       DBGMSF(debug, "VCP_SUBSET_UDF path");

       if (dref->dfr) {
          DBGMSF(debug, "dref->dfr is set");
          GHashTableIter iter;
          gpointer hash_key;
          gpointer hash_value;
          g_hash_table_iter_init(&iter, dref->dfr->features);
          bool found = g_hash_table_iter_next(&iter, &hash_key, &hash_value);
          while (found) {
             DDCA_Feature_Metadata * feature_metadata = hash_value;
             assert( memcmp(feature_metadata, DDCA_FEATURE_METADATA_MARKER, 4) == 0 );

             // Test Feature_Set_Flags other than FSF_SHOW_UNSUPPORTED, FSF_FORCE,
             // which do not apply in this context
             bool include = true;
             //     Feature_Set_Flags                     //DDCA_Feature_Flags
             if ( ((feature_set_flags & FSF_NOTABLE) &&  (feature_metadata->feature_flags & DDCA_TABLE)) ||
                  ((feature_set_flags & FSF_RO_ONLY) && !(feature_metadata->feature_flags & DDCA_RO)   ) ||
                  ((feature_set_flags & FSF_RW_ONLY) && !(feature_metadata->feature_flags & DDCA_RW)   ) ||
                  ((feature_set_flags & FSF_WO_ONLY) && !(feature_metadata->feature_flags & DDCA_WO)   )
                )
                include = false;
             if (include) {
                Display_Feature_Metadata * dfm =
                   dyn_create_dynamic_feature_from_dfr_metadata_dfm(feature_metadata);
                g_ptr_array_add(members_dfm, dfm);
             }

             found = g_hash_table_iter_next(&iter, &hash_key, &hash_value);
          }
       }   // if (dref->dfr)
       result = dyn_create_feature_set0(subset_id, display_ref, members_dfm);
    }      // VCP_SUBSET_DYNAMIC

    else if (subset_id == VCP_SUBSET_SCAN) {
       VCP_Feature_Set * vcp_feature_set =
          create_vcp_feature_set(
             subset_id,
             // dref->vcp_version,
             get_vcp_version_by_dref(dref),
             feature_set_flags);
       int ct = get_vcp_feature_set_size(vcp_feature_set);
       for (int ndx = 0; ndx < ct; ndx++) {
           VCP_Feature_Table_Entry * vfte = get_vcp_feature_set_entry(vcp_feature_set, ndx);
           DDCA_Vcp_Feature_Code feature_code = vfte->code;
           Display_Feature_Metadata * dfm =
                 dyn_get_feature_metadata_by_dref(
                       feature_code,
                       dref,
                       true);    // with_default
           bool showit = true;
           if (feature_set_flags & FSF_RO_ONLY) {
              if ( !(dfm->feature_flags & DDCA_READABLE) )
                 showit = false;
           }
           if (showit)
              g_ptr_array_add(members_dfm, dfm);
           else
              dfm_free(dfm);
        }
        result = dyn_create_feature_set0(subset_id, display_ref, members_dfm);
        free_vcp_feature_set(vcp_feature_set);
    }

    else {   // (subset_id != VCP_SUBSET_DYNAMIC, != VCP_SUBSET_SCAN
       VCP_Feature_Set * vcp_feature_set =
          create_vcp_feature_set(
             subset_id,
             // dref->vcp_version,
             get_vcp_version_by_dref(dref),
             feature_set_flags);
       int ct = get_vcp_feature_set_size(vcp_feature_set);
       for (int ndx = 0; ndx < ct; ndx++) {
          VCP_Feature_Table_Entry * vfte = get_vcp_feature_set_entry(vcp_feature_set, ndx);
          DDCA_Vcp_Feature_Code feature_code = vfte->code;
          Display_Feature_Metadata * dfm =
                dyn_get_feature_metadata_by_dref(
                      feature_code,
                      dref,
                      true);    // with_default
          bool showit = true;
          // if ( !(dfm->feature_flags & DDCA_READABLE) )
          if (feature_set_flags & FSF_READABLE_ONLY) {
             if ( !(dfm->feature_flags & DDCA_READABLE) )
                showit = false;
          }
          if (showit)
             g_ptr_array_add(members_dfm, dfm);
          else
             dfm_free(dfm);
       }
       result = dyn_create_feature_set0(subset_id, display_ref, members_dfm);
       free_vcp_feature_set(vcp_feature_set);
    }

    if (debug) {
       DBGMSG("Returning: %p", result);
       if (result)
          dbgrpt_dyn_feature_set(result, false, 1);
    }
    return result;
}

#ifdef UNUSED
Dyn_Feature_Set *
dyn_create_single_feature_set_by_hexid2(
      DDCA_Vcp_Feature_Code  feature_code,
      DDCA_Display_Ref       display_ref,
      bool                  force)
{
   bool debug = false;
   DBGMSF(debug, "feature_code=0x%02x, display_ref=%s, force=%s",
                 feature_code, dref_repr_t(display_ref), sbool(force));
   Display_Ref * dref = (Display_Ref *) display_ref;
   assert( dref && memcmp(dref->marker, DISPLAY_REF_MARKER, 4) == 0);

   Dyn_Feature_Set * result = calloc(1, sizeof(Dyn_Feature_Set));
   memcpy(result->marker, DYN_FEATURE_SET_MARKER, 4);
   result->dref = dref;
   result->subset = VCP_SUBSET_SINGLE_FEATURE;
   result->members_dfm = g_ptr_array_new();
   Display_Feature_Metadata *  dfm = NULL;
   if (dref->dfr) {
      DDCA_Featurfree_dfm_funce_Metadata * feature_metadata  =
         get_dynamic_feature_metadata(dref->dfr, feature_code);
      if (feature_metadata) {
         dfm = dyn_create_dynamic_feature_from_dfr_metadata_dfm(feature_metadata);
      }
   }

   if (!dfm) {
      VCP_Feature_Table_Entry* vcp_entry = NULL;
       if (force)
          vcp_entry = vcp_find_feature_by_hexid_w_default(feature_code);
       else
          vcp_entry = vcp_find_feature_by_hexid(feature_code);
       if (vcp_entry)
          dfm = dyn_create_dynamic_feature_from_vcp_feature_table_entry_dfm(
                   vcp_entry,
                   get_vcp_version_by_dref(dref) );
                   // dref->vcp_version);
   }

   if (dfm)
      g_ptr_array_add(result->members_dfm, dfm);

   else {
      // free_dyn_feature_set(result)   ??
      // result = NULL  ??
   }

   DBGMSF(debug, "Done. Returning %p", result);
   return result;
}
#endif

// replaces
// VCP_Feature_Table_Entry * get_feature_set_entry(VCP_Feature_Set feature_set, unsigned index);

Display_Feature_Metadata *
dyn_get_feature_set_entry(
      Dyn_Feature_Set * feature_set,
      unsigned          index)
{
   assert(feature_set && feature_set->members_dfm);
   Display_Feature_Metadata * result = NULL;
   if (index < feature_set->members_dfm->len)
      result = g_ptr_array_index(feature_set->members_dfm, index);
   return result;
}


// replaces
// int get_feature_set_size(VCP_Feature_Set feature_set);

int
dyn_get_feature_set_size(
      Dyn_Feature_Set * feature_set)
{
   // show_backtrace(2);
   assert(feature_set);
   assert(feature_set->members_dfm);
   int result = feature_set->members_dfm->len;
   return result;
}

#ifdef UNUSED
Dyn_Feature_Set *
dyn_create_feature_set_from_feature_set_ref2(
   Feature_Set_Ref *       fsref,
   DDCA_Display_Ref        dref,
   Feature_Set_Flags       flags)
{
   bool debug = false;
   DBGTRC(debug, TRACE_GROUP, "Starting. fsref=%s, dref=%s, flags=%s",
          fsref_repr_t(fsref), dref_repr_t(dref), interpret_feature_flags_t(flags));

   Dyn_Feature_Set* result = NULL;
   if (fsref->subset == VCP_SUBSET_SINGLE_FEATURE) {
      // iftests within function
      result = dyn_create_single_feature_set_by_hexid2(fsref->specific_feature, dref, flags & FSF_FORCE);
   }
   else {
      result = dyn_create_feature_set(fsref->subset, dref, flags);
   }

   if (debug || IS_TRACING()) {
      DBGMSG("Returning VCP_Feature_Set %p",  result);
      if (result)
         dbgrpt_dyn_feature_set(result, true, 1);
   }
   return result;
}
#endif


// wrap dfm_free() in signature of GDestroyNotify()
static void
free_dfm_func(gpointer data) {
   dfm_free((Display_Feature_Metadata *) data);
}


void dyn_free_feature_set(
      Dyn_Feature_Set * feature_set)
{
   bool debug = false;
   DBGMSF(debug, "Starting. feature_set=%s", dyn_feature_set_repr_t(feature_set));
   if (feature_set->members_dfm) {
      g_ptr_array_set_free_func(feature_set->members_dfm, free_dfm_func);
      g_ptr_array_free(feature_set->members_dfm,true);
   }
   free(feature_set);
   DBGMSF(debug, "Done");
}

