/** @file dyn_feature_codes.h
 *
 * Access VCP feature code descriptions at the DDC level in order to
 * incorporate user-defined per-monitor feature information.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DYN_FEATURE_CODES_H_
#define DYN_FEATURE_CODES_H_

#include "ddcutil_types.h"

#include "base/feature_metadata.h"

#include "vcp/vcp_feature_codes.h"

#include "ddc/ddc_vcp_version.h"

Display_Feature_Metadata *
dyn_get_feature_metadata_by_mmk_and_vspec(
     DDCA_Vcp_Feature_Code       feature_code,
     DDCA_Monitor_Model_Key      mmk,
     DDCA_MCCS_Version_Spec      vspec,
     bool                        with_default);

Display_Feature_Metadata *
dyn_get_feature_metadata_by_dref(
      DDCA_Vcp_Feature_Code      id,
      Display_Ref *              dref,
      bool                       with_default);

Display_Feature_Metadata *
dyn_get_feature_metadata_by_dh(
      DDCA_Vcp_Feature_Code      id,
      Display_Handle *           dh,
      bool                       with_default);

bool
dyn_format_nontable_feature_detail(
      Display_Feature_Metadata * dfm,
      // DDCA_MCCS_Version_Spec  vcp_version,
      Nontable_Vcp_Value *       code_info,
      char *                     buffer,
      int                        bufsz);

bool
dyn_format_table_feature_detail(
      Display_Feature_Metadata * dfm,
      // DDCA_MCCS_Version_Spec  vcp_version,
      Buffer *                   accumulated_value,
      char * *                   aformatted_data);

bool
dyn_format_feature_detail(
      Display_Feature_Metadata * dfm,
      DDCA_MCCS_Version_Spec     vcp_version,
      DDCA_Any_Vcp_Value *       valrec,
      char * *                   aformatted_data);

char *
dyn_get_feature_name(
      Byte                       feature_code,
      Display_Ref*               dref);

bool dyn_format_feature_detail_sl_lookup(
      Nontable_Vcp_Value *       code_info,
      DDCA_Feature_Value_Entry * value_table,
      char *                     buffer,
      int                        bufsz);

void
init_dyn_feature_codes();

#endif /* DYN_FEATURE_CODES_H_ */
