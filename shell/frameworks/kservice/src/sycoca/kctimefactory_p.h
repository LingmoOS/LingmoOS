/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCTIME_FACTORY_H
#define KCTIME_FACTORY_H

#include <QHash>
#include <ksycocafactory_p.h>

/**
 * Simple dict for associating a timestamp with each file in ksycoca
 */
class KCTimeDict
{
public:
    void addCTime(const QString &path, const QByteArray &resource, quint32 ctime);
    quint32 ctime(const QString &path, const QByteArray &resource) const;
    void remove(const QString &path, const QByteArray &resource);
    void dump() const;
    bool isEmpty() const
    {
        return m_hash.isEmpty();
    }

    void load(QDataStream &str);
    void save(QDataStream &str) const;

private:
    typedef QHash<QString, quint32> Hash;
    Hash m_hash;
};

/**
 * Internal factory for storing the timestamp of each file in ksycoca
 * @internal
 */
class KCTimeFactory : public KSycocaFactory
{
    K_SYCOCAFACTORY(KST_CTimeInfo)
public:
    /**
     * Create factory
     */
    explicit KCTimeFactory(KSycoca *db);

    ~KCTimeFactory() override;

    /**
     * Write out header information
     */
    void saveHeader(QDataStream &str) override;

    /**
     * Write out data
     */
    void save(QDataStream &str) override;

    KSycocaEntry *createEntry(const QString &) const override
    {
        return nullptr;
    }
    KSycocaEntry *createEntry(int) const override
    {
        return nullptr;
    }

    // Loads the dict and returns it; does not set m_ctimeDict;
    // this is only used in incremental mode for loading the old timestamps.
    KCTimeDict loadDict() const;

    // The API for inserting/looking up entries is in KCTimeDict.
    KCTimeDict *dict()
    {
        return &m_ctimeDict;
    }

private:
    KCTimeDict m_ctimeDict;
    int m_dictOffset;
};

#endif
