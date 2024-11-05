/** @file edid.h
 *
 * Functions for processing the Parsed_Edid data structure, irrespective of how
 * the bytes of the EDID are obtained.
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EDID_H_
#define EDID_H_

/** \cond */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>    // normally included by stdlib.h, but explicitly required for Alpine Linux
/** \endcond */

#include "coredefs.h"


// Field sizes for holding strings extracted from an EDID
// Note that these are sized to allow for a trailing null character.

#define EDID_MFG_ID_FIELD_SIZE        4
#define EDID_EXTRA_STRING_FIELD_SIZE 14
#define EDID_MODEL_NAME_FIELD_SIZE   14
#define EDID_SERIAL_ASCII_FIELD_SIZE 14
#define EDID_SOURCE_MAX_LEN           5
#define EDID_SOURCE_FIELD_SIZE        6

//Calculates checksum for a 128 byte EDID
Byte edid_checksum(Byte * edid);

bool is_valid_edid_checksum(Byte * edidbytes);
bool is_valid_edid_header(Byte * edidbytes);
bool is_valid_raw_edid(Byte * edidbytes, int len);
bool is_valid_raw_cea861_extension_block(Byte * edid, int len);

void parse_mfg_id_in_buffer(Byte * mfgIdBytes, char * buffer, int bufsize);

// Extracts the 3 character manufacturer id from an EDID byte array.
// The id is returned, with a trailing null character, in a buffer provided by the caller.
void get_edid_mfg_id_in_buffer(Byte* edidbytes, char * result, int bufsize);


#define EDID_MARKER_NAME "EDID"
/** Represents a parsed EDID */
typedef
struct {
   char         marker[4];                                   ///< always "EDID"
   Byte         bytes[128];                                  ///< raw bytes of EDID
   char         mfg_id[EDID_MFG_ID_FIELD_SIZE];              ///< 3 character mfg id, null terminated
   uint16_t       product_code;                                ///< product code number
   char         model_name[EDID_MODEL_NAME_FIELD_SIZE];      ///< model name (tag 0xfc)
   uint32_t     serial_binary;                               ///< binary serial number
   char         serial_ascii[EDID_SERIAL_ASCII_FIELD_SIZE];  ///< serial number string (tag 0xff)
   char         extra_descriptor_string[EDID_EXTRA_STRING_FIELD_SIZE];  ///< (tag 0xfe)
   int          year;                    ///< can be year of manufacture or model
   bool         is_model_year;           ///< if true, year is model year, if false, is manufacture year
   uint8_t      manufacture_week;        ///< xFF if is_model_year == true
   Byte         edid_version_major;      ///< EDID major version number
   Byte         edid_version_minor;      ///< EDID minor version number
   uint16_t       wx;                      ///< whitepoint x coordinate
   uint16_t       wy;                      ///< whitepoint y coordinate
   uint16_t       rx;                      ///< red   x coordinate
   uint16_t       ry;                      ///< red   y coordinate
   uint16_t       gx;                      ///< green x coordinate
   uint16_t       gy;                      ///< green y coordinate
   uint16_t       bx;                      ///< blue  x coordinate
   uint16_t       by;                      ///< blue  y coordinate
   Byte         video_input_definition;  /// EDID byte 20 (x14)
   // bool         is_digital_input;      // from byte 20 (x14), bit 7
   Byte         supported_features;      ///< EDID byte 24 (x18) supported features bitmap
   uint8_t      extension_flag;          ///< number of optional extension blocks
   char         edid_source[EDID_SOURCE_FIELD_SIZE];  ///< describes source of EDID
} Parsed_Edid;


Parsed_Edid * create_parsed_edid(Byte* edidbytes);
Parsed_Edid * create_parsed_edid2(Byte* edidbytes, char * source);
void          report_parsed_edid_base(Parsed_Edid * edid, bool verbose_synopsis, bool show_raw, int depth);
void          report_parsed_edid(Parsed_Edid * edid, bool verbose, int depth);
void          free_parsed_edid(Parsed_Edid * parsed_edid);
bool          is_embedded_parsed_edid(Parsed_Edid * parsed_edid);

#endif /* EDID_H_ */
