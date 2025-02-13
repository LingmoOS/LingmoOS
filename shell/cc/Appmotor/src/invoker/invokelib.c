/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2021 Open Mobile Platform LLC.
** Copyright (c) 2021 Jolla Ltd.
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "report.h"
#include "invokelib.h"

static void invoke_send_or_die(int fd, const void *data, size_t size)
{
    if (write(fd, data, size) != (ssize_t)size) {
        const char m[] = "*** socket write failure, terminating\n";
        if (write(STDERR_FILENO, m, sizeof m - 1) == -1) {
            // dontcare
        }
        _exit(EXIT_FAILURE);
    }
}

void invoke_send_msg(int fd, uint32_t msg)
{
    debug("%s: %08x\n", __FUNCTION__, msg);
    invoke_send_or_die(fd, &msg, sizeof(msg));
}

bool invoke_recv_msg(int fd, uint32_t *msg)
{
    uint32_t  readBuf = 0;
    int len = sizeof(readBuf);
    ssize_t numRead = read(fd, &readBuf, len);

    if (numRead == -1)
    {
        debug("%s: Error reading message: %m\n", __FUNCTION__);
        *msg = 0;
        return false;
    }
    else if (numRead < len)
    {
        debug("%s: Error: unexpected end-of-file \n", __FUNCTION__);
        *msg = 0;
        return false;
    }
    else
    {
        debug("%s: %08x\n", __FUNCTION__, readBuf);
        *msg = readBuf;
        return true;
    }
}

void invoke_send_str(int fd, const char *str)
{
    if (!str)
        str = "";
    uint32_t size = strlen(str) + 1;

    /* Send size. */
    invoke_send_msg(fd, size);

    debug("%s: '%s'\n", __FUNCTION__, str);

    /* Send the string. */
    invoke_send_or_die(fd, str, size);
}
