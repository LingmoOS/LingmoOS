/* -*- C++ -*-
    The detailed API for Weavers in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "queueapi.h"

namespace ThreadWeaver
{
QueueAPI::QueueAPI(QObject *parent)
    : QueueSignals(parent)
{
}

QueueAPI::QueueAPI(Private::QueueSignals_Private *d, QObject *parent)
    : QueueSignals(d, parent)
{
}

}

#include "moc_queueapi.cpp"
