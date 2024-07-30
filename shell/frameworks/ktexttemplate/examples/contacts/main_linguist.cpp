/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "mainwindow.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include <KTextTemplate/QtLocalizer>

#include "ktexttemplate_paths.h"

#define TEMPLATE_DIR KTEXTTEMPLATE_TEMPLATE_PATH "/linguist/"

static QSharedPointer<KTextTemplate::AbstractLocalizer> getLocalizer()
{
    QSharedPointer<KTextTemplate::QtLocalizer> localizer = QSharedPointer<KTextTemplate::QtLocalizer>(new KTextTemplate::QtLocalizer);
    localizer->setAppTranslatorPrefix("contacts_");
    localizer->setAppTranslatorPath(qApp->applicationDirPath());

    QStringList locales = QStringList() << "en_US"
                                        << "en_GB"
                                        << "de_DE"
                                        << "fr_FR";
    if (!locales.contains(QLocale::system().name()))
        locales.append(QLocale::system().name());
    for (const QString &localeName : locales) {
        QTranslator *qtTranslator = new QTranslator;
        qtTranslator->load("qt_" + localeName, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        qtTranslator->setObjectName("qt_" + localeName);
        localizer->installTranslator(qtTranslator, localeName);

        QTranslator *myappTranslator = new QTranslator;
        myappTranslator->load("contacts_" + localeName + ".qm", qApp->applicationDirPath());
        myappTranslator->setObjectName("contacts_" + localeName);
        localizer->installTranslator(myappTranslator, localeName);
    }

    return localizer.staticCast<KTextTemplate::AbstractLocalizer>();
}

template<>
void AppMainWindow<KTextTemplate::QtLocalizer>::initLocalizer()
{
    m_localizer = getLocalizer();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppMainWindow<KTextTemplate::QtLocalizer> win(TEMPLATE_DIR);
    win.show();

    return app.exec();
}
