/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjob.h"
#include <QTimer>

class KListOpenFilesJobPrivate
{
};

KListOpenFilesJob::KListOpenFilesJob(const QString &)
    : d(nullptr)
{
}

KListOpenFilesJob::~KListOpenFilesJob() = default;

void KListOpenFilesJob::start()
{
    QTimer::singleShot(0, [this]() {
        setError(static_cast<int>(KListOpenFilesJob::Error::NotSupported));
        setErrorText(QObject::tr("KListOpenFilesJob is not supported on Windows"));
        emitResult();
    });
}

KProcessList::KProcessInfoList KListOpenFilesJob::processInfoList() const
{
    return KProcessList::KProcessInfoList();
}

#include "moc_klistopenfilesjob.cpp"
