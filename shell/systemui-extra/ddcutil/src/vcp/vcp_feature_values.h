// vcp_feature_values.h

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VCP_FEATURE_VALUES_H_
#define VCP_FEATURE_VALUES_H_

#include <glib-2.0/glib.h>

#include "ddcutil_types.h"

#include "util/coredefs.h"
#include "util/data_structures.h"

#include "base/ddc_packets.h"
#include "base/feature_metadata.h"

#ifdef SINGLE_VCP_VALUE

// Removed from API due to complexity.  Used only internally.
/** Represents a single VCP value of any type */
typedef struct {
   DDCA_Vcp_Feature_Code  opcode;         /**< VCP feature code */
   DDCA_Vcp_Value_Type    value_type;      // probably a different type would be better
   union {
      struct {
         uint8_t *  bytes;          /**< pointer to bytes of table value */
         uint16_t   bytect;         /**< number of bytes in table value */
      }         t;                  /**< table value */
      struct {
         uint16_t   max_val;        /**< maximum value (mh, ml bytes) for continuous value */
         uint16_t   cur_val;        /**< current value (sh, sl bytes) for continuous value */
      }         c;                  /**< continuous (C) value */
      struct {
      // __BYTE_ORDER__ ifdef ensures proper overlay of ml/mh on max_val, sl/sh on cur_val
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
         uint8_t    ml;            /**< ML byte for NC value */
         uint8_t    mh;            /**< MH byte for NC value */
         uint8_t    sl;            /**< SL byte for NC value */
         uint8_t    sh;            /**< SH byte for NC value */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
         uint8_t    mh;
         uint8_t    ml;
         uint8_t    sh;
         uint8_t    sl;
#else
#error "Unexpected byte order value: __BYTE_ORDER__"
#endif
      }         nc;                /**< non-continuous (NC) value */
   }       val;
} Single_Vcp_Value;

#endif


char * vcp_value_type_name(DDCA_Vcp_Value_Type value_type);
char * vcp_value_type_id(DDCA_Vcp_Value_Type  value_type);


DDCA_Any_Vcp_Value *
create_nontable_vcp_value(
      Byte feature_code,
      Byte mh,
      Byte ml,
      Byte sh,
      Byte sl);

DDCA_Any_Vcp_Value *
create_cont_vcp_value(
      Byte     feature_code,
      ushort   max_val,
      ushort   cur_val);

DDCA_Any_Vcp_Value *
create_table_vcp_value_by_bytes(
      Byte   feature_code,
      Byte * bytes,
      ushort bytect);

DDCA_Any_Vcp_Value *
create_table_vcp_value_by_buffer(
      Byte     feature_code,
      Buffer*  buffer);

DDCA_Any_Vcp_Value *
create_single_vcp_value_by_parsed_vcp_response(
      Byte feature_id,
      Parsed_Vcp_Response * presp);

#ifdef UNUSED
Parsed_Vcp_Response *
single_vcp_value_to_parsed_vcp_response(
            DDCA_Any_Vcp_Value * valrec);
#endif

#ifdef MOVED_TO_FEATURE_METADATA
// Simple stripped-down version of Parsed_Nontable_Vcp_Response
// for use within vcp_feature_codes.c

typedef
struct {
   Byte   vcp_code;
   ushort max_value;
   ushort cur_value;
   // for new way
   Byte   mh;
   Byte   ml;
   Byte   sh;
   Byte   sl;
} Nontable_Vcp_Value;
#endif

Nontable_Vcp_Value *
      single_vcp_value_to_nontable_vcp_value(
                               DDCA_Any_Vcp_Value * valrec);
void  free_single_vcp_value(   DDCA_Any_Vcp_Value * vcp_value);
void  dbgrpt_single_vcp_value( DDCA_Any_Vcp_Value * valrec, int depth);

// void report_any_vcp_value(DDCA_Any_Vcp_Value * valrec, int depth);


extern const int summzrize_single_vcp_value_buffer_size;
char * summarize_single_vcp_value_r(DDCA_Any_Vcp_Value * valrec, char * buffer, int bufsz);
char * summarize_single_vcp_value(DDCA_Any_Vcp_Value * valrec);


// Vcp_Value_Set declarations

typedef GPtrArray *  Vcp_Value_Set;   // GPtrArray of DDCA_Single_Vcp_Value

Vcp_Value_Set vcp_value_set_new(int initial_size);

void free_vcp_value_set(Vcp_Value_Set vset);

void vcp_value_set_add(Vcp_Value_Set vset,
      DDCA_Any_Vcp_Value * pval);

int vcp_value_set_size(Vcp_Value_Set vset);

DDCA_Any_Vcp_Value *
vcp_value_set_get(Vcp_Value_Set vset, int ndx);

void dbgrpt_vcp_value_set(Vcp_Value_Set vset, int depth);

#endif /* SRC_VCP_VCP_FEATURE_VALUES_H_ */
