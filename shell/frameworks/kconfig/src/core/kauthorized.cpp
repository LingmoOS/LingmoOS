/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999, 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kauthorized.h"

#include <QDebug>
#include <QDir>
#include <QList>
#include <QUrl>

#include "kconfig_core_log_settings.h"
#include <QCoreApplication>
#include <ksharedconfig.h>
#include <stdlib.h> // srand(), rand()
#ifndef Q_OS_WIN
#include <netdb.h>
#include <unistd.h>
#endif

#include <kconfiggroup.h>

#include <QMutexLocker>
#include <QRecursiveMutex>

extern bool kde_kiosk_exception;

class URLActionRule
{
public:
#define checkExactMatch(s, b)                                                                                                                                  \
    if (s.isEmpty())                                                                                                                                           \
        b = true;                                                                                                                                              \
    else if (s[s.length() - 1] == QLatin1Char('!')) {                                                                                                          \
        b = false;                                                                                                                                             \
        s.chop(1);                                                                                                                                             \
    } else                                                                                                                                                     \
        b = true;
#define checkStartWildCard(s, b)                                                                                                                               \
    if (s.isEmpty())                                                                                                                                           \
        b = true;                                                                                                                                              \
    else if (s[0] == QLatin1Char('*')) {                                                                                                                       \
        b = true;                                                                                                                                              \
        s.remove(0, 1);                                                                                                                                        \
    } else                                                                                                                                                     \
        b = false;
#define checkEqual(s, b) b = (s == QLatin1String("="));

    URLActionRule(const QByteArray &act,
                  const QString &bProt,
                  const QString &bHost,
                  const QString &bPath,
                  const QString &dProt,
                  const QString &dHost,
                  const QString &dPath,
                  bool perm)
        : action(act)
        , baseProt(bProt)
        , baseHost(bHost)
        , basePath(bPath)
        , destProt(dProt)
        , destHost(dHost)
        , destPath(dPath)
        , permission(perm)
    {
        checkExactMatch(baseProt, baseProtWildCard);
        checkStartWildCard(baseHost, baseHostWildCard);
        checkExactMatch(basePath, basePathWildCard);
        checkExactMatch(destProt, destProtWildCard);
        checkStartWildCard(destHost, destHostWildCard);
        checkExactMatch(destPath, destPathWildCard);
        checkEqual(destProt, destProtEqual);
        checkEqual(destHost, destHostEqual);
    }

    bool baseMatch(const QUrl &url, const QString &protClass) const
    {
        if (baseProtWildCard) {
            if (!baseProt.isEmpty() //
                && !url.scheme().startsWith(baseProt) //
                && (protClass.isEmpty() || (protClass != baseProt))) {
                return false;
            }
        } else {
            if (url.scheme() != baseProt //
                && (protClass.isEmpty() || (protClass != baseProt))) {
                return false;
            }
        }
        if (baseHostWildCard) {
            if (!baseHost.isEmpty() && !url.host().endsWith(baseHost)) {
                return false;
            }
        } else {
            if (url.host() != baseHost) {
                return false;
            }
        }
        if (basePathWildCard) {
            if (!basePath.isEmpty() && !url.path().startsWith(basePath)) {
                return false;
            }
        } else {
            if (url.path() != basePath) {
                return false;
            }
        }
        return true;
    }

    bool destMatch(const QUrl &url, const QString &protClass, const QUrl &base, const QString &baseClass) const
    {
        if (destProtEqual) {
            if (url.scheme() != base.scheme() //
                && (protClass.isEmpty() || baseClass.isEmpty() || protClass != baseClass)) {
                return false;
            }
        } else if (destProtWildCard) {
            if (!destProt.isEmpty() //
                && !url.scheme().startsWith(destProt) //
                && (protClass.isEmpty() || (protClass != destProt))) {
                return false;
            }
        } else {
            if (url.scheme() != destProt //
                && (protClass.isEmpty() || (protClass != destProt))) {
                return false;
            }
        }
        if (destHostWildCard) {
            if (!destHost.isEmpty() && !url.host().endsWith(destHost)) {
                return false;
            }
        } else if (destHostEqual) {
            if (url.host() != base.host()) {
                return false;
            }
        } else {
            if (url.host() != destHost) {
                return false;
            }
        }
        if (destPathWildCard) {
            if (!destPath.isEmpty() && !url.path().startsWith(destPath)) {
                return false;
            }
        } else {
            if (url.path() != destPath) {
                return false;
            }
        }
        return true;
    }

