/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filecopyworker.h"

#include <KLocalizedString>
#include <QFile>

using namespace KNSCore;

class KNSCore::FileCopyWorkerPrivate
{
public:
    FileCopyWorkerPrivate()
    {
    }
    QFile source;
    QFile destination;
};

FileCopyWorker::FileCopyWorker(const QUrl &source, const QUrl &destination, QObject *parent)
    : QThread(parent)
    , d(new FileCopyWorkerPrivate)
{
    d->source.setFileName(source.toLocalFile());
    d->destination.setFileName(destination.toLocalFile());
}

FileCopyWorker::~FileCopyWorker() = default;

void FileCopyWorker::run()
{
    if (d->source.open(QIODevice::ReadOnly)) {
        if (d->destination.open(QIODevice::WriteOnly)) {
            const qint64 totalSize = d->source.size();

            for (qint64 i = 0; i < totalSize; i += 1024) {
                d->destination.write(d->source.read(1024));
                d->source.seek(i);
                d->destination.seek(i);

                Q_EMIT progress(i, totalSize / 1024);
            }
            Q_EMIT completed();
        } else {
            Q_EMIT error(i18n("Could not open %1 for writing", d->destination.fileName()));
        }
    } else {
        Q_EMIT error(i18n("Could not open %1 for reading", d->source.fileName()));
    }
}

#include "moc_filecopyworker.cpp"
