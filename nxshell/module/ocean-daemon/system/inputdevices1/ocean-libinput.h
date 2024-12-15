// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCEAN_LIBINPUT_H
#define OCEAN_LIBINPUT_H

#include <libinput.h>

struct data;

struct data *new_libinput(char *userdata, enum libinput_log_priority priority);

int start(struct data *d);
void stop(struct data *d);

#endif // !OCEAN_LIBINPUT_H
