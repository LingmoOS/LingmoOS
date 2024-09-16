// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/widgets/toolbox_proxy.h"
#include "src/widgets/toolbutton.h"
#include "src/widgets/playlist_widget.h"
#include "src/widgets/notification_widget.h"
#include <gtest/gtest.h>
#include "src/common/mainwindow.h"
#include "src/libdmr/player_engine.h"
#include "application.h"
#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include <DSlider>
#include <DListWidget>
#include <QMenu>
#include "presenter.h"
#include "titlebar.h"
#include "src/widgets/tip.h"

using namespace dmr;
/*TEST(ToolBox, buttonBoxButton)
{
    MainWindow* w = dApp->getMainWindow();
    ButtonBoxButton *btn = new ButtonBoxButton("test", w);

    btn->show();
    QTest::qWait(400);
    QTest::mouseMove(btn);

    btn->deleteLater();
}*/

TEST(ToolBox, buttonTooltip)
{
    MainWindow* w = dApp->getMainWindow();
    ButtonToolTip *tip = new ButtonToolTip(w);

    tip->setText("123");
    tip->show();
    tip->changeTheme(darkTheme);
    tip->show();

    tip->deleteLater();
}

TEST(ToolBox, notificationWidget)
{
    MainWindow* w = dApp->getMainWindow();
    NotificationWidget *nwBottom = new NotificationWidget(w);
    NotificationWidget *nwNone = new NotificationWidget(w);

    nwBottom->setAnchor(NotificationWidget::ANCHOR_BOTTOM);
    nwNone->setAnchor(NotificationWidget::ANCHOR_NONE);
    nwBottom->show();
    nwNone->show();
    nwBottom->syncPosition(w->geometry());
    nwNone->syncPosition(w->geometry());
}

TEST(ToolBox, tip)
{
    MainWindow* w = dApp->getMainWindow();
    Tip *tip = new Tip(QPixmap(), "", w);

    tip->setText("test");
    tip->setBackground(QBrush(QColor(Qt::white)));
    tip->setRadius(2);
    tip->setBorderColor(QColor(Qt::blue));
    tip->pop(QPoint(200, 300));
    QColor color = tip->borderColor();
    QBrush brush = tip->background();
    tip->deleteLater();
}

TEST(ToolBox, animationLabel)
{
    MainWindow *mw = new MainWindow();
    AnimationLabel *aLabel = new AnimationLabel(mw, mw);
    aLabel->show();

    QEvent moveEvent(QEvent::Move);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(0,0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(aLabel, &moveEvent);
    QApplication::sendEvent(aLabel, &releaseEvent);
}
