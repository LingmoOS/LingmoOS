/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2012, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config.h"

#include <kcatalog_p.h>

#include "ki18n_logging.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSet>
#include <QStandardPaths>
#include <QStringList>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#if __ANDROID_API__ < 23
#include <dlfcn.h>
#endif
#endif

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <locale.h>
#include <stdlib.h>

#include "gettext.h" // Must be included after <stdlib.h>

// not defined on win32 :(
#ifdef _WIN32
#ifndef LC_MESSAGES
#define LC_MESSAGES 42
#endif
#endif

#if HAVE_NL_MSG_CAT_CNTR
extern "C" int Q_DECL_IMPORT _nl_msg_cat_cntr;
#endif

static char *s_langenv = nullptr;
static const int s_langenvMaxlen = 64;
// = "LANGUAGE=" + 54 chars for language code + terminating null \0 character

static void copyToLangArr(const QByteArray &lang)
{
    const int bytes = std::snprintf(s_langenv, s_langenvMaxlen, "LANGUAGE=%s", lang.constData());
    if (bytes < 0) {
        qCWarning(KI18N) << "There was an error while writing LANGUAGE environment variable:" << std::strerror(errno);
    } else if (bytes > (s_langenvMaxlen - 1)) { // -1 for the \0 character
        qCWarning(KI18N) << "The value of the LANGUAGE environment variable:" << lang << "( size:" << lang.size() << "),\n"
                         << "was longer than (and consequently truncated to) the max. length of:" << (s_langenvMaxlen - strlen("LANGUAGE=") - 1);
    }
}

class KCatalogStaticData
{
public:
    KCatalogStaticData()
    {
#ifdef Q_OS_ANDROID
        QJniEnvironment env;
        QJniObject context = QNativeInterface::QAndroidApplication::context();
        m_assets = context.callObjectMethod("getAssets", "()Landroid/content/res/AssetManager;");
        m_assetMgr = AAssetManager_fromJava(env.jniEnv(), m_assets.object());

#if __ANDROID_API__ < 23
        fmemopenFunc = reinterpret_cast<decltype(fmemopenFunc)>(dlsym(RTLD_DEFAULT, "fmemopen"));
#endif
#endif
    }

    QHash<QByteArray /*domain*/, QString /*directory*/> customCatalogDirs;
    QMutex mutex;

#ifdef Q_OS_ANDROID
    QJniObject m_assets;
    AAssetManager *m_assetMgr = nullptr;
#if __ANDROID_API__ < 23
    FILE *(*fmemopenFunc)(void *, size_t, const char *);
#endif
#endif
};

Q_GLOBAL_STATIC(KCatalogStaticData, catalogStaticData)

class KCatalogPrivate
{
public:
    KCatalogPrivate();

    QByteArray domain;
    QByteArray language;
    QByteArray localeDir;

    QByteArray systemLanguage;
    bool bindDone;

    static QByteArray currentLanguage;

    void setupGettextEnv();
    void resetSystemLanguage();
};

KCatalogPrivate::KCatalogPrivate()
    : bindDone(false)
{
}

QByteArray KCatalogPrivate::currentLanguage;

KCatalog::KCatalog(const QByteArray &domain, const QString &language_)
    : d(new KCatalogPrivate)
{
    d->domain = domain;
    d->language = QFile::encodeName(language_);
    d->localeDir = QFile::encodeName(catalogLocaleDir(domain, language_));

    if (!d->localeDir.isEmpty()) {
        // Always get translations in UTF-8, regardless of user's environment.
        bind_textdomain_codeset(d->domain, "UTF-8");

        // Invalidate current language, to trigger binding at next translate call.
        KCatalogPrivate::currentLanguage.clear();

        if (!s_langenv) {
            // Call putenv only here, to initialize LANGUAGE variable.
            // Later only change s_langenv to what is currently needed.
            // This doesn't work on Windows though, so there we need putenv calls on every change
            s_langenv = new char[s_langenvMaxlen];
            copyToLangArr(qgetenv("LANGUAGE"));
            putenv(s_langenv);
        }
    }
}

KCatalog::~KCatalog() = default;

