/* SPDX-FileCopyrightText: None */
/* SPDX-License-Identifier: MIT */
#ifndef OOM_GLOBALS_H
#define OOM_GLOBALS_H

extern int enable_debug;

extern char *procdir_path;

#define DLL_LOCAL __attribute__((visibility("hidden")))
#endif
