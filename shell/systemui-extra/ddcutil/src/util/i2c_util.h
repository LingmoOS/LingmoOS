/** @file i2c_util.h
 *
 * I2C utility functions
 */

// Copyright (C) 2014-2022 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef I2C_UTIL_H_
#define I2C_UTIL_H_

#include <stdbool.h>

int           i2c_name_to_busno(const char * name);
int           extract_number_after_hyphen(const char * name);
int           i2c_compare(const void * v1, const void * v2);

unsigned long i2c_get_functionality_flags_by_fd(int fd);
char *        i2c_interpret_functionality_flags(unsigned long functionality);
void          i2c_report_functionality_flags(long functionality, int maxline, int depth);
bool          dev_i2c_devices_exist();

#endif /* I2C_UTIL_H_ */
