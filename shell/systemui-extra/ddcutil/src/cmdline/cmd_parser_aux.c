/** \file cmd_parser_aux.c
 *
 *  Functions and strings that are independent of the parser package used.
 */

// Copyright (C) 2014-2023 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <config.h>

/** \cond */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/string_util.h"
/** \endcond */

#include "base/parms.h"

#include "cmdline/cmd_parser_aux.h"

//
// Command Description data structure
//

static Cmd_Desc cmdinfo[] = {
 // cmd_id              cmd_name   minchars min_arg_ct max_arg_ct          supported_options
   {CMDID_DETECT,       "detect",         3,  0,       0,                  Option_None},
   {CMDID_CAPABILITIES, "capabilities",   3,  0,       0,                  Option_Explicit_Display},
   {CMDID_GETVCP,       "getvcp",         3,  1,       MAX_GETVCP_VALUES,  Option_Explicit_Display},
   {CMDID_SETVCP,       "setvcp",         3,  2,       MAX_SETVCP_VALUES*2,Option_Explicit_Display},
   {CMDID_LISTVCP,      "listvcp",        5,  0,       0,                  Option_None},
#ifdef INCLUDE_TESTCASES
   {CMDID_TESTCASE,     "testcase",       3,  1,       1,                  Option_None},
   {CMDID_LISTTESTS,    "listtests",      5,  0,       0,                  Option_None},
#endif
   {CMDID_LOADVCP,      "loadvcp",        3,  1,       1,                  Option_Explicit_Display},
   {CMDID_DUMPVCP,      "dumpvcp",        3,  0,       1,                  Option_Explicit_Display},
#ifdef ENABLE_ENVCMDS
   {CMDID_INTERROGATE,  "interrogate",    3,  0,       0,                  Option_None},
   {CMDID_ENVIRONMENT,  "environment",    3,  0,       0,                  Option_None},
   {CMDID_USBENV,       "usbenvironment", 6,  0,       0,                  Option_None},
#endif
   {CMDID_VCPINFO,      "vcpinfo",        5,  0,       MAX_GETVCP_VALUES,  Option_None},
#ifdef WATCH_COMMAND
   {CMDID_READCHANGES,  "watch",          3,  0,       0,                  Option_Explicit_Display},
#endif
#ifdef USE_USB
   {CMDID_CHKUSBMON,    "chkusbmon",      3,  1,       1,                  Option_None},
#endif
   {CMDID_PROBE,        "probe",          5,  0,       0,                  Option_Explicit_Display},
   {CMDID_SAVE_SETTINGS,"scs",            3,  0,       0,                  Option_Explicit_Display},
};
static int cmdct = sizeof(cmdinfo)/sizeof(Cmd_Desc);


#ifndef NDEBUG
/** Validates the Cmd_Desc data structure.
 *  Called during parser initialization.
 */
void validate_cmdinfo() {
   int ndx = 0;
   for (; ndx < cmdct; ndx++) {
      assert( cmdinfo[ndx].max_arg_ct <= MAX_ARGS);
   }
}
#endif


/** Reports the contents of the Cmd_Desc data structure.
 *  For debugging.
 *
 *  @param  cmd_desc pointer to command table
 */
void show_cmd_desc(Cmd_Desc * cmd_desc) {
   printf("CmdDesc at %p\n", (void*)cmd_desc);
   printf("   cmd_id:     0x%04x\n", cmd_desc->cmd_id);
   printf("   cmd_name:   %s\n",     cmd_desc->cmd_name);
   printf("   minchars:   %d\n",     cmd_desc->minchars);
   printf("   min_arg_ct: %d\n",     cmd_desc->min_arg_ct);
   printf("   max_arg_ct: %d\n",     cmd_desc->max_arg_ct);
}


/** Looks for a command in the command table, allowing for valid abbreviations.
 *
 *  @param  cmd  pointer to command specified by the user, in lower case
 *  @return pointer to #Cmd_Desc struct for the command, NULL if not found
 */
Cmd_Desc * find_command(char * cmd) {
   Cmd_Desc * result = NULL;
   int ndx = 0;
   for (; ndx < cmdct; ndx++) {
      Cmd_Desc desc = cmdinfo[ndx];
      if (is_abbrev(cmd, desc.cmd_name, desc.minchars)) {
         result = &cmdinfo[ndx];
      }
   }
   return result;
}