#if defined(Q_OS_ANDROID) && __ANDROID_API__ < 23
static QString androidUnpackCatalog(const QString &relpath)
{
    // the catalog files are no longer extracted to the local file system
    // by androiddeployqt starting with Qt 5.14, libintl however needs
    // local files rather than qrc: or asset: URLs, so we unpack the .mo
    // files on demand to the local cache folder

    const QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/") + relpath;
    QFileInfo cacheFile(cachePath);
    if (cacheFile.exists()) {
        return cachePath;
    }

    const QString assetPath = QLatin1String("assets:/share/locale/") + relpath;
    if (!QFileInfo::exists(assetPath)) {
        return {};
    }

    QDir().mkpath(cacheFile.absolutePath());
    QFile f(assetPath);
    if (!f.copy(cachePath)) {
        qCWarning(KI18N) << "Failed to copy catalog:" << f.errorString() << assetPath << cachePath;
        return {};
    }
    return cachePath;
}
#endif

QString KCatalog::catalogLocaleDir(const QByteArray &domain, const QString &language)
{
    QString relpath = QStringLiteral("%1/LC_MESSAGES/%2.mo").arg(language, QFile::decodeName(domain));

    {
        QMutexLocker lock(&catalogStaticData->mutex);
        const QString customLocaleDir = catalogStaticData->customCatalogDirs.value(domain);
        const QString filename = customLocaleDir + QLatin1Char('/') + relpath;
        if (!customLocaleDir.isEmpty() && QFileInfo::exists(filename)) {
#if defined(Q_OS_ANDROID)
            // The exact file name must be returned on Android because libintl-lite loads a catalog by filename with bindtextdomain()
            return filename;
#else
            return customLocaleDir;
#endif
        }
    }

#if defined(Q_OS_ANDROID)
#if __ANDROID_API__ < 23
    // fall back to copying the catalog to the file system on old systems
    // without fmemopen()
    if (!catalogStaticData->fmemopenFunc) {
        return androidUnpackCatalog(relpath);
    }
#endif
    const QString assetPath = QLatin1String("assets:/share/locale/") + relpath;
    if (!QFileInfo::exists(assetPath)) {
        return {};
    }
    return assetPath;

#else
    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("locale/") + relpath);
#ifdef Q_OS_WIN
    // QStandardPaths fails on Windows for executables that aren't properly deployed yet, such as unit tests
    if (file.isEmpty()) {
        const QString p = QLatin1String(INSTALLED_LOCALE_PREFIX) + QLatin1String("/bin/data/locale/") + relpath;
        if (QFile::exists(p)) {
            file = p;
        }
    }
#endif

    QString localeDir;
    if (!file.isEmpty()) {
        // Path of the locale/ directory must be returned.
        localeDir = QFileInfo(file.left(file.size() - relpath.size())).absolutePath();
    }
    return localeDir;
#endif
}

QSet<QString> KCatalog::availableCatalogLanguages(const QByteArray &domain_)
{
    QString domain = QFile::decodeName(domain_);
    QStringList localeDirPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("locale"), QStandardPaths::LocateDirectory);
#ifdef Q_OS_WIN
    // QStandardPaths fails on Windows for executables that aren't properly deployed yet, such as unit tests
    localeDirPaths += QLatin1String(INSTALLED_LOCALE_PREFIX) + QLatin1String("/bin/data/locale/");
#endif

    {
        QMutexLocker lock(&catalogStaticData->mutex);
        auto it = catalogStaticData->customCatalogDirs.constFind(domain_);
        if (it != catalogStaticData->customCatalogDirs.constEnd()) {
            localeDirPaths.prepend(*it);
        }
    }

    QSet<QString> availableLanguages;
    for (const QString &localDirPath : std::as_const(localeDirPaths)) {
        QDir localeDir(localDirPath);
        const QStringList languages = localeDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        for (const QString &language : languages) {
            QString relPath = QStringLiteral("%1/LC_MESSAGES/%2.mo").arg(language, domain);
            if (localeDir.exists(relPath)) {
                availableLanguages.insert(language);
            }
        }
    }
    return availableLanguages;
}

