/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kidletime.h"

#include <config-kidletime.h>

#include "kabstractidletimepoller_p.h"
#include "logging.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonArray>
#include <QPluginLoader>
#include <QPointer>
#include <QSet>

class KIdleTimeHelper
{
public:
    KIdleTimeHelper()
        : q(nullptr)
    {
    }
    ~KIdleTimeHelper()
    {
        delete q;
    }
    KIdleTimeHelper(const KIdleTimeHelper &) = delete;
    KIdleTimeHelper &operator=(const KIdleTimeHelper &) = delete;
    KIdleTime *q;
};

Q_GLOBAL_STATIC(KIdleTimeHelper, s_globalKIdleTime)

KIdleTime *KIdleTime::instance()
{
    if (!s_globalKIdleTime()->q) {
        new KIdleTime;
    }

    return s_globalKIdleTime()->q;
}

class KIdleTimePrivate
{
    Q_DECLARE_PUBLIC(KIdleTime)
    KIdleTime *q_ptr;

public:
    KIdleTimePrivate()
        : catchResume(false)
        , currentId(0)
    {
    }

    void loadSystem();
    void unloadCurrentSystem();
    void resumingFromIdle();
    void timeoutReached(int msec);

    QPointer<KAbstractIdleTimePoller> poller;
    bool catchResume;

    int currentId;
    QHash<int, int> associations;
};

KIdleTime::KIdleTime()
    : QObject(nullptr)
    , d_ptr(new KIdleTimePrivate())
{
    Q_ASSERT(!s_globalKIdleTime()->q);
    s_globalKIdleTime()->q = this;

    d_ptr->q_ptr = this;

    Q_D(KIdleTime);
    d->loadSystem();

    connect(d->poller.data(), &KAbstractIdleTimePoller::resumingFromIdle, this, [d]() {
        d->resumingFromIdle();
    });
    connect(d->poller.data(), &KAbstractIdleTimePoller::timeoutReached, this, [d](int msec) {
        d->timeoutReached(msec);
    });
}

KIdleTime::~KIdleTime()
{
    Q_D(KIdleTime);
    d->unloadCurrentSystem();
}

void KIdleTime::catchNextResumeEvent()
{
    Q_D(KIdleTime);

    if (!d->catchResume && d->poller) {
        d->catchResume = true;
        d->poller.data()->catchIdleEvent();
    }
}

void KIdleTime::stopCatchingResumeEvent()
{
    Q_D(KIdleTime);

    if (d->catchResume && d->poller) {
        d->catchResume = false;
        d->poller.data()->stopCatchingIdleEvents();
    }
}

int KIdleTime::addIdleTimeout(int msec)
{
    Q_D(KIdleTime);
    if (Q_UNLIKELY(msec < 0)) {
        qCWarning(KIDLETIME, "KIdleTime::addIdleTimeout: invalid timeout: %d", msec);
        return 0;
    }
    if (Q_UNLIKELY(!d->poller)) {
        return 0;
    }

    d->poller.data()->addTimeout(msec);

    ++d->currentId;
    d->associations[d->currentId] = msec;

    return d->currentId;
}

void KIdleTime::removeIdleTimeout(int identifier)
{
    Q_D(KIdleTime);

    const auto it = d->associations.constFind(identifier);
    if (it == d->associations.cend() || !d->poller) {
        return;
    }

    const int msec = it.value();

    d->associations.erase(it);

    const bool isFound = std::any_of(d->associations.cbegin(), d->associations.cend(), [msec](int i) {
        return i == msec;
    });

    if (!isFound) {
        d->poller.data()->removeTimeout(msec);
    }
}

void KIdleTime::removeAllIdleTimeouts()
{
    Q_D(KIdleTime);

    std::vector<int> removed;

    for (auto it = d->associations.cbegin(); it != d->associations.cend(); ++it) {
        const int msec = it.value();
        const bool alreadyIns = std::find(removed.cbegin(), removed.cend(), msec) != removed.cend();
        if (!alreadyIns && d->poller) {
            removed.push_back(msec);
            d->poller.data()->removeTimeout(msec);
        }
    }

    d->associations.clear();
}

