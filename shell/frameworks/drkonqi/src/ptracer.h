/*
    SPDX-FileCopyrightText: 2019 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PTRACER_H
#define PTRACER_H

#include <QtGlobal>

/** On Linux, tell the process to allow the debugger to attach to it */
void setPtracer(qint64 debuggerpid, qint64 debuggeepid);

#endif
