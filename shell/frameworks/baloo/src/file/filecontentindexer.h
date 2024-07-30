/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILECONTENTINDEXER_H
#define BALOO_FILECONTENTINDEXER_H

#include <QRunnable>
#include <QObject>
#include <QAtomicInt>
#include <QStringList>

#include <QDBusServiceWatcher>
#include <QDBusMessage>

namespace Baloo {

class FileContentIndexerProvider;

class FileContentIndexer : public QObject, public QRunnable
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.fileindexer")

    Q_PROPERTY(QString currentFile READ currentFile NOTIFY startedIndexingFile)
public:
    FileContentIndexer(uint batchSize, FileContentIndexerProvider* provider, uint& finishedCount, QObject* parent = nullptr);

    QString currentFile() { return m_currentFile; }

    void run() override;

    void quit() {
        m_stop.storeRelaxed(true);
    }

public Q_SLOTS:
    Q_SCRIPTABLE void registerMonitor(const QDBusMessage& message);
    Q_SCRIPTABLE void unregisterMonitor(const QDBusMessage& message);

Q_SIGNALS:
    Q_SCRIPTABLE void startedIndexingFile(const QString& filePath);
    Q_SCRIPTABLE void finishedIndexingFile(const QString& filePath);
    Q_SCRIPTABLE void committedBatch(uint time, uint batchSize);

    void done();

private Q_SLOTS:
    void monitorClosed(const QString& service);
    void slotStartedIndexingFile(const QString& filePath);
    void slotFinishedIndexingFile(const QString& filePath, bool fileUpdated);

private:
    uint m_batchSize;
    FileContentIndexerProvider* m_provider;
    uint& m_finishedCount;

    QAtomicInt m_stop;

    QString m_currentFile;
    QStringList m_updatedFiles;

    QStringList m_registeredMonitors;
    QDBusServiceWatcher m_monitorWatcher;

    QString m_extractorPath;
};

}

#endif // BALOO_FILECONTENTINDEXER_H