/** Looks for a command in the command table by its command id.
 *  @param  cmdid id number of command
 *  @return pointer to #Cmd_Desc struct for the command, NULL if not found
 */
Cmd_Desc * get_command(int cmdid) {
   bool debug = false;
   Cmd_Desc * result = NULL;
   int ndx = 0;
   for (; ndx < cmdct; ndx++) {
      Cmd_Desc desc = cmdinfo[ndx];
      if (cmdid == desc.cmd_id)  {
         result = &cmdinfo[ndx];
      }
   }
   if (debug) {
      DBGMSG("cmdid=0x%04x, returning %p", cmdid, result);
      show_cmd_desc(result);
   }
   return result;
}

// End of command description data structure


//
// Parsers for specific argument types
//

bool all_digits(char * val, int ct) {
   bool debug = false;
   DBGMSF(debug, "ct-%d, val -> |%.*s|", ct, ct, val );
   bool ok = true;
   int ndx;
   for (ndx = 0; ndx < ct; ndx++) {
      if ( !isdigit(*(val+ndx)) ) {
         ok = false;
         break;
      }
   }
   DBGMSF(debug, "Returning: %d  ", ok );
   return ok;
}


bool parse_dot_separated_arg(const char * val, int * piAdapterIndex, int * piDisplayIndex) {
   int rc = sscanf(val, "%d.%d", piAdapterIndex, piDisplayIndex);
   // DBGMSG("val=|%s| sscanf() returned %d  ", val, rc );
   bool ok = (rc == 2);
   return ok;
}


bool parse_colon_separated_arg(const char * val, int * pv1, int * pv2) {
   int rc = sscanf(val, "%d:%d", pv1, pv2);
   // DBGMSG("val=|%s| sscanf() returned %d  ", val, rc );
   bool ok = (rc == 2);
   return ok;
}


bool parse_int_arg(char * val, int * pIval) {
   int ct = sscanf(val, "%d", pIval);
   return (ct == 1);
}


//
// Feature subset table and functions
//

typedef struct feature_subset_table_entry_s {
   VCP_Feature_Subset   subset_id;
   Cmd_Id_Type          valid_commands;
   int                  min_chars;
   char *               subset_name;
   char *               subset_desc;
} Feature_Subset_Table_Entry;

// Valid subset identifiers for commands that can take a subset id as an argument
const Feature_Subset_Table_Entry subset_table[] = {
   // special handling
   {VCP_SUBSET_KNOWN,     CMDID_GETVCP|CMDID_VCPINFO, 3, "KNOWN",     "All features known to ddcutil that are valid for the display"},
   {VCP_SUBSET_KNOWN,     CMDID_GETVCP|CMDID_VCPINFO, 3, "ALL",       "Same as KNOWN"},
   {VCP_SUBSET_KNOWN,     CMDID_GETVCP|CMDID_VCPINFO, 3, "MCCS",      "Same as KNOWN"},
// {VCP_SUBSET_SUPPORTED, CMDID_GETVCP,               3, "SUPPORTED", "All known features that are valid for the display"},
   {VCP_SUBSET_SCAN,      CMDID_GETVCP,               3, "SCAN",      "All feature codes 00..FF, except those known to be WO"},
   {VCP_SUBSET_MFG,       CMDID_GETVCP,               3, "MANUFACTURER", "Manufacturer specific codes"},
   {VCP_SUBSET_MFG,       CMDID_GETVCP,               3, "MFG",        "Same as MANUFACTURER"},
   {VCP_SUBSET_UDF,   CMDID_GETVCP|CMDID_VCPINFO, 3, "UDF",        "User defined features"},
   {VCP_SUBSET_UDF,   CMDID_GETVCP|CMDID_VCPINFO, 3, "USER",       "User defined features"},

   // ddcutil defined groups
   {VCP_SUBSET_PROFILE,   CMDID_GETVCP|CMDID_VCPINFO, 3, "PROFILE",   "Features for color profile management"},
   {VCP_SUBSET_COLOR,     CMDID_GETVCP|CMDID_VCPINFO, 3, "COLOR",     "Color related features"},
   {VCP_SUBSET_LUT,       CMDID_GETVCP|CMDID_VCPINFO, 3, "LUT",       "LUT related features"},

   // by MCCS spec group
   {VCP_SUBSET_CRT,       CMDID_GETVCP|CMDID_VCPINFO, 3, "CRT",       "CRT related features"},
   {VCP_SUBSET_AUDIO,     CMDID_GETVCP|CMDID_VCPINFO, 3, "AUDIO",     "Audio related features"},
   {VCP_SUBSET_WINDOW,    CMDID_GETVCP|CMDID_VCPINFO, 3, "WINDOW",    "Window related features"},
   {VCP_SUBSET_TV,        CMDID_GETVCP|CMDID_VCPINFO, 2, "TV",        "TV related features"},
   {VCP_SUBSET_DPVL,      CMDID_GETVCP|CMDID_VCPINFO, 3, "DPVL",      "DPVL related features"},
   {VCP_SUBSET_PRESET,                 CMDID_VCPINFO, 3, "PRESET",    "Presets"},     // all WO

   // by feature type
   {VCP_SUBSET_TABLE,     CMDID_GETVCP|CMDID_VCPINFO, 3, "TABLE",     "Table type features"},
   {VCP_SUBSET_SCONT,     CMDID_GETVCP|CMDID_VCPINFO, 3, "SCONT",     "Simple Continuous features"},
   {VCP_SUBSET_CCONT,     CMDID_GETVCP|CMDID_VCPINFO, 3, "CCONT",     "Complex Continuous features"},
   {VCP_SUBSET_CONT,      CMDID_GETVCP|CMDID_VCPINFO, 3, "CONT",      "All Continuous features"},
   {VCP_SUBSET_SNC,       CMDID_GETVCP|CMDID_VCPINFO, 3, "SNC",       "Simple NC features"},
   {VCP_SUBSET_CNC,       CMDID_GETVCP|CMDID_VCPINFO, 3, "CNC",       "Complex NC features"},
   {VCP_SUBSET_NC_WO,                  CMDID_VCPINFO, 4, "NC_WO",     "Write-only NC features"},
   {VCP_SUBSET_NC_CONT,   CMDID_GETVCP|CMDID_VCPINFO, 4, "NC_CONT",   "NC features with continuous subrange"},
   {VCP_SUBSET_NC,        CMDID_GETVCP|CMDID_VCPINFO, 2, "NC",        "All NC features"},
};
const int subset_table_ct = sizeof(subset_table)/sizeof(Feature_Subset_Table_Entry);

