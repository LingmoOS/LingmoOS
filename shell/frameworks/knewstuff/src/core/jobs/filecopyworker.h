/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILECOPYWORKER_H
#define FILECOPYWORKER_H

#include <QThread>
#include <QUrl>

namespace KNSCore
{
class FileCopyWorkerPrivate;
class FileCopyWorker : public QThread
{
    Q_OBJECT
public:
    explicit FileCopyWorker(const QUrl &source, const QUrl &destination, QObject *parent = nullptr);
    ~FileCopyWorker() override;
    void run() override;

    Q_SIGNAL void progress(qlonglong current, qlonglong total);
    Q_SIGNAL void completed();
    Q_SIGNAL void error(const QString &message);

private:
    const std::unique_ptr<FileCopyWorkerPrivate> d;
};

}

#endif // FILECOPYWORKER_H
