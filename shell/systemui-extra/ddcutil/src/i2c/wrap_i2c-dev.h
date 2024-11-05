/** \file wrap_i2c-dev.h
 *
 * Including file i2c-dev.h presents multiple issues.
 * This header file addresses those issues in one place.
 *
 * Currently this file is included by i2c_base_io.c and i2c_do_io.c.
 *
 * This code is in a separate file instead of an existing header
 * file because it is not part of the public interface of any
 * header file.
 */
// Copyright (C) 2014-2016 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WRAP_I2C_DEV_H_
#define WRAP_I2C_DEV_H_

// On Fedora 23 and SUSE 13.2, there is no problem with NULL.
// But on Fedora 22 and SUSE 13.1, we get an error that NULL is
// undefined.  Including stddef.h does not solve the problem - apparently
// the dummy version in /usr/include/linux/i2c-dev.h is picked up.
// This hack seems to solve that problem.

#ifndef NULL
#define NULL ((void*)0)
#endif


// On Fedora, i2c-dev.h is minimal.  i2c.h is required for struct i2c_msg and
// other stuff.  On Ubuntu and SuSE, including both causes redefinition errors.
// If I2C_FUNC_I2C is not defined, the definition is present in the full version
// of i2c-dev.h but not in the abbreviated version, so i2c.h must be included.

#include <linux/i2c-dev.h>
#ifndef I2C_FUNC_I2C
#include <linux/i2c.h>
#endif

#endif /* WRAP_I2C_DEV_H_ */
