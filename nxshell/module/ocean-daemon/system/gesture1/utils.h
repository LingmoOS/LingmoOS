// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GESTURE_UTILS_H__
#define __GESTURE_UTILS_H__

#include <libinput.h>

struct libinput* open_from_udev(char *seat, void *user_data, int verbose);
struct libinput* open_from_path(char **path, void *user_data, int verbose);

#endif