#ifndef NDEBUG
void validate_subset_table() {
   // quick and dirty check that tables are in sync
   // +3 for VCP_SUBSET_SINGLE_FEATURE, VCP_SUBSET_MULTI_FEATURE, VCP_SUBSET_NONE
   // -2 for triple VCP_SUBSET_KNOWN
   // -1 for double VCP_SUBSET_MFG
   // -1 for double VCP_SUBSET_DYNAMIC
   assert(subset_table_ct+(3-4) == vcp_subset_count);
}
#endif

/** Assemble a help message listing valid feature subsets.
 *  @return multi-line help string
 */
char * assemble_command_argument_help() {
   GString * buf = g_string_sized_new(1000);
   g_string_append(buf,
         "Command Arguments\n"
         "  getvcp, vcpinfo:\n"
         "    <feature-code-or-group> can be any of the following:\n"
         "      - the hex feature code for a specific feature, with or without a leading 0x,\n"
         "        e.g. 10 or 0x10\n");

   for (int ndx = 0; ndx < subset_table_ct; ndx++) {
      g_string_append_printf(buf, "      - %-10s - %s\n",  subset_table[ndx].subset_name, subset_table[ndx].subset_desc);
   }
   g_string_append(buf,
   "    Keywords can be abbreviated to the first 3 characters.\n"
   "    Case is ignored.  e.g. \"COL\", \"pro\"\n"
   "\n"
   "  setvcp:\n"
   "    <feature-code>: hexadecimal feature code, with or without a leading 0x,\n"
   "       e.g. 10 or 0x10\n"
   "    [+|-] optionally indicate a relative value change, must be surrounded by blanks\n"
   "    <new-value>: a decimal number in the range 0..255, or a single byte hex value,\n"
   "       e.g. 0x80\n");

   char * result = buf->str;
   g_string_free(buf, false);
   // DBGMSG("Returning: |%s|", result);
   return result;
}


/** Finds a subset identifier description.
 *  Given a command
 */

VCP_Feature_Subset find_subset(char * name, int cmd_id) {
   assert(name && (cmd_id == CMDID_GETVCP || cmd_id == CMDID_VCPINFO));
   VCP_Feature_Subset result = VCP_SUBSET_NONE;
   char * us = strdup_uc(name);
   int ndx = 0;
   for (;ndx < subset_table_ct; ndx++) {
      if ( is_abbrev(us, subset_table[ndx].subset_name, subset_table[ndx].min_chars) ) {
         if (cmd_id & subset_table[ndx].valid_commands)
            result = subset_table[ndx].subset_id;
         break;
      }
   }
   free(us);
   return result;
}

