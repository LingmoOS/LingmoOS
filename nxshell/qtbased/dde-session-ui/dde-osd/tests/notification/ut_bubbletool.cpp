// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "notification/bubbletool.h"
#include "notificationentity.h"
#include "appicon.h"
#include "actionbutton.h"
#undef private

#include <QWidget>

#include <gtest/gtest.h>

class UT_BubbleTool : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new QWidget(nullptr);
        QString appName = "deepin-screen-recorder";
        uint id = 0;
        QString appIcon = "deepin-screenshot";
        QString summary = "截图完成";
        QString body = "1.png";
        QStringList actions ;
        actions << "_open" << "查看";
        QVariantMap hints;

        hints.insert("x-deepin-action-_open", "dde-file-manager,--show-item./home/uos/Desktop/1.png");
        QDBusArgument arg;
        arg.beginStructure();
        arg << 40 << 40 << 1 << false << 8 << 4 << QByteArray(40*40, '0');
        arg.endStructure();
        hints.insert("desktop-entry", arg.asVariant());
        int expireTimeout = -1;
        entity = std::make_shared<NotificationEntity>(appName,
                                                      QString(),
                                                      appIcon,
                                                      summary,
                                                      body,
                                                      actions,
                                                      hints,
                                                      QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                      QString::number(id),
                                                      QString::number(expireTimeout));
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    QWidget *obj = nullptr;
    EntityPtr entity;
};

TEST_F(UT_BubbleTool, coverageTest)
{
    BubbleTool::actionInvoke("0", entity);
    AppIcon appicon;
    appicon.setIcon(entity->appIcon());

    QDBusArgument arg;
    arg.beginStructure();
    arg << 40 << 40 << 1 << false << 8 << 4 << QByteArray(40*40, '0');
    arg.endStructure();
    qDebug() << arg.currentType();

    BubbleTool::converToPixmap(&appicon, arg, entity->id());
    ActionButton actionButton;
    BubbleTool::processActions(&actionButton, QStringList() << "default");


    QByteArray array(10, '0');
    QImage image(1, 1, QImage::Format_ARGB32);
    BubbleTool::copyLineRGB32((QRgb *)image.scanLine(0), array.data(), 1);
    BubbleTool::copyLineARGB32((QRgb *)image.scanLine(0), array.data(), 1);
}

TEST_F(UT_BubbleTool, getAppNameTest)
{
    QString key = "deepin-control-center";
    QString appName = BubbleTool::getDeepinAppName(key);//获取应用名称
}
