/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTRACTOR_APP_H
#define EXTRACTOR_APP_H

#include <QVector>
#include <QMimeDatabase>
#include <QSocketNotifier>
#include <QFile>

#include <memory>

#include <KFileMetaData/ExtractorCollection>

#include "database.h"
#include "extractor/commandpipe.h"
#include "../fileindexerconfig.h"

class KIdleTime;
class QString;

namespace Baloo {

class Transaction;

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(QObject* parent = nullptr);
    ~App();

private Q_SLOTS:
    void slotNewBatch(const QVector<quint64>& ids);
    void processNextFile();

private:
    bool index(Transaction* tr, const QString& filePath, quint64 id);

    QMimeDatabase m_mimeDb;

    KFileMetaData::ExtractorCollection m_extractorCollection;

    FileIndexerConfig m_config;

    QSocketNotifier m_notifyNewData;
    QFile m_input;
    QFile m_output;
    Private::WorkerPipe m_workerPipe;

    KIdleTime* m_idleTime = nullptr;
    bool m_isBusy = true;

    QVector<quint64> m_ids;
    std::unique_ptr<Transaction> m_tr;
};

}
#endif
