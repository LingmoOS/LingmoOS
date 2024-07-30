/* -*- C++ -*-
    This file is part of ThreadWeaver, a KDE framework.

    SPDX-FileCopyrightText: 2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "queuesignals.h"
#include "queuesignals_p.h"

namespace ThreadWeaver
{
/** @brief Construct a QueueSignals object, passing the QObject parent. */
QueueSignals::QueueSignals(QObject *parent)
    : QObject(parent)
    , m_d(nullptr)
{
}

QueueSignals::QueueSignals(Private::QueueSignals_Private *d, QObject *parent)
    : QObject(parent)
    , m_d(d)
{
}

QueueSignals::~QueueSignals()
{
    delete m_d;
    m_d = nullptr;
}

Private::QueueSignals_Private *QueueSignals::d()
{
    return m_d;
}

const Private::QueueSignals_Private *QueueSignals::d() const
{
    return m_d;
}

}

#include "moc_queuesignals.cpp"
