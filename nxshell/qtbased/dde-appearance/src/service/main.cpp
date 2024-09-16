// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearance1adaptor.h"
#include "dbus/appearance1.h"
#include "modules/common/commondefine.h"

#include <DLog>

#include <QGuiApplication>

using Dtk::Core::DLogManager;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("org.deepin.dde.appearance");
    QTranslator translator;
    QString languagePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QString("dde-appearance/translations"),
                                                  QStandardPaths::LocateDirectory);
    translator.load(languagePath+"/dde-appearance_" + QLocale::system().name());
    app.installTranslator(&translator);

    DLogManager::registerJournalAppender();
    DLogManager::registerConsoleAppender();

    Appearance1 *appearance = new Appearance1();

    new Appearance1Adaptor(appearance);

    bool appearanceRegister = APPEARANCEDBUS.registerService(APPEARANCE_SERVICE);
    bool appearanceObjectRegister = APPEARANCEDBUS.registerObject(APPEARANCE_PATH, APPEARANCE_INTERFACE, appearance);

    if (!appearanceRegister || ! appearanceObjectRegister) {
        qWarning() << "appearance dbus service already registered";
        return -1;
    }

    return app.exec();
}
