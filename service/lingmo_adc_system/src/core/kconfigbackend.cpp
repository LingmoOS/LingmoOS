/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Thomas Braxton <brax108@cox.net>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigbackend_p.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QStringList>

#include "kconfigdata_p.h"
#include "kconfigini_p.h"

typedef QExplicitlySharedDataPointer<KConfigBackend> BackendPtr;

class KConfigBackendPrivate
{
public:
    QString localFileName;

    static QString whatSystem(const QString & /*fileName*/)
    {
        return QStringLiteral("INI");
    }
};

void KConfigBackend::registerMappings(const KEntryMap & /*entryMap*/)
{
}

BackendPtr KConfigBackend::create(const QString &file, const QString &sys)
{
    // qDebug() << "creating a backend for file" << file << "with system" << sys;
    KConfigBackend *backend = nullptr;

#if 0 // TODO port to Qt5 plugin loading
    const QString system = (sys.isEmpty() ? KConfigBackendPrivate::whatSystem(file) : sys);
    if (system.compare(QLatin1String("INI"), Qt::CaseInsensitive) != 0) {
        const QString constraint = QString::fromLatin1("[X-KDE-PluginInfo-Name] ~~ '%1'").arg(system);
        KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("KConfigBackend"), constraint);

        //qDebug() << "found" << offers.count() << "offers for KConfigBackend plugins with name" << system;
        foreach (const KService::Ptr &offer, offers) {
            backend = offer->createInstance<KConfigBackend>(nullptr);
            if (backend) {
                //qDebug() << "successfully created a backend for" << system;
                backend->setFilePath(file);
                return BackendPtr(backend);
            }
        } // foreach offers
    }
#else
    Q_UNUSED(sys);
#endif

    // qDebug() << "default creation of the Ini backend";
    backend = new KConfigIniBackend;
    backend->setFilePath(file);
    return BackendPtr(backend);
}

KConfigBackend::KConfigBackend()
    : d(new KConfigBackendPrivate)
{
}

KConfigBackend::~KConfigBackend()
{
    delete d;
}

QString KConfigBackend::filePath() const
{
    return d->localFileName;
}

void KConfigBackend::setLocalFilePath(const QString &file)
{
    d->localFileName = file;
}
