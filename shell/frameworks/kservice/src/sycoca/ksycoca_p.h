/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999-2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2005-2009 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSYCOCA_P_H
#define KSYCOCA_P_H

#include "ksycocafactory_p.h"
#include <KDirWatch>
#include <QDateTime>
#include <QElapsedTimer>
#include <QStringList>

#include <memory>

class QFile;
class QDataStream;
class KSycocaAbstractDevice;
class KMimeTypeFactory;
class KServiceFactory;
class KServiceGroupFactory;

// This is for the part of the global header that we don't need to store,
// i.e. it's just a struct for returning temp data from readSycocaHeader().
struct KSycocaHeader {
    KSycocaHeader()
        : timeStamp(0)
        , updateSignature(0)
    {
    }
    QString prefixes;
    QString language;
    qint64 timeStamp; // in ms
    quint32 updateSignature;
};

QDataStream &operator>>(QDataStream &in, KSycocaHeader &h);

/**
 * \internal
 * Exported for unittests
 */
class KSERVICE_EXPORT KSycocaPrivate
{
public:
    explicit KSycocaPrivate(KSycoca *qq);

    // per-thread "singleton"
    static KSycocaPrivate *self()
    {
        return KSycoca::self()->d;
    }

    bool checkVersion();
    bool openDatabase();
    enum BehaviorIfNotFound {
        IfNotFoundDoNothing = 0,
        IfNotFoundRecreate = 1,
    };
    Q_DECLARE_FLAGS(BehaviorsIfNotFound, BehaviorIfNotFound)
    bool checkDatabase(BehaviorsIfNotFound ifNotFound);
    void closeDatabase();
    void setStrategyFromString(const QString &strategy);
    bool tryMmap();

    /**
     * Check if the on-disk cache needs to be rebuilt, and do it then.
     */
    void checkDirectories();

    /**
     * Check if the on-disk cache needs to be rebuilt, and return true
     */
    bool needsRebuild();

    /**
     * Recreate the cache and reopen the database
     */
    bool buildSycoca();

    KSycocaHeader readSycocaHeader();

    KSycocaAbstractDevice *device();
    QDataStream *&stream();

    QString findDatabase();
    void slotDatabaseChanged();

    KMimeTypeFactory *mimeTypeFactory();
    KServiceFactory *serviceFactory();
    KServiceGroupFactory *serviceGroupFactory();

    enum {
        DatabaseNotOpen, // openDatabase must be called
        BadVersion, // it's opened, but it's not usable
        DatabaseOK,
    } databaseStatus;
    bool readError;

    qint64 timeStamp; // in ms since epoch
    enum { StrategyMmap, StrategyMemFile, StrategyFile } m_sycocaStrategy;
    QString m_databasePath;
    QString language;
    quint32 updateSig;
    QMap<QString, qint64> allResourceDirs; // path, modification time in "ms since epoch"
    QMap<QString, qint64> extraFiles; // path, modification time in "ms since epoch"

    void addFactory(KSycocaFactory *factory)
    {
        m_factories.append(factory);
    }
    KSycocaFactoryList *factories()
    {
        return &m_factories;
    }

    QElapsedTimer m_lastCheck;
    QDateTime m_dbLastModified;

    // Using KDirWatch because it will reliably tell us every time ksycoca is recreated.
    // QFileSystemWatcher's inotify implementation easily gets confused between "removed" and "changed",
    // and fails to re-add an inotify watch after the file was replaced at some point (KServiceTest::testThreads),
    // thinking it only got changed and not removed+recreated.
    // NOTE: this may be nullptr when file watching is disabled on the current thread
    std::unique_ptr<KDirWatch> m_fileWatcher;
    bool m_haveListeners;

    KSycoca *q;

private:
    KSycocaFactoryList m_factories;
    size_t sycoca_size;
    const char *sycoca_mmap;
    QFile *m_mmapFile;
    KSycocaAbstractDevice *m_device;

public:
    KMimeTypeFactory *m_mimeTypeFactory;
    KServiceFactory *m_serviceFactory;
    KServiceGroupFactory *m_serviceGroupFactory;
};

#endif /* KSYCOCA_P_H */