#ifdef OLD
bool parse_feature_id_or_subset(char * val, int cmd_id, Feature_Set_Ref * fsref) {
   bool debug = true;
   bool ok = true;
   VCP_Feature_Subset subset_id = find_subset(val, cmd_id);
   if (subset_id != VCP_SUBSET_NONE)
      fsref->subset = subset_id;
   else {
     Byte feature_hexid = 0;   // temp
     ok = any_one_byte_hex_string_to_byte_in_buf(val, &feature_hexid);
     if (ok) {
        fsref->subset = VCP_SUBSET_SINGLE_FEATURE;
        fsref->specific_feature = feature_hexid;
        bs256_insert(fsref->features, feature_hexid);     // for future
     }
   }
   DBGMSF(debug, "Returning: %s", sbool(ok));
   if (ok && debug)
      dbgrpt_feature_set_ref(fsref, 0);
   return ok;
}

bool parse_feature_ids(char ** vals, int vals_ct, int cmd_id, Feature_Set_Ref * fsref) {
   bool debug = true;
   DBGMSF(debug, "Starting. vals_ct=%d, cmd_id=%d, fsref=%p", vals_ct, cmd_id, fsref);
   bool ok = true;
   assert(cmd_id == CMDID_GETVCP || cmd_id == CMDID_VCPINFO);
   assert(vals_ct > 0);
   fsref->subset = VCP_SUBSET_NONE;
   for (int ndx = 0; ndx < vals_ct; ndx++) {
      Byte feature_hexid = 0;   // temp
      ok = any_one_byte_hex_string_to_byte_in_buf(vals[ndx], &feature_hexid);
      DBGMSF(debug, "vals[ndx]=%s, ok=%s, feature_hexid=0x%02x", vals[ndx], sbool(ok), feature_hexid);
      if (ok) {
         fsref->features = bs256_insert(fsref->features, feature_hexid);
      }
   }
   if (ok)
      fsref->subset = VCP_SUBSET_MULTI_FEATURES;

   DBGMSF(debug, "Returning: %s", sbool(ok));
   if (ok && debug)
      dbgrpt_feature_set_ref(fsref, 0);

   return ok;
}
#endif


Feature_Set_Ref * parse_feature_ids_or_subset(int cmd_id, char **vals, int vals_ct) {
   bool debug = false;
   DBGMSF(debug, "cmd_id=%d, vals[0]=%s, vals_ct=%d", cmd_id, vals[0], vals_ct);

   Feature_Set_Ref * fsref = calloc(1, sizeof(Feature_Set_Ref));
   bool ok = false;
   if (vals_ct <= 1) {
      char * val = (vals_ct > 0) ? vals[0] : "ALL";
      VCP_Feature_Subset subset_id = find_subset(val, cmd_id);
      if (subset_id != VCP_SUBSET_NONE) {
         fsref->subset = subset_id;
         ok = true;
      }
      else {
         Byte feature_hexid = 0;   // temp
         ok = any_one_byte_hex_string_to_byte_in_buf(val, &feature_hexid);
         if (ok) {
            fsref->subset = VCP_SUBSET_SINGLE_FEATURE;
            // fsref->specific_feature = feature_hexid;
            fsref->features = bs256_insert(fsref->features, feature_hexid);     // for future
         }
      }
   }
   else {   // ct > 1
      // ok = parse_feature_ids(vals, vals_ct, cmd_id, fsref);
      assert(cmd_id == CMDID_GETVCP || cmd_id == CMDID_VCPINFO);
      fsref->subset = VCP_SUBSET_NONE;
      for (int ndx = 0; ndx < vals_ct; ndx++) {
         Byte feature_hexid = 0;   // temp
         ok = any_one_byte_hex_string_to_byte_in_buf(vals[ndx], &feature_hexid);
         DBGMSF(debug, "vals[ndx]=%s, ok=%s, feature_hexid=0x%02x", vals[ndx], sbool(ok), feature_hexid);
         if (ok) {
            fsref->features = bs256_insert(fsref->features, feature_hexid);
         }
      }
      if (ok)
         fsref->subset = VCP_SUBSET_MULTI_FEATURES;
   }

   if (!ok) {
      free(fsref);
      fsref = NULL;
   }
   DBGMSF(debug, "Done.  Returning: %p", (void*) fsref);
   if (ok && debug)
      dbgrpt_feature_set_ref(fsref, 1);
   return fsref;
}



