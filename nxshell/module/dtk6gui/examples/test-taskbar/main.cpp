// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DGuiApplicationHelper>
#include <DFontManager>

#include <QApplication>

#include "testtaskbarwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto name = QUuid::createUuid().toString() + QStringLiteral(".desktop");
    a.setDesktopFileName(name);
    QDir appDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QFile desktopFile(appDir.absoluteFilePath(name));
    if(!desktopFile.exists()) {
        desktopFile.open(QIODevice::WriteOnly);
        desktopFile.write("[Desktop Entry]\n");
        desktopFile.write("Type=Application\n");
        desktopFile.write("Version=1.1\n");
        desktopFile.write("Name=" + QApplication::applicationDisplayName().toUtf8() + "\n");
        desktopFile.write("Exec=" + QApplication::applicationFilePath().toUtf8() + "\n");
        desktopFile.close();
    }

    TestTaskbarWindow *pTaskbarWindow = new TestTaskbarWindow;
    pTaskbarWindow->showMaximized();

    //这里删除用于测试的desktop文件
    QObject::connect(pTaskbarWindow, &TestTaskbarWindow::closeWindow, [&desktopFile] {
        QFile::remove(desktopFile.fileName());
    });

    //控制中心修改字体大小可以看到打印输出
    QObject::connect(DGuiApplicationHelper::instance()->fontManager(), &DFontManager::fontChanged, [] {
        qDebug() << DGuiApplicationHelper::instance()->fontManager()->baseFont();
    });

    return a.exec();
}
