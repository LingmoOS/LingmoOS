/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_EXTRACTORPROCESS_H
#define BALOO_EXTRACTORPROCESS_H

#include "extractor/commandpipe.h"

#include <QProcess>
#include <QObject>
#include <QVector>

namespace Baloo {

class ExtractorProcess : public QObject
{
    Q_OBJECT
public:
    ExtractorProcess(const QString& extractorPath, QObject* parent = nullptr);
    ~ExtractorProcess();

    void index(const QVector<quint64>& fileIds);
    void start();

Q_SIGNALS:
    void startedIndexingFile(QString filePath);
    void finishedIndexingFile(QString filePath, bool fileUpdated);
    void done();
    void failed();

private:
    const QString m_extractorPath;

    QProcess m_extractorProcess;
    Baloo::Private::ControllerPipe m_controller;
};
}

#endif // BALOO_EXTRACTORPROCESS_H
