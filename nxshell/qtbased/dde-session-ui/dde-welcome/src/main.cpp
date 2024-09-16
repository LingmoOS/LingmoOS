// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwidget.h"
#include "utils.h"
#include "propertygroup.h"
#include "multiscreenmanager.h"

#include <DApplication>
#include <DLog>
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include <DAccessibilityChecker>
#endif

#include <QScreen>
#include <QWindow>
#include <QDesktopWidget>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-welcome");

    if (!app.setSingleInstance(app.applicationName(), DApplication::UserScope))
        return -1;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    app.installTranslator(&translator);

    PropertyGroup *property_group = new PropertyGroup();

    property_group->addProperty("contentVisible");

    auto createFrame = [&] (QScreen *screen) -> QWidget* {
        MainWidget *w = new MainWidget;
        w->setScreen(screen);
        property_group->addObject(w);
        QObject::connect(w, &MainWidget::destroyed, property_group, &PropertyGroup::removeObject);
        w->show();
        return w;
    };

    MultiScreenManager multi_screen_manager;
    multi_screen_manager.register_for_mutil_screen(createFrame);
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    DAccessibilityChecker checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif
    return app.exec();
}
