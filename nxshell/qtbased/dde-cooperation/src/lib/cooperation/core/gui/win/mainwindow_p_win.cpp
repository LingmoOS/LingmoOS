// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../mainwindow.h"
#include "../mainwindow_p.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QToolButton>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QPainterPath>

using namespace cooperation_core;
void MainWindowPrivate::initWindow()
{
    q->setObjectName("MainWindow");
    q->setFixedSize(500, 630);

    q->setWindowIcon(QIcon::fromTheme(":/icons/deepin/builtin/icons/dde-cooperation_128px.png"));

    workspaceWidget = new WorkspaceWidget(q);
    q->setCentralWidget(workspaceWidget);
    workspaceWidget->setStyleSheet("background-color: rgba(230,230,230, 0.1);"
                                   "border-bottom-right-radius: 10px;"
                                   "border-bottom-left-radius: 10px;");
}

void MainWindowPrivate::paintEvent(QPaintEvent *event)
{
    QPainter painter(q);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(230, 230, 230));
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(q->rect(), 10, 10);
    painter.drawPath(path);
}

void MainWindowPrivate::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() == Qt::LeftButton) && leftButtonPressed) {
        q->move(event->globalPos() - lastPosition);
    }
}

void MainWindowPrivate::mouseReleaseEvent(QMouseEvent *event)
{
    leftButtonPressed = false;
}

void MainWindowPrivate::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        leftButtonPressed = true;
        lastPosition = event->globalPos() - q->pos();
    }
}

void MainWindowPrivate::initTitleBar()
{
    q->setWindowFlags(Qt::FramelessWindowHint);
    q->setAttribute(Qt::WA_TranslucentBackground);
    QWidget *titleBar = new QWidget(q->centralWidget());
    titleBar->setFixedSize(500, 50);
    titleBar->setStyleSheet("QWidget {"
                            "background-color: white;"
                            "border-top-right-radius: 10px;"
                            "border-top-left-radius: 10px;"
                            "}");
    titleBar->setFocusPolicy(Qt::ClickFocus);

    QToolButton *refreshBtn = new QToolButton(q);
    refreshBtn->setIcon(QIcon(":/icons/deepin/builtin/texts/refresh_14px.svg"));
    refreshBtn->setIconSize(QSize(16, 16));
    refreshBtn->setToolTip(tr("Re-scan for devices"));
    refreshBtn->setStyleSheet("QToolButton {"
                              "background-color: rgba(0,0,0,0.15);"
                              "border-radius: 8px;"
                              "}");
    refreshBtn->setFixedSize(35, 35);
    connect(refreshBtn, &QToolButton::clicked, q, &MainWindow::onLookingForDevices);

    QToolButton *closeButton = new QToolButton(titleBar);
    closeButton->setStyleSheet("QWidget {"
                               "border-top-right-radius: 20px;"
                               "}");
    closeButton->setFixedSize(50, 50);
    closeButton->setIcon(QIcon(":/icons/deepin/builtin/icons/close_normal.svg"));
    closeButton->setIconSize(QSize(50, 50));

    QToolButton *minButton = new QToolButton(titleBar);
    minButton->setIcon(QIcon(":/icons/deepin/builtin/icons/minimise_normal.svg"));
    minButton->setFixedSize(50, 50);
    minButton->setIconSize(QSize(50, 50));

    QToolButton *helpButton = new QToolButton(titleBar);
    helpButton->setFixedSize(50, 50);
    helpButton->setIcon(QIcon(":/icons/deepin/builtin/icons/menu_normal.svg"));
    helpButton->setIconSize(QSize(50, 50));

    QLabel *iconLabel = new QLabel(titleBar);
    iconLabel->setStyleSheet("QWidget {"
                             "border-top-left-radius: 20px;"
                             "}");
    iconLabel->setPixmap(
            QIcon(":/icons/deepin/builtin/icons/dde-cooperation_128px.png").pixmap(35, 35));

    QMenu *menu = new QMenu();
    helpButton->setMenu(menu);
    helpButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    helpButton->setPopupMode(QToolButton::InstantPopup);
    helpButton->setStyleSheet("QToolButton::menu-indicator {"
                              "image: none;"
                              "}");

    QAction *action = new QAction(tr("Settings"), menu);
    action->setData(MenuAction::kSettings);
    menu->addAction(action);

    action = new QAction(tr("Download Windows client"), menu);
    action->setData(MenuAction::kDownloadWindowClient);
    menu->addAction(action);

    QObject::connect(menu, &QMenu::triggered, [this](QAction *act) {
        bool ok{ false };
        int val{ act->data().toInt(&ok) };
        if (ok)
            handleSettingMenuTriggered(val);
    });

    QObject::connect(closeButton, &QToolButton::clicked, q, &MainWindow::close);
    QObject::connect(minButton, &QToolButton::clicked, q, &MainWindow::showMinimized);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(refreshBtn, Qt::AlignLeft);
    titleLayout->addStretch();
    titleLayout->addWidget(helpButton, Qt::AlignHCenter);
    titleLayout->addWidget(minButton, Qt::AlignHCenter);
    titleLayout->addWidget(closeButton, Qt::AlignHCenter);
    titleLayout->setContentsMargins(8, 0, 0, 0);
    titleBar->setLayout(titleLayout);

    q->layout()->addWidget(titleBar);
}