/** Checks that the output level, after parsing, is appropriate for a command.
 *  Writes a message to stdout if not.
 *
 *  @param   parsed_cmd  pointer to a #Parsed_Cmd
 *  @return  true/false
 */
bool validate_output_level(Parsed_Cmd* parsed_cmd) {
   bool ok = true;
   Byte valid_output_levels;
   // once upon a time the switch statement really had multiple cases
   switch(parsed_cmd->cmd_id) {
      case (CMDID_PROBE):
         // don't want to deal with how to report errors, handle write-only features
         // of machine readable output triggered by --terse
         valid_output_levels =                 DDCA_OL_NORMAL | DDCA_OL_VERBOSE | DDCA_OL_VV;
         break;
      default:
         valid_output_levels = DDCA_OL_TERSE | DDCA_OL_NORMAL | DDCA_OL_VERBOSE | DDCA_OL_VV;
   }
   if (!(parsed_cmd->output_level & valid_output_levels)) {
      printf("Output level invalid for command %s: %s\n",
             get_command(parsed_cmd->cmd_id)->cmd_name,
             output_level_name(parsed_cmd->output_level) );
      ok = false;
   }
   return ok;
}


//
// Help message fragments
//
char * commands_list_help =
       "Commands:\n"
       "   detect                                  Detect monitors\n"
       "   capabilities                            Query monitor capabilities string\n"
//     "   info\n"
//     "   listvcp\n"
       "   vcpinfo (feature-code-or-group)         Show VCP feature characteristics\n"
       "   getvcp <feature-code-or-group>          Report VCP feature value(s)\n"
       "   setvcp <feature-code> [+|-] <new-value> Set VCP feature value\n"
       "   dumpvcp (filename)                      Write color profile related settings to file\n"
       "   loadvcp <filename>                      Load profile related settings from file\n"
       "   scs                                     Store current settings in monitor's nonvolatile storage\n"
#ifdef INCLUDE_TESTCASES
       "   testcase <testcase-number>\n"
       "   listtests\n"
#endif
#ifdef ENABLE_ENVCMDS
       "   environment                             Probe execution environment\n"
#ifdef USE_USB
       "   usbenv                                  Probe for USB connected monitors\n"
#endif
#endif
       "   probe                                   Probe monitor abilities\n"
#ifdef ENABLE_ENVCMDS
       "   interrogate                             Report everything possible\n"
#endif
#ifdef USE_USB
       "   chkusbmon                               Check if USB device is monitor (for UDEV)\n"
#endif
#ifdef DEPRECATED
       "   watch                                   Watch display for reported changes (under development)\n"
#endif
      "\n";

#ifdef OLD
char * command_argument_help =
       "Command Arguments\n"
       "  getvcp, vcpinfo:\n"
       "    <feature-code-or-group> can be any of the following:\n"
       "      - the hex feature code for a specific feature, with or without a leading 0x,\n"
       "        e.g. 10 or 0x10\n"
       "      - KNOWN        - all feature codes known to ddcutil\n"
       "      - ALL          - like KNOWN, but implies --show-unsupported\n"
       "      - SCAN         - scan all feature codes 0x00..0xff\n"
       "      - COLOR        - all color related feature codes\n"
       "      - PROFILE      - color related codes for profile management\n"
       "      - LUT          - LUT related features\n"
       "      - AUDIO        - audio features\n"
       "      - WINDOW       - window operations (e.g. PIP)\n"
       "      - TV           - TV related settings\n"
       "      - PRESET       - MCCS codes classed as PRESET\n"
       "      - MANUFACTURER - manufacturer specific codes\n"
       "      - MFG          - same as MANUFACTURER\n"
       "      - TABLE        - Table type features\n"
       "      - SCONT        - simple Continuous features\n"
       "      - CCONT        - complex Continuous features\n"
       "      - CONT         - all Continuous features\n"
       "      - SNC          - simple Non-Continuous features\n"
       "      - CNC          - complex Non-Continuous features, using multiple bytes\n"
       "      - NC_WO        - write/only Non-Continuous features\n"
       "      - NC_CONT      - features classed as NC having a continuous subrange\n"
       "      - NC           - all Non-Continuous features\n"
       "    Keywords can be abbreviated to the first 3 characters.\n"
       "    Case is ignored.  e.g. \"COL\", \"pro\"\n"
       "\n"
       "  setvcp:\n"
       "    <feature-code>: hexadecimal feature code, with or without a leading 0x,\n"
       "       e.g. 10 or 0x10\n"
       "    [+|-] optionally indicate a relative value change, must be surrounded by blanks\n"
       "    <new-value>: a decimal number in the range 0..255, or a single byte hex value,\n"
       "       e.g. 0x80\n"
      ;
