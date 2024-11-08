/** @file tuned_sleep.h
 *
 *  Perform sleep. The sleep time is determined by io mode, sleep event time,
 *  and applicable multipliers.
 */

// Copyright (C) 2019-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TUNED_SLEEP_H_
#define TUNED_SLEEP_H_

/** \cond */
#include <stdbool.h>
// #include "public/ddcutil_types.h"
/** \endcond */
#include "base/displays.h"
#include "base/execution_stats.h"   // for Sleep_Event_Type

extern bool suppress_se_post_read;

bool enable_deferred_sleep(bool enable);
bool is_deferred_sleep_enabled();

void check_deferred_sleep(
      Display_Handle * dh,
      const char *     func,
      int              lineno,
      const char *     filename);

void tuned_sleep_with_trace(
      Display_Handle * dh,
      Sleep_Event_Type event_type,
      int              special_sleep_time_millis,
      const char *     func,
      int              lineno,
      const char *     filename,
      const char *     msg);

// Convenience macros:

#define CHECK_DEFERRED_SLEEP(_dh) \
   check_deferred_sleep(_dh, __func__, __LINE__, __FILE__)

#define TUNED_SLEEP_WITH_TRACE(_dh, _event_type, _msg) \
   tuned_sleep_with_trace(_dh, _event_type, 0, __func__, __LINE__, __FILE__, _msg)

#define SPECIAL_TUNED_SLEEP_WITH_TRACE(_dh, _time_millis, _msg) \
   tuned_sleep_with_trace(_dh, SE_SPECIAL, _time_millis, __func__, __LINE__, __FILE__, _msg)

void init_tuned_sleep();

#endif /* TUNED_SLEEP_H_ */
