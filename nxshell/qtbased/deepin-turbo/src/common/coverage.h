/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef COVERAGE_H
#define COVERAGE_H

#ifdef WITH_COVERAGE
#ifdef __cplusplus
extern "C" void __gcov_flush(void);
extern "C" pid_t __gcov_fork(void);
extern "C" int __gcov_execv(const char *path, char *const argv[]);
extern "C" int __gcov_execve(const char *path, char *const argv[],
    char *const envp[]);
#endif // __cplusplus

#define _exit(status) {                         \
        __gcov_flush();                         \
        _exit(status);                          \
    }

#define fork() __gcov_fork()
#define execv(path, argv) __gcov_execv((path), (argv))
#define execve(path, argv, envp) __gcov_execve((path), (argv), (envp))

#endif // WITH_COVERAGE

#endif // COVERAGE_H
