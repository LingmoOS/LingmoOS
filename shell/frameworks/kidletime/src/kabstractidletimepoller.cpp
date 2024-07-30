/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kabstractidletimepoller_p.h"

KAbstractIdleTimePoller::KAbstractIdleTimePoller(QObject *parent)
    : QObject(parent)
{
}

KAbstractIdleTimePoller::~KAbstractIdleTimePoller()
{
}

#include "moc_kabstractidletimepoller_p.cpp"
