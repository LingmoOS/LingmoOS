// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>

#include <QPushButton>

#include <signal.h>
#include <unistd.h>

#include "operatedbusdata.h"
#include "accessible/appaccessible.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.loadTranslator();
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-defender-datainterface");
    a.setQuitOnLastWindowClosed(false);

    // 提供自动化测试所需要的实现
    QAccessible::installFactory([](const QString &classname, QObject *object) {
        QAccessibleInterface *interface = nullptr;
        if (object && object->isWidgetType()) {
            // 实测,QPushButton的access不能通过QAbstractButton实现
            if ("QPushButton" == classname) {
                interface = new AppAccessibleButton(static_cast<QPushButton *>(object));
            } else if ("QLabel" == classname) {
                interface = new AppAccessibleLabel(static_cast<QLabel *>(object));
            } else if ("QAbstractButton" == classname) {
                interface = new AppAccessibleQAbstractButton(static_cast<QAbstractButton *>(object));
            }
        }

        return interface;
    });

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    OperateDBusData dbus;
    return a.exec();
}