#endif

char * monitor_selection_option_help =
       "Monitor Selection:\n"
       "  The monitor to be communicated with can be specified using the following options:\n"
       "  --display <display_number>, where <display_number> ranges from 1 to the number of\n"
       "    displays detected\n"
       "  --bus <bus number>, for /dev/"I2C"-<bus number>\n"
#ifdef USE_USB
       "  --usb <usb bus number>.<usb device number>, for monitors communicating via USB\n"
#endif
       "  --edid <hex string>, where <hex string> is a 256 hex character representation of the\n"
       "          128 byte first block of the EDID\n"
       "  --mfg <mfg_id>, where <mfg_id> is the 3 character manufacturer id reported by the EDID\n"
       "  --model <model_name>, where <model name> is as reported by the EDID\n"
       "  --sn <serial_number>, where <serial_number> is the string form of the serial number\n"
       "          reported by the EDID\n"
       "  Options --mfg, --model and --sn may be combined.\n"
       "\n"
      ;

char * tracing_comma_separated_option_help =
       "Tracing:\n"
       "  The argument to --trace is a comma separated list of trace classes, surrounded by \n"
       "  quotation marks if necessary."
       "  e.g. --trace all, --trace \"I2C,ADL\"\n"
       "  Valid trace classes are:  BASE, I2C, ADL, DDC, DDCIO, VCP, TOP, ENV, API, UDF, SLEEP, RETRY, ALL.\n"
       "  Trace class names are not case sensitive.\n"
       "  (Some trace classes are more useful than others.)\n"
  //   "\n"
      ;

char * tracing_multiple_call_option_help =
       "Trace by trace class:\n"
       "  The argument to --trace is a trace class.  Specify the --trace option multiple\n"
       "  times to activate multiple trace classes, e.g. \"--trace "I2C" --trace ddc\"\n"
       "  Valid trace classes are:  BASE, I2C, ADL, DDC, DDCIO, VCP, TOP, ENV, API, UDF, SLEEP, RETRY, ALL.\n"
       "  Trace class names are not case sensitive.\n"
       "  (Some trace classes are more useful than others.)\n"
  //   "\n"
      ;

char * trcfunc_multiple_call_option_help =
       "Trace by function name:\n"
       "  The argument to --trcfunc is a function name.  Specify the --trcfunc option multiple\n"
       "  times to trace multiple functions, e.g. \"--trcfunc func1 --trcfunc func2\"\n"
      ;

char * trcfile_multiple_call_option_help =
       "Trace by file name:\n"
       "  The argument to --trcfile is a simple file name.  Specify the --trcfile option multiple\n"
       "  times to trace multiple functions, e.g. \"--trcfile ddc_edid.c --trcfile ddc_output.c\"\n"
      ;


char * stats_multiple_call_option_help =
       "Stats:\n"
       "  The argument to --stats is a statistics class.  Specify the --stats option multiple\n"
       "  times to activate multiple statistics classes, e.g. \"--stats calls --stats errors\"\n"
       "  Valid statistics classes are:  TRY, TRIES, ERRS, ERRORS, CALLS, ALL.\n"
       "  Statistics class names are not case sensitive and can abbreviated to 3 characters.\n"
       "  If no argument is specified, or ALL is specified, then all statistics classes are\n"
       "  output.\n"
      ;

char * maxtries_option_help =
      "Retries:\n"
      "  The argument to --maxtries is a comma separated list of 3 values:\n"
      "    maximum write-only exchange count\n"
      "    maximum write-read exchange count\n"
      "    maximum multi-part-read exchange count\n"
      "  A value of \"0\" or \".\" leaves the default value unchanged\n"
      "  e.g. --maxtries \".,.,15\" changes only the maximum multi-part-read exchange count"
      ;

//
// Initialization
//

#ifndef NDEBUG
void init_cmd_parser_base() {
   validate_cmdinfo();
   validate_subset_table();
}
#endif