    QByteArray action;
    QString baseProt;
    QString baseHost;
    QString basePath;
    QString destProt;
    QString destHost;
    QString destPath;
    bool baseProtWildCard : 1;
    bool baseHostWildCard : 1;
    bool basePathWildCard : 1;
    bool destProtWildCard : 1;
    bool destHostWildCard : 1;
    bool destPathWildCard : 1;
    bool destProtEqual : 1;
    bool destHostEqual : 1;
    bool permission;
};

Q_DECLARE_TYPEINFO(URLActionRule, Q_RELOCATABLE_TYPE);

class KAuthorizedPrivate
{
public:
    KAuthorizedPrivate()
        : actionRestrictions(false)
        , blockEverything(false)
    {
        Q_ASSERT_X(QCoreApplication::instance(), "KAuthorizedPrivate()", "There has to be an existing QCoreApplication::instance() pointer");

        KSharedConfig::Ptr config = KSharedConfig::openConfig();

        Q_ASSERT_X(config, "KAuthorizedPrivate()", "There has to be an existing KSharedConfig::openConfig() pointer");
        if (!config) {
            blockEverything = true;
            return;
        }
        actionRestrictions = config->hasGroup(QStringLiteral("KDE Action Restrictions")) && !kde_kiosk_exception;
    }

    ~KAuthorizedPrivate()
    {
    }

    bool actionRestrictions : 1;
    bool blockEverything : 1;
    QList<URLActionRule> urlActionRestrictions;
    QRecursiveMutex mutex;
};

Q_GLOBAL_STATIC(KAuthorizedPrivate, authPrivate)
#define KAUTHORIZED_D KAuthorizedPrivate *d = authPrivate()

KAuthorized::KAuthorized()
    : QObject(nullptr)
{
}

bool KAuthorized::authorize(const QString &genericAction)
{
    KAUTHORIZED_D;
    if (d->blockEverything) {
        return false;
    }

    if (!d->actionRestrictions) {
        return true;
    }

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KDE Action Restrictions"));
    return cg.readEntry(genericAction, true);
}

bool KAuthorized::authorize(KAuthorized::GenericRestriction action)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<KAuthorized::GenericRestriction>();

    if (metaEnum.isValid() && action != 0) {
        return KAuthorized::authorize(QString::fromLatin1(metaEnum.valueToKey(action)).toLower());
    }
    qCWarning(KCONFIG_CORE_LOG) << "Invalid GenericRestriction requested" << action;
    return false;
}

bool KAuthorized::authorizeAction(const QString &action)
{
    KAUTHORIZED_D;
    if (d->blockEverything) {
        return false;
    }
    if (!d->actionRestrictions || action.isEmpty()) {
        return true;
    }

    return authorize(QLatin1String("action/") + action);
}

bool KAuthorized::authorizeAction(KAuthorized::GenericAction action)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<KAuthorized::GenericAction>();
    if (metaEnum.isValid() && action != 0) {
        return KAuthorized::authorizeAction(QString::fromLatin1(metaEnum.valueToKey(action)).toLower());
    }
    qCWarning(KCONFIG_CORE_LOG) << "Invalid GenericAction requested" << action;
    return false;
}

bool KAuthorized::authorizeControlModule(const QString &menuId)
{
    if (menuId.isEmpty() || kde_kiosk_exception) {
        return true;
    }
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KDE Control Module Restrictions"));
    return cg.readEntry(menuId, true);
}

