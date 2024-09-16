// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDBusConnection>
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <QDesktopWidget>
#include <gtest/gtest.h>
#include "../dbusinterface/drawinterface.h"


using namespace testing;

class DrawInterfaceTest:public testing::Test{

public:
    DrawInterface *m_draw;
    QList<QImage> list;
    virtual void SetUp() override{
        std::cout << "start DrawInterfaceTest" << std::endl;
        m_draw = new DrawInterface("com.deepin.Draw", "/com/deepin/Draw", QDBusConnection::sessionBus(), nullptr);
    }

    virtual void TearDown() override{
        delete m_draw;
        system("killall deepin-draw");
        std::cout << "end DrawInterfaceTest" << std::endl;
    }
};

TEST_F(DrawInterfaceTest, openFiles)
{

    QList<QString> list;
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            + "/Wallpapers/desktop.jpg";
    qDebug() << desktopPath;
    list.append(desktopPath);
    m_draw->openFiles(list);
    sleep(1);

}
TEST_F(DrawInterfaceTest, openImages)
{
    QList<QImage> list;
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    QPixmap pix = t_primaryScreen->grabWindow(QApplication::desktop()->winId());
    list.append(pix.toImage());
    m_draw->openImages(list);
}
