/*
    SPDX-FileCopyrightText: 2024 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstylemanager.h"

#include <KActionMenu>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

void KStyleManager::initStyle()
{
    // do nothing if we have the proper platform theme already
    if (QGuiApplicationPrivate::platformTheme() && QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("kde")) {
        return;
    }

    // get config, with fallback to kdeglobals
    const auto config = KSharedConfig::openConfig();

    // enforce the style configured by the user, with kdeglobals fallback
    // if not set or the style is not there, use Breeze
    QString styleToUse = KConfigGroup(config, QStringLiteral("General")).readEntry("widgetStyle", QString());
    if (styleToUse.isEmpty() || !QApplication::setStyle(styleToUse)) {
        styleToUse = QStringLiteral("breeze");
        QApplication::setStyle(styleToUse);
    }
}

QAction *KStyleManager::createConfigureAction(QObject *parent)
{
    // if we are running with our application theme, just return a disabled & hidden action
    // there we just follow the Plasma theme
    if (QGuiApplicationPrivate::platformTheme() && QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("kde")) {
        QAction *a = new QAction(parent);
        a->setEnabled(false);
        a->setVisible(false);
        return a;
    }

    // else: provide style chooser

    // get config, without fallback to kdeglobals
    const auto config = KSharedConfig::openConfig(QString(), KConfig::NoGlobals);
    const QString styleWeUse = KConfigGroup(config, QStringLiteral("General")).readEntry("widgetStyle", QString());

    // build menu with default to reset setting and all known styles
    KActionMenu *menu = new KActionMenu(QIcon::fromTheme(QStringLiteral("preferences-desktop-theme-applications")), i18n("Application Style"), parent);
    QActionGroup *group = new QActionGroup(menu);
    const QStringList styles = QStringList() << QString() << QStyleFactory::keys();
    for (const QString &style : styles) {
        QAction *action = new QAction(style.isEmpty() ? i18n("Default") : style, menu);
        action->setData(style);
        action->setActionGroup(group);
        action->setCheckable(true);
        if (style.toLower() == styleWeUse.toLower()) {
            action->setChecked(true);
        }
        menu->addAction(action);
    }

    // toggle style change
    QObject::connect(group, &QActionGroup::triggered, group, [](QAction *action) {
        const QString styleToUse = action->data().toString();
        const auto config = KSharedConfig::openConfig();
        if (styleToUse.isEmpty()) {
            KConfigGroup(config, QStringLiteral("General")).deleteEntry("widgetStyle");
        } else {
            KConfigGroup(config, QStringLiteral("General")).writeEntry("widgetStyle", styleToUse);
        }
        initStyle();
    });

    return menu;
}
