/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>
    SPDX-FileCopyrightText: 2008, 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    Parts of this class have been take from the KAboutApplication class, which was
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kaboutplugindialog.h"

#include "kabstractaboutdialog_p.h"
// KF
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KWidgetItemDelegate>
// Qt
#include <QGuiApplication>
#include <QTabWidget>

class KAboutPluginDialogPrivate : public KAbstractAboutDialogPrivate
{
public:
    KAboutPluginDialogPrivate(const KPluginMetaData &pluginMetaData, KAboutPluginDialog *parent)
        : q(parent)
        , pluginMetaData(pluginMetaData)
        , pluginLicense(KAboutLicense::byKeyword(pluginMetaData.license()))
    {
    }

    void init(KAboutPluginDialog::Options opt);

public:
    KAboutPluginDialog *const q;

    const KPluginMetaData pluginMetaData;
    const KAboutLicense pluginLicense;
};

KAboutPluginDialog::KAboutPluginDialog(const KPluginMetaData &pluginMetaData, QWidget *parent)
    : KAboutPluginDialog(pluginMetaData, NoOptions, parent)
{
}

KAboutPluginDialog::KAboutPluginDialog(const KPluginMetaData &pluginMetaData, Options opt, QWidget *parent)
    : QDialog(parent)
    , d(new KAboutPluginDialogPrivate(pluginMetaData, this))
{
    d->init(opt);
}

KAboutPluginDialog::~KAboutPluginDialog()
{
    // The delegates want to be deleted before the items it created
    qDeleteAll(findChildren<KWidgetItemDelegate *>());
}

void KAboutPluginDialogPrivate::init(KAboutPluginDialog::Options opt)
{
    q->setWindowTitle(i18nc("@title:window", "About %1", pluginMetaData.name()));

    // Set up the title widget...
    const QIcon pluginIcon = !pluginMetaData.iconName().isEmpty() ? QIcon::fromTheme(pluginMetaData.iconName()) : qApp->windowIcon();
    QWidget *titleWidget = createTitleWidget(pluginIcon, pluginMetaData.name(), pluginMetaData.version(), q);

    // Then the tab bar...
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    QString extraInformation;
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 87)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    extraInformation = pluginMetaData.extraInformation();
    QT_WARNING_POP
#endif
    // Set up the first page...
    QWidget *aboutWidget = createAboutWidget(pluginMetaData.description(), //
                                             extraInformation,
                                             pluginMetaData.copyrightText(),
                                             pluginMetaData.website(),
                                             {pluginLicense},
                                             q);

    tabWidget->addTab(aboutWidget, i18nc("@title:tab", "About"));

    // And here we go, authors page...
    const int authorCount = pluginMetaData.authors().count();
    if (authorCount) {
        // TODO: add bug report address to plugin metadata
        QWidget *authorWidget = createAuthorsWidget(pluginMetaData.authors(), false, QString(), QString(), q);

        const QString authorPageTitle = i18ncp("@title:tab", "Author", "Authors", authorCount);
        tabWidget->addTab(authorWidget, authorPageTitle);
    }

    // And credits page...
    if (!pluginMetaData.otherContributors().isEmpty()) {
        QWidget *creditWidget = createCreditWidget(pluginMetaData.otherContributors(), q);
        tabWidget->addTab(creditWidget, i18nc("@title:tab", "Thanks To"));
    }

    // Finally, the optional translators page...
    if (!(opt & KAboutPluginDialog::HideTranslators) && !pluginMetaData.translators().isEmpty()) {
        QWidget *translatorWidget = createTranslatorsWidget(pluginMetaData.translators(), q);
        tabWidget->addTab(translatorWidget, i18nc("@title:tab", "Translation"));
    }

    createForm(titleWidget, tabWidget, q);
}