// Exported for unittests (e.g. in KIO, we're missing tests for this in kconfig)
KCONFIGCORE_EXPORT void loadUrlActionRestrictions(const KConfigGroup &cg)
{
    KAUTHORIZED_D;
    const QString Any;

    d->urlActionRestrictions.clear();
    d->urlActionRestrictions.append(URLActionRule("open", Any, Any, Any, Any, Any, Any, true));
    d->urlActionRestrictions.append(URLActionRule("list", Any, Any, Any, Any, Any, Any, true));
    // TEST:
    //  d->urlActionRestrictions.append(
    //  URLActionRule("list", Any, Any, Any, Any, Any, Any, false));
    //  d->urlActionRestrictions.append(
    //  URLActionRule("list", Any, Any, Any, "file", Any, QDir::homePath(), true));
    d->urlActionRestrictions.append(URLActionRule("link", Any, Any, Any, QStringLiteral(":internet"), Any, Any, true));
    d->urlActionRestrictions.append(URLActionRule("redirect", Any, Any, Any, QStringLiteral(":internet"), Any, Any, true));

    // We allow redirections to file: but not from internet protocols, redirecting to file:
    // is very popular among KIO workers and we don't want to break them
    d->urlActionRestrictions.append(URLActionRule("redirect", Any, Any, Any, QStringLiteral("file"), Any, Any, true));
    d->urlActionRestrictions.append(URLActionRule("redirect", QStringLiteral(":internet"), Any, Any, QStringLiteral("file"), Any, Any, false));

    // local protocols may redirect everywhere
    d->urlActionRestrictions.append(URLActionRule("redirect", QStringLiteral(":local"), Any, Any, Any, Any, Any, true));

    // Anyone may redirect to about:
    d->urlActionRestrictions.append(URLActionRule("redirect", Any, Any, Any, QStringLiteral("about"), Any, Any, true));

    // Anyone may redirect to mailto:
    d->urlActionRestrictions.append(URLActionRule("redirect", Any, Any, Any, QStringLiteral("mailto"), Any, Any, true));

    // Anyone may redirect to itself, cq. within it's own group
    d->urlActionRestrictions.append(URLActionRule("redirect", Any, Any, Any, QStringLiteral("="), Any, Any, true));

    d->urlActionRestrictions.append(URLActionRule("redirect", QStringLiteral("about"), Any, Any, Any, Any, Any, true));

    int count = cg.readEntry("rule_count", 0);
    QString keyFormat = QStringLiteral("rule_%1");
    for (int i = 1; i <= count; i++) {
        QString key = keyFormat.arg(i);
        const QStringList rule = cg.readEntry(key, QStringList());
        if (rule.count() != 8) {
            continue;
        }
        const QByteArray action = rule[0].toLatin1();
        const QString refProt = rule[1];
        const QString refHost = rule[2];
        QString refPath = rule[3];
        const QString urlProt = rule[4];
        const QString urlHost = rule[5];
        QString urlPath = rule[6];
        const bool bEnabled = (rule[7].compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);

        if (refPath.startsWith(QLatin1String("$HOME"))) {
            refPath.replace(0, 5, QDir::homePath());
        } else if (refPath.startsWith(QLatin1Char('~'))) {
            refPath.replace(0, 1, QDir::homePath());
        }
        if (urlPath.startsWith(QLatin1String("$HOME"))) {
            urlPath.replace(0, 5, QDir::homePath());
        } else if (urlPath.startsWith(QLatin1Char('~'))) {
            urlPath.replace(0, 1, QDir::homePath());
        }

        if (refPath.startsWith(QLatin1String("$TMP"))) {
            refPath.replace(0, 4, QDir::tempPath());
        }
        if (urlPath.startsWith(QLatin1String("$TMP"))) {
            urlPath.replace(0, 4, QDir::tempPath());
        }

        d->urlActionRestrictions.append(URLActionRule(action, refProt, refHost, refPath, urlProt, urlHost, urlPath, bEnabled));
    }
}

namespace KAuthorizedInternal
{
/**
 * Helper for KAuthorized::allowUrlAction in KIO
 * @private
 */
KCONFIGCORE_EXPORT void allowUrlAction(const QString &action, const QUrl &_baseURL, const QUrl &_destURL)
{
    KAUTHORIZED_D;
    QMutexLocker locker((&d->mutex));

    const QString basePath = _baseURL.adjusted(QUrl::StripTrailingSlash).path();
    const QString destPath = _destURL.adjusted(QUrl::StripTrailingSlash).path();

    d->urlActionRestrictions.append(
        URLActionRule(action.toLatin1(), _baseURL.scheme(), _baseURL.host(), basePath, _destURL.scheme(), _destURL.host(), destPath, true));
}

/**
 * Helper for KAuthorized::authorizeUrlAction in KIO
 * @private
 */
KCONFIGCORE_EXPORT bool
authorizeUrlAction(const QString &action, const QUrl &_baseURL, const QUrl &_destURL, const QString &baseClass, const QString &destClass)
{
    KAUTHORIZED_D;
    QMutexLocker locker(&(d->mutex));
    if (d->blockEverything) {
        return false;
    }

    if (_destURL.isEmpty()) {
        return true;
    }

    bool result = false;
    if (d->urlActionRestrictions.isEmpty()) {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KDE URL Restrictions"));
        loadUrlActionRestrictions(cg);
    }

    QUrl baseURL(_baseURL);
    baseURL.setPath(QDir::cleanPath(baseURL.path()));

    QUrl destURL(_destURL);
    destURL.setPath(QDir::cleanPath(destURL.path()));

    for (const URLActionRule &rule : std::as_const(d->urlActionRestrictions)) {
        if ((result != rule.permission) && // No need to check if it doesn't make a difference
            (action == QLatin1String(rule.action.constData())) && rule.baseMatch(baseURL, baseClass)
            && rule.destMatch(destURL, destClass, baseURL, baseClass)) {
            result = rule.permission;
        }
    }
    return result;
}
} // namespace

#include "moc_kauthorized.cpp"
