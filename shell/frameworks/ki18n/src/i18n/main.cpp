/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2021,2023 Ingo Klöcker <kloecker@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ki18n_logging.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QThread>
#include <QTranslator>

#include <memory>

static bool loadCatalog(const QString &catalog, const QLocale &locale)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    auto translator = std::make_unique<QTranslator>(QCoreApplication::instance());
    if (!translator->load(locale, catalog, QString(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        qCDebug(KI18N) << "Loading the" << catalog << "catalog failed for locale" << locale;
        return false;
    }
    QCoreApplication::instance()->installTranslator(translator.release());
    return true;
}

static bool loadCatalog(const QString &catalog, const QLocale &locale, const QLocale &fallbackLocale)
{
    // try to load the catalog for locale
    if (loadCatalog(catalog, locale)) {
        return true;
    }
    // if this fails, then try the fallback locale (if it's different from locale)
    if (fallbackLocale != locale) {
        return loadCatalog(catalog, fallbackLocale);
    }
    return false;
}

// load global Qt translation, needed in KDE e.g. by lots of builtin dialogs (QColorDialog, QFontDialog) that we use
static void loadTranslation(const QString &localeName, const QString &fallbackLocaleName)
{
    const QLocale locale{localeName};
    const QLocale fallbackLocale{fallbackLocaleName};
    // first, try to load the qt_ meta catalog
    if (loadCatalog(QStringLiteral("qt_"), locale, fallbackLocale)) {
        return;
    }
    // if loading the meta catalog failed, then try loading the four catalogs
    // it depends on, i.e. qtbase, qtscript, qtmultimedia, qtxmlpatterns, separately
    const auto catalogs = {
        QStringLiteral("qtbase_"),
        QStringLiteral("qtscript_"),
        QStringLiteral("qtmultimedia_"),
        QStringLiteral("qtxmlpatterns_"),
    };
    for (const auto &catalog : catalogs) {
        loadCatalog(catalog, locale, fallbackLocale);
    }
}

static QLocale getSystemLocale()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    // On Windows and Apple OSs, we cannot use QLocale::system() if an application-specific
    // language was set by kxmlgui because Qt ignores LANGUAGE on Windows and Apple OSs.
    // The following code is a simplified variant of QSystemLocale::fallbackUiLocale()
    // (in qlocale_unix.cpp) ignoring LC_ALL, LC_MESSAGES, and LANG.
    QString language = qEnvironmentVariable("LANGUAGE");
    if (!language.isEmpty()) {
        language = language.split(QLatin1Char{':'}).constFirst();
        if (!language.isEmpty()) {
            return QLocale{language};
        }
    }
#endif
    return QLocale::system();
}

static void load()
{
    // The way Qt translation system handles plural forms makes it necessary to
    // have a translation file which contains only plural forms for `en`. That's
    // why we load the `en` translation unconditionally, then load the
    // translation for the current locale to overload it.
    QMetaObject::invokeMethod(QCoreApplication::instance(), [] {
        loadCatalog(QStringLiteral("qt_"), QLocale{QStringLiteral("en")});

        const QLocale locale = getSystemLocale();
        if (locale.name() != QStringLiteral("en")) {
            loadTranslation(locale.name(), locale.bcp47Name());
        }
    });
}

Q_COREAPP_STARTUP_FUNCTION(load)
