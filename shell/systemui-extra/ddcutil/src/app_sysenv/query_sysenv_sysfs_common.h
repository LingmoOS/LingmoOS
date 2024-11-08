// query_sysenv_sysfs_common.h

// Copyright (C) 2018-2021 Sanford Rockowitz <rockowitz@minsoft.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef QUERY_SYSENV_SYSFS_COMMON_H_
#define QUERY_SYSENV_SYSFS_COMMON_H_

#include <stdbool.h>
#include <sys/types.h>

ushort h2ushort(char * hval);

unsigned h2uint(char * hval);

#endif /* QUERY_SYSENV_SYSFS_COMMON_H_ */
