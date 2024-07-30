/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOB_P_H
#define KJOB_P_H

#include "kjob.h"
#include <QEventLoopLocker>
#include <QMap>

#include <array>

class KJobUiDelegate;
class QTimer;
class QEventLoop;

// This is a private class, but it's exported for
// KIO::Job's usage. Other Job classes in kdelibs may
// use it too.
class KCOREADDONS_EXPORT KJobPrivate
{
public:
    KJobPrivate();
    virtual ~KJobPrivate();

    void speedTimeout();

    KJob *q_ptr = nullptr;

    KJobUiDelegate *uiDelegate = nullptr;
    QString errorText;
    int error = KJob::NoError;
    KJob::Unit progressUnit = KJob::Bytes;

    struct Amounts {
        qulonglong processedAmount = 0;
        qulonglong totalAmount = 0;
    };

    std::array<Amounts, KJob::UnitsCount> m_jobAmounts;

    unsigned long percentage = 0;
    QTimer *speedTimer = nullptr;
    QEventLoop *eventLoop = nullptr;
    // eventLoopLocker prevents QCoreApplication from exiting when the last
    // window is closed until the job has finished running
    QEventLoopLocker eventLoopLocker;
    KJob::Capabilities capabilities = KJob::NoCapabilities;
    bool suspended = false;
    bool isAutoDelete = true;
    bool m_hideFinishedNotification = false;
    bool isFinished = false;
    bool m_startedWithExec = false;

    Q_DECLARE_PUBLIC(KJob)
};

#endif
