/* demo_feature_lists.c
 *
 * Demonstrate feature list functions.
 */

// Copyright (C) 2018-2020 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public/ddcutil_c_api.h"


void assert_ddcrc_ok(DDCA_Status ddcrc, const char * ddc_func, const char * caller) {
    if (ddcrc) {
        printf("Error in %s(): %s() returned %d (%s): %s\n",
               caller, ddc_func, ddcrc, ddca_rc_name(ddcrc), ddca_rc_desc(ddcrc));
        exit(1);
    }
}

void report_ddcrc_error(DDCA_Status ddcrc, const char * ddc_func, const char * caller) {
    if (ddcrc) {
        printf("Error in %s(): %s() returned %d (%s): %s\n",
               caller, ddc_func, ddcrc, ddca_rc_name(ddcrc), ddca_rc_desc(ddcrc));
    }
}



DDCA_Display_Ref  get_dref_by_dispno(int dispno) {
   printf("Getting display reference for display %d...\n", dispno);
   DDCA_Display_Identifier did;
   DDCA_Display_Ref        dref = NULL;

   ddca_create_dispno_display_identifier(1, &did);     // always succeeds
   DDCA_Status rc = ddca_get_display_ref(did, &dref);
   report_ddcrc_error(rc, "ddca_create_display_ref", __func__);
   assert( (rc==0 && dref) || (rc!=0 && !dref));
   return dref;
}



void demo_feature_lists_for_dref(DDCA_Display_Ref dref) {
   DDCA_Status ddcrc = 0;

   // Note that the defined features vary by MCCS version.
   // In fact whether a feature is of type Table can vary by
   // MCCS version.

   // get the feature list for feature set PROFILE
   DDCA_Feature_List vcplist1;
   ddcrc = ddca_get_feature_list_by_dref(
         DDCA_SUBSET_PROFILE,
         dref,
         false,                  // exclude table features
         &vcplist1);
   assert_ddcrc_ok(ddcrc, "ddca_get_feature_list_by_dref",__func__);        // this is sample code

   // alternatively, use convenience function ddca_feature_list_string(), see below
   printf("\nFeatures in feature set PROFILE:\n   ");
   for (int ndx = 0; ndx < 256; ndx++) {
      if (ddca_feature_list_contains(vcplist1, ndx))
         printf(" %02x", ndx);
   }
   puts("");

   // Assume we have read the values for the VCP features in PROFILE.
   // The user then changes the feature set to COLOR

   DDCA_Feature_List vcplist2;
   ddcrc = ddca_get_feature_list_by_dref(
         DDCA_SUBSET_COLOR,
         dref,
         false,               // exclude table features
         &vcplist2);
   assert_ddcrc_ok(ddcrc, "ddca_get_feature_list",__func__);        // this is sample code

   printf("\nFeatures in feature set COLOR:\n   ");
   for (int ndx = 0; ndx < 256; ndx++) {
      if (ddca_feature_list_contains(vcplist2, ndx))
         printf(" %02x", ndx);
   }
   puts("");

   // We only would need to get read the features that have not yet been read
   DDCA_Feature_List vcplist3 = ddca_feature_list_and_not(vcplist2, vcplist1);

   printf("\nFeatures in feature set COLOR but not in PROFILE:\n   ");
   // a convenience function:
   const char * s = ddca_feature_list_string(vcplist3, /*prefix=*/ "", /*sepstr=*/ " ");
   printf("%s\n", s);
}


int main(int argc, char** argv) {
    // Feature group definitions can be VCP version sensitive.
    // In real code, we'd get the MCCS version from the monitor information.

    DDCA_Display_Ref dref = get_dref_by_dispno(1);
    assert(dref);      // there's at least 1 display

    demo_feature_lists_for_dref(dref);

   return 0;
}
