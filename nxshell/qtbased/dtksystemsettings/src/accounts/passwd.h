// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <sys/types.h>

char *mkpasswd(const char *words);

int lock_shadow_file();
int unlock_shadow_file();

int exist_pw_uid(__uid_t uid);

char *get_pw_name(__uid_t uid);
char *get_pw_gecos(__uid_t uid);
__uid_t get_pw_uid(__uid_t uid);
__gid_t get_pw_gid(__uid_t uid);
char *get_pw_dir(__uid_t uid);
char *get_pw_shell(__uid_t uid);