static QStringList pluginCandidates()
{
    QStringList ret;

    const QStringList libPath = QCoreApplication::libraryPaths();
    for (const QString &path : libPath) {
#ifdef Q_OS_MACOS
        const QDir pluginDir(path + QStringLiteral("/kf6/kidletime"));
#else
        const QDir pluginDir(path + QStringLiteral("/kf6/org.kde.kidletime.platforms"));
#endif
        if (!pluginDir.exists()) {
            continue;
        }

        const auto entries = pluginDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

        ret.reserve(ret.size() + entries.size());
        for (const QString &entry : entries) {
            ret << pluginDir.absoluteFilePath(entry);
        }
    }

    return ret;
}

static bool checkPlatform(const QJsonObject &metadata, const QString &platformName)
{
    const QJsonArray platforms = metadata.value(QStringLiteral("MetaData")).toObject().value(QStringLiteral("platforms")).toArray();
    return std::any_of(platforms.begin(), platforms.end(), [&platformName](const QJsonValue &value) {
        return QString::compare(platformName, value.toString(), Qt::CaseInsensitive) == 0;
    });
}

static KAbstractIdleTimePoller *loadPoller()
{
    const QString platformName = QGuiApplication::platformName();

    const QList<QStaticPlugin> staticPlugins = QPluginLoader::staticPlugins();
    for (const QStaticPlugin &staticPlugin : staticPlugins) {
        const QJsonObject metadata = staticPlugin.metaData();
        if (metadata.value(QLatin1String("IID")) != QLatin1String(KAbstractIdleTimePoller_iid)) {
            continue;
        }
        if (checkPlatform(metadata, platformName)) {
            auto *poller = qobject_cast<KAbstractIdleTimePoller *>(staticPlugin.instance());
            if (poller) {
                if (poller->isAvailable()) {
                    qCDebug(KIDLETIME) << "Loaded system poller from a static plugin";
                    return poller;
                }
                delete poller;
            }
        }
    }

    const QStringList lstPlugins = pluginCandidates();
    for (const QString &candidate : lstPlugins) {
        if (!QLibrary::isLibrary(candidate)) {
            continue;
        }
        QPluginLoader loader(candidate);
        if (checkPlatform(loader.metaData(), platformName)) {
            auto *poller = qobject_cast<KAbstractIdleTimePoller *>(loader.instance());
            if (poller) {
                qCDebug(KIDLETIME) << "Trying plugin" << candidate;
                if (poller->isAvailable()) {
                    qCDebug(KIDLETIME) << "Using" << candidate << "for platform" << platformName;
                    return poller;
                }
                delete poller;
            }
        }
    }

    qCWarning(KIDLETIME) << "Could not find any system poller plugin";
    return nullptr;
}

void KIdleTimePrivate::loadSystem()
{
    if (!poller.isNull()) {
        unloadCurrentSystem();
    }

    // load plugin
    poller = loadPoller();

    if (poller && !poller->isAvailable()) {
        poller = nullptr;
    }
    if (!poller.isNull()) {
        poller.data()->setUpPoller();
    }
}

void KIdleTimePrivate::unloadCurrentSystem()
{
    if (!poller.isNull()) {
        poller.data()->unloadPoller();
        poller.data()->deleteLater();
    }
}

void KIdleTimePrivate::resumingFromIdle()
{
    Q_Q(KIdleTime);

    if (catchResume) {
        Q_EMIT q->resumingFromIdle();
        q->stopCatchingResumeEvent();
    }
}

void KIdleTimePrivate::timeoutReached(int msec)
{
    Q_Q(KIdleTime);

    const auto listKeys = associations.keys(msec);

    for (const auto key : listKeys) {
        Q_EMIT q->timeoutReached(key, msec);
    }
}

void KIdleTime::simulateUserActivity()
{
    Q_D(KIdleTime);

    if (Q_LIKELY(d->poller)) {
        d->poller.data()->simulateUserActivity();
    }
}

int KIdleTime::idleTime() const
{
    Q_D(const KIdleTime);
    if (Q_LIKELY(d->poller)) {
        return d->poller.data()->forcePollRequest();
    }
    return 0;
}

QHash<int, int> KIdleTime::idleTimeouts() const
{
    Q_D(const KIdleTime);

    return d->associations;
}

#include "moc_kidletime.cpp"
