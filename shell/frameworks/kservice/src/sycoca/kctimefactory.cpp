/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kctimefactory_p.h"
#include "sycocadebug.h"
#include <ksycoca.h>
#include <ksycocatype.h>
#include <ksycocautils_p.h>

#include <assert.h>

// NOTE: the storing of "resource" here is now completely useless (since everything is under GenericDataLocation)

static inline QString key(const QString &path, const QByteArray &resource)
{
    return QString::fromLatin1(resource) + QLatin1Char('|') + path;
}

void KCTimeDict::addCTime(const QString &path, const QByteArray &resource, quint32 ctime)
{
    Q_ASSERT(ctime != 0);
    assert(!path.isEmpty());
    m_hash.insert(key(path, resource), ctime);
}

quint32 KCTimeDict::ctime(const QString &path, const QByteArray &resource) const
{
    return m_hash.value(key(path, resource), 0);
}

void KCTimeDict::remove(const QString &path, const QByteArray &resource)
{
    m_hash.remove(key(path, resource));
}

void KCTimeDict::dump() const
{
    qCDebug(SYCOCA) << m_hash.keys();
}

void KCTimeDict::load(QDataStream &str)
{
    QString key;
    quint32 ctime;
    while (true) {
        str >> key >> ctime;
        if (key.isEmpty()) {
            break;
        }
        m_hash.insert(key, ctime);
    }
}

void KCTimeDict::save(QDataStream &str) const
{
    for (auto it = m_hash.cbegin(), endIt = m_hash.cend(); it != endIt; ++it) {
        str << it.key() << it.value();
    }
    str << QString() << quint32(0);
}

///////////

KCTimeFactory::KCTimeFactory(KSycoca *db)
    : KSycocaFactory(KST_CTimeInfo, db)
    , m_ctimeDict()
{
    if (!sycoca()->isBuilding()) {
        QDataStream *str = stream();
        (*str) >> m_dictOffset;
    } else {
        m_dictOffset = 0;
    }
}

KCTimeFactory::~KCTimeFactory()
{
}

void KCTimeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);

    str << m_dictOffset;
}

void KCTimeFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    m_dictOffset = str.device()->pos();
    m_ctimeDict.save(str);
    const qint64 endOfFactoryData = str.device()->pos();
    saveHeader(str);
    str.device()->seek(endOfFactoryData);
}

KCTimeDict KCTimeFactory::loadDict() const
{
    KCTimeDict dict;
    QDataStream *str = stream();
    assert(str);
    str->device()->seek(m_dictOffset);
    dict.load(*str);
    return dict;
}
