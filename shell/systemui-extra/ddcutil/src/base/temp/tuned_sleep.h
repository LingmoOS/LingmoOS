/** @file tuned_sleep.h
 *
 *  Perform sleep. The sleep time is determined by io mode, sleep event time,
 *  and applicable multipliers.
 */

// Copyright (C) 2019-2020 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TUNED_SLEEP_H_
#define TUNED_SLEEP_H_

#include <stdbool.h>

#include "base/sleep.h"
#include "base/execution_stats.h"

// Sleep time adjustments
void   lock_default_sleep_multiplier_factor();
void   unlock_default_sleep_multiplier_factor();
void   set_default_sleep_multiplier_factor(double multiplier);
double get_default_sleep_multiplier_factor();
void   set_sleep_multiplier_factor(double multiplier);
double get_sleep_multiplier_factor();

void   set_sleep_multiplier_ct(int multiplier);
int    get_sleep_multiplier_ct();


// Perform tuned sleep
void tuned_sleep_with_tracex(
      DDCA_IO_Mode     io_mode,
      Sleep_Event_Type event_type,
      const char *     func,
      int              lineno,
      const char *     filename,
      const char *     msg);

// Convenience functions and macros:
void tuned_sleep_dh(Display_Handle* dh, Sleep_Event_Type event_type);

#define TUNED_SLEEP_WITH_TRACE(_io_mode, _event_type, _msg) \
   tuned_sleep_with_tracex(_io_mode, _event_type, __func__, __LINE__, __FILE__, _msg)

#define TUNED_SLEEP_I2C_WITH_TRACE(_event_type, _msg) \
   tuned_sleep_with_tracex(DDCA_IO_I2C, _event_type, __func__, __LINE__, __FILE__, _msg)

#endif /* TUNED_SLEEP_H_ */