#ifdef Q_OS_ANDROID
static void androidAssetBindtextdomain(const QByteArray &domain, const QByteArray &localeDir)
{
    AAsset *asset = AAssetManager_open(catalogStaticData->m_assetMgr, localeDir.mid(8).constData(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        qWarning() << "unable to load asset" << localeDir;
        return;
    }

    off64_t size = AAsset_getLength64(asset);
    const void *buffer = AAsset_getBuffer(asset);
#if __ANDROID_API__ >= 23
    FILE *moFile = fmemopen(const_cast<void *>(buffer), size, "r");
#else
    FILE *moFile = catalogStaticData->fmemopenFunc(const_cast<void *>(buffer), size, "r");
#endif
    loadMessageCatalogFile(domain, moFile);
    fclose(moFile);
    AAsset_close(asset);
}
#endif

void KCatalogPrivate::setupGettextEnv()
{
    // Point Gettext to current language, recording system value for recovery.
    systemLanguage = qgetenv("LANGUAGE");
    if (systemLanguage != language) {
        // putenv has been called in the constructor,
        // it is enough to change the string set there.
        copyToLangArr(language);
#ifdef Q_OS_WINDOWS
        putenv(s_langenv);
#endif
    }

    // Rebind text domain if language actually changed from the last time,
    // as locale directories may differ for different languages of same catalog.
    if (language != currentLanguage || !bindDone) {
        Q_ASSERT_X(QCoreApplication::instance(), "KCatalogPrivate::setupGettextEnv", "You need to instantiate a Q*Application before using KCatalog");
        if (!QCoreApplication::instance()) {
            qCWarning(KI18N) << "KCatalog being used without a Q*Application instance. Some translations won't work";
        }

        currentLanguage = language;
        bindDone = true;

        // qDebug() << "bindtextdomain" << domain << localeDir;
#ifdef Q_OS_ANDROID
        if (localeDir.startsWith("assets:/")) {
            androidAssetBindtextdomain(domain, localeDir);
        } else {
            bindtextdomain(domain, localeDir);
        }
#else
        bindtextdomain(domain, localeDir);
#endif

#if HAVE_NL_MSG_CAT_CNTR
        // Magic to make sure GNU Gettext doesn't use stale cached translation
        // from previous language.
        ++_nl_msg_cat_cntr;
#endif
    }
}

void KCatalogPrivate::resetSystemLanguage()
{
    if (language != systemLanguage) {
        copyToLangArr(systemLanguage);
#ifdef Q_OS_WINDOWS
        putenv(s_langenv);
#endif
    }
}

QString KCatalog::translate(const QByteArray &msgid) const
{
    if (!d->localeDir.isEmpty()) {
        QMutexLocker locker(&catalogStaticData()->mutex);
        d->setupGettextEnv();
        const char *msgid_char = msgid.constData();
        const char *msgstr = dgettext(d->domain.constData(), msgid_char);
        d->resetSystemLanguage();
        return msgstr != msgid_char // Yes we want pointer comparison
            ? QString::fromUtf8(msgstr)
            : QString();
    } else {
        return QString();
    }
}

QString KCatalog::translate(const QByteArray &msgctxt, const QByteArray &msgid) const
{
    if (!d->localeDir.isEmpty()) {
        QMutexLocker locker(&catalogStaticData()->mutex);
        d->setupGettextEnv();
        const char *msgid_char = msgid.constData();
        const char *msgstr = dpgettext_expr(d->domain.constData(), msgctxt.constData(), msgid_char);
        d->resetSystemLanguage();
        return msgstr != msgid_char // Yes we want pointer comparison
            ? QString::fromUtf8(msgstr)
            : QString();
    } else {
        return QString();
    }
}

QString KCatalog::translate(const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const
{
    if (!d->localeDir.isEmpty()) {
        QMutexLocker locker(&catalogStaticData()->mutex);
        d->setupGettextEnv();
        const char *msgid_char = msgid.constData();
        const char *msgid_plural_char = msgid_plural.constData();
        const char *msgstr = dngettext(d->domain.constData(), msgid_char, msgid_plural_char, n);
        d->resetSystemLanguage();
        // If original and translation are same, dngettext will return
        // the original pointer, which is generally fine, except in
        // the corner cases where e.g. msgstr[1] is same as msgid.
        // Therefore check for pointer difference only with msgid or
        // only with msgid_plural, and not with both.
        return (n == 1 && msgstr != msgid_char) || (n != 1 && msgstr != msgid_plural_char) ? QString::fromUtf8(msgstr) : QString();
    } else {
        return QString();
    }
}

QString KCatalog::translate(const QByteArray &msgctxt, const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const
{
    if (!d->localeDir.isEmpty()) {
        QMutexLocker locker(&catalogStaticData()->mutex);
        d->setupGettextEnv();
        const char *msgid_char = msgid.constData();
        const char *msgid_plural_char = msgid_plural.constData();
        const char *msgstr = dnpgettext_expr(d->domain.constData(), msgctxt.constData(), msgid_char, msgid_plural_char, n);
        d->resetSystemLanguage();
        return (n == 1 && msgstr != msgid_char) || (n != 1 && msgstr != msgid_plural_char) ? QString::fromUtf8(msgstr) : QString();
    } else {
        return QString();
    }
}

void KCatalog::addDomainLocaleDir(const QByteArray &domain, const QString &path)
{
    QMutexLocker locker(&catalogStaticData()->mutex);
    catalogStaticData()->customCatalogDirs.insert(domain, path);
}
