/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LINGMOARPING_H
#define LINGMOARPING_H

#include <sys/capability.h>
#include <sys/prctl.h>
#include <QDebug>

static cap_flag_value_t cap_raw = CAP_CLEAR;
static const cap_value_t caps[] = { CAP_NET_RAW };

static void limit_capabilities()
{
    cap_t cap_p;

    cap_p = cap_get_proc();
    if (!cap_p) {
        qWarning()<<"cap_get_proc failed.";
    }

    cap_get_flag(cap_p, CAP_NET_RAW, CAP_PERMITTED, &cap_raw);

    if (cap_raw != CAP_CLEAR) {
        if (cap_clear(cap_p) < 0) {
            qWarning()<<"cap clear failed"<<errno;
        }

        cap_set_flag(cap_p, CAP_PERMITTED, 1, caps, CAP_SET);

        if (cap_set_proc(cap_p) < 0) {
            qWarning()<<"cap set proc failed" << errno;
        }
    }

    if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
        qWarning()<<"pr set keepcaps 1 failed"<<errno;
    }

    if (setuid(getuid()) < 0) {
        qWarning()<<"set uid failed"<< errno;
    }

    if (prctl(PR_SET_KEEPCAPS, 0) < 0) {
        qWarning()<<"pr set keepcaps 0 failed" << errno;
    }

    cap_free(cap_p);
}

static int modify_capability_raw(int on)
{
    cap_t cap_p;

    if (cap_raw != CAP_SET) {
        qWarning()<<"modify on"<<on;
        return on ? -1 : 0;
    }

    cap_p = cap_get_proc();
    if (!cap_p) {
        qWarning()<<"modify cap raw get proc failed"<<errno;
        return -1;
    }

    cap_set_flag(cap_p, CAP_EFFECTIVE, 1, caps, on ? CAP_SET : CAP_CLEAR);

    if (cap_set_proc(cap_p) < 0) {
        qWarning()<<"modify cap raw set proc failed"<<errno;
    }

    cap_free(cap_p);
    return 0;
}

static void drop_capabilities(void)
{
#if 0
    cap_t cap_p = cap_init();

    if (!cap_p) {
        qWarning()<<"cap init failed"<<errno;
    }

    if (cap_set_proc(cap_p) < 0) {
        qWarning()<<"drop cap set cap failed" << errno;
    }

    cap_free(cap_p);
#endif
}

static inline int enable_capability_raw()
{
    return modify_capability_raw(1);
}

static inline int disable_capability_raw()
{
    return modify_capability_raw(0);
}

#endif // LINGMOARPING_H
