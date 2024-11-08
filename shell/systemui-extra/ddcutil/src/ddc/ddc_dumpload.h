/** \file dumpload.h
 *
 * Load/store VCP settings from/to file.
 */

// Copyright (C) 2014-2021 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DDC_DUMPLOAD_H_
#define DDC_DUMPLOAD_H_

/** \cond */
#include <stdio.h>

#include "util/error_info.h"
/** \endcond */

#include "base/displays.h"
#include "base/status_code_mgt.h"

#include "vcp/vcp_feature_values.h"


/**
Internal form data structure used to hold data being loaded or dumped.

Whatever the external form, a file or a string, loadvcp converts the data to
**Dumpload_Data** and then writes the data to the monitor.
*/
typedef
struct {
   time_t         timestamp_millis;       ///< creation timestamp
   Byte           edidbytes[128];         ///< 128 byte EDID,
   char           edidstr[257];           ///< 128 byte EDID as hex string (for future use)
   char           mfg_id[4];              ///< 3 character manufacturer id (from EDID)
   char           model[14];              ///< model string (from EDID)
   char           serial_ascii[14];       ///< serial number string (from EDID)
   uint16_t       product_code;           ///< numeric product code (from EDID)
   DDCA_MCCS_Version_Spec   vcp_version;  ///< monitor VCP/MCCS version
   int            vcp_value_ct;           ///< number of VCP values
   Vcp_Value_Set  vcp_values;             ///< VCP values
} Dumpload_Data;

void
dbgrpt_dumpload_data(Dumpload_Data * data, int depth);

void
free_dumpload_data(Dumpload_Data * pdata);

char *
format_timestamp(time_t time_millis, char * buf, int bufsz);

Error_Info *
loadvcp_by_dumpload_data(
      Dumpload_Data*   pdata,
      Display_Handle * dh);

Error_Info *
loadvcp_by_string(
      char *           catenated,
      Display_Handle * dh);

Error_Info *
create_dumpload_data_from_g_ptr_array(
      GPtrArray *      garray,
      Dumpload_Data ** dumpload_data_loc);

GPtrArray *
convert_dumpload_data_to_string_array(
      Dumpload_Data *  data);

Public_Status_Code
dumpvcp_as_dumpload_data(
      Display_Handle * dh,
      Dumpload_Data**  dumpload_data_loc);

Public_Status_Code
dumpvcp_as_string(
      Display_Handle * dh,
      char**           result_loc);

void init_ddc_dumpload();

#endif /* DDC_DUMPLOAD_H_ */
