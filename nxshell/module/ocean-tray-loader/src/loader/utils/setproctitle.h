// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define PROCTITLE_ERROR  -1
#define PROCTITLE_OK   0

int init_setproctitle(char* argv[], char* environ[]);
void setproctitle(const char *title);
