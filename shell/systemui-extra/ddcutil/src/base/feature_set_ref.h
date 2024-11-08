/* \file feature_sets.h
 *
 * Feature set identifiers
 */

// Copyright (C) 2014-2018 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FEATURE_SET_REF_H_
#define FEATURE_SET_REF_H_

/** \cond */
#include <stdbool.h>

#include "util/coredefs.h"
#include "util/data_structures.h"
/** \endcond */


// If ids are added to or removed from this enum, be sure to update the
// corresponding tables in feature_sets.c, cmd_parser_aux.c
typedef enum {
   // ddcutil defined groups
   //                           0x80000000,   // unusable, error if high bit set
   VCP_SUBSET_PROFILE         = 0x40000000,
   VCP_SUBSET_COLOR           = 0x20000000,
   VCP_SUBSET_LUT             = 0x10000000,

   // MCCS spec groups
   VCP_SUBSET_CRT             = 0x08000000,
   VCP_SUBSET_TV              = 0x04000000,
   VCP_SUBSET_AUDIO           = 0x02000000,
   VCP_SUBSET_WINDOW          = 0x01000000,
   VCP_SUBSET_DPVL            = 0x00800000,
   VCP_SUBSET_PRESET          = 0x00400000,    // uses VCP_SPEC_PRESET

   // Subsets by feature type
   VCP_SUBSET_SCONT           = 0x00100000,    // simple Continuous feature
   VCP_SUBSET_CCONT           = 0x00080000,    // complex Continuous feature
   VCP_SUBSET_CONT            = 0x00040000,    // Continuous feature
   VCP_SUBSET_SNC             = 0x00020000,    // simple NC feature
   VCP_SUBSET_CNC             = 0x00010000,    // complex NC feature
   VCP_SUBSET_NC_WO           = 0x00008000,    // write-only NC feature
   VCP_SUBSET_NC_CONT         = 0x00004000,    // combines reserved values with a continuous subrange
   VCP_SUBSET_NC              = 0x00002000,    // Non-Continuous feature
   VCP_SUBSET_TABLE           = 0x00001000,    // is a table feature

   // subsets used only on command processing, not in feature descriptor table
   VCP_SUBSET_SCAN            = 0x00000040,
// VCP_SUBSET_ALL             = 0x00000040,
// VCP_SUBSET_SUPPORTED       = 0x00000020,
   VCP_SUBSET_KNOWN           = 0x00000020,

   VCP_SUBSET_MFG             = 0x00000010,    // mfg specific codes

   VCP_SUBSET_UDF             = 0x00000008,    // user defined features
   VCP_SUBSET_SINGLE_FEATURE  = 0x00000002,
   VCP_SUBSET_MULTI_FEATURES  = 0x00000001,    // user defined collection of features
   VCP_SUBSET_NONE            = 0x00000000,
} VCP_Feature_Subset;

extern const int vcp_subset_count;  // number of VCP_Feature_Subset values

char * feature_subset_name(VCP_Feature_Subset subset_id);
char * feature_subset_names(VCP_Feature_Subset subset_ids);

typedef struct {
   VCP_Feature_Subset  subset;
//    Byte                specific_feature;
   Bit_Set_256         features;             // for VCP_SUBSET_MULTI_FEATURES
} Feature_Set_Ref;

typedef enum {
   // apply to multiple feature feature sets
   FSF_SHOW_UNSUPPORTED      = 0x01,
   FSF_NOTABLE               = 0x02,
   FSF_RW_ONLY               = 0x04,
   FSF_RO_ONLY               = 0x08,
   FSF_WO_ONLY               = 0x10,

   // applies to single feature feature set
   FSF_FORCE                 = 0x20
} Feature_Set_Flags;
#define FSF_READABLE_ONLY    (FSF_RW_ONLY | FSF_RO_ONLY)

char * feature_set_flag_names_t(Feature_Set_Flags flags);

void dbgrpt_feature_set_ref(Feature_Set_Ref * fsref, int depth);
char * fsref_repr_t(Feature_Set_Ref * fsref);

#endif /* FEATURE_SET_REF_H_ */
