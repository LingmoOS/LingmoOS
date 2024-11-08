/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QPainterPath>
#include <QDebug>
#include <QDesktopWidget>


#include "noteHeadMenu.h"
#include "ui_noteHeadMenu.h"
#include "resizable_tool_button.h"
#include "paletteWidget.h"
#include <lingmosdk/diagnosetest/libkydatacollect.h>

static QPixmap drawSymbolicColoredPixmap (const QPixmap&, QPushButton *);

noteHeadMenu::noteHeadMenu(QWidget *parent) :
    QWidget(parent)
  , ui(new Ui::noteHeadMenu)
  , colorWidget(PaletteWidget::LINGMO_BLUE)
{
    ui->setupUi(this);
    setMinimumSize(250,34);
    btnInit();
    slotsInit();
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
            this, &noteHeadMenu::handleTabletMode);
}

noteHeadMenu::~noteHeadMenu()
{
    delete ui;
}

void noteHeadMenu::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRect(this->rect());
    p.fillPath(rectPath,QBrush(colorWidget));
    QWidget::paintEvent(event);
}

//void noteHeadMenu::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    requestFullscreen();
//}

void noteHeadMenu::slotsInit()
{
    connect(ui->pushButtonNew, &QPushButton::clicked, this, [=](){
        qDebug() << "emit requestNewNote";
        requestNewNote();
    });

    // 初始化埋点数据，传入应用类型，桌面应用，时间类型，点击事件
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    // 传入事件描述“创建新便签”，所在界面“主界面”，上传事件
    kdk_dia_upload_default(node,"create_new_note_enable_rate_event","mainWindow");
    // 释放内存
    kdk_dia_data_free(node);
}

void noteHeadMenu::btnInit()
{
    ui->pushButtonPalette->setToolTip(tr("Palette"));
    ui->pushButtonPalette->setIcon(QIcon::fromTheme("lingmo-note-color-symbolic"));
//    ui->pushButtonPalette->setFocusPolicy(Qt::NoFocus);
    ui->pushButtonPalette->setProperty("isWindowButton", 0x1);
    ui->pushButtonPalette->setProperty("useIconHighlightEffect", 0x2);
    ui->pushButtonPalette->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    ui->pushButtonPalette->setFlat(true);
    ui->pushButtonPalette->setTabletModeButtonSize(QSize(24,24), QSize(48, 48));

    ui->toolButtonMenu->setToolTip(tr("Menu"));
    ui->toolButtonMenu->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    ui->toolButtonMenu->setProperty("isWindowButton", 0x1);
    ui->toolButtonMenu->setProperty("useIconHighlightEffect", 0x2);
    ui->toolButtonMenu->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    ui->toolButtonMenu->setToolTip(tr("Menu"));
    ui->toolButtonMenu->setAutoRaise(true);
    ui->toolButtonMenu->setTabletModeButtonSize(QSize(24,24), QSize(48, 48));

    ui->pushButtonExit->setToolTip(tr("Close"));
    ui->pushButtonExit->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->pushButtonExit->setFocusPolicy(Qt::NoFocus);
    ui->pushButtonExit->setProperty("isWindowButton",0x02);
    ui->pushButtonExit->setProperty("useIconHighlightEffect", 0x2);
    ui->pushButtonExit->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    ui->pushButtonExit->setTabletModeButtonSize(QSize(24,24), QSize(48, 48));
    ui->pushButtonExit->setFlat(true);

    ui->pushButtonNew->setToolTip(tr("Create New Note"));
    ui->pushButtonNew->setStyleSheet("QPushButton{border-image:url(:/image/1x/new-normal.png);}"
                                     "QPushButton:hover{border-image:url(:/image/1x/new-hover.png);}"
                                     "QPushButton:pressed{border-image:url(:/image/1x/new-click.png);}");

    initMenu();
    connect(ui->toolButtonMenu, &QToolButton::clicked, [=](){

        switch(LingmoUISettingsMonitor::instance().tabletMode()) {
        case LingmoUISettingsMonitor::TabletStatus::PCMode: {
            if (ui->toolButtonMenu != nullptr) {
                QPoint temp = QPoint(ui->toolButtonMenu->mapToGlobal(QPoint(0, 0)).x(), ui->toolButtonMenu->mapToGlobal(QPoint(0, 0)).y() + ui->toolButtonMenu->height());

                QDesktopWidget *desktopWidget = QApplication::desktop();
                QRect           clientRect    = desktopWidget->availableGeometry();

                if (height() + temp.y() < clientRect.height()) {
                    m_menu->move(temp);
                }
                else {
                    m_menu->move(QPoint(ui->toolButtonMenu->mapToGlobal(QPoint(0, 0)).x(), ui->toolButtonMenu->mapToGlobal(QPoint(0, 0)).y() - m_menu->height()));
                }
            }
            m_menu->show();
            break;
        }
        case LingmoUISettingsMonitor::TabletStatus::PADHorizontalMode:
        case LingmoUISettingsMonitor::TabletStatus::PADVerticalMode:
            emit requestShowNote();
            break;
        default:
            break;
        }

    });
}

void noteHeadMenu::initMenu()
{
    m_menu = new QMenu(ui->toolButtonMenu);
    QMenu *m_childMenu = new QMenu(m_menu);
    QAction *m_menuAction = new QAction(m_menu);

    m_menuActionDel = new QAction(m_menu);
    m_topAction = new QAction(m_menu);
    QAction *m_jpgAction = new QAction(m_childMenu);
    QAction *m_pdfAction = new QAction(m_childMenu);
    QAction *m_mailAction = new QAction(m_childMenu);

    m_menu->setProperty("fillIconSymbolicColor", true);
    m_menuAction->setText(tr("Open note list"));
    m_topAction->setText(tr("Always in the front"));
    m_menuActionDel->setText(tr("Delete this note"));
    m_childMenu->setTitle(tr("Share"));

    m_jpgAction->setText("Export to jpg");
    m_pdfAction->setText("Export to pdf");
    m_mailAction->setText("Mail");

    m_childMenu->addAction(m_jpgAction);
    m_childMenu->addAction(m_pdfAction);
    m_childMenu->addAction(m_mailAction);

    //m_menu->addMenu(m_childMenu);
    m_menu->addAction(m_topAction);
    m_menu->addAction(m_menuActionDel);
    m_menu->addAction(m_menuAction);

    connect(m_menu, &QMenu::aboutToShow, this, [=](){
        requestUpdateMenuIcon();
    });

    connect(m_menuAction, &QAction::triggered, this, [=](){
        requestShowNote();
    });
    connect(m_topAction, &QAction::triggered, this, [=](){
        requestTopMost();
    });

}

QPixmap drawSymbolicColoredPixmap(const QPixmap& source, QPushButton *btn)
{
    QColor whiteColor = QColor(0, 0, 0);   //zybAdd 解决story6496:深色模式下，便签页头部按钮颜色显示异常，返回浅色模式后，新建的便签页也会显示异常的问题
    QColor baseColor = btn->palette().color(QPalette::Text).light(150);
    QImage img = source.toImage();

    qDebug() << "drawSymbolicColoredPixmap" << baseColor.red() << baseColor.green() <<baseColor.blue();
    for (int x = 0; x < img.width(); ++x) {
        for (int y = 0; y < img.height(); ++y) {
            auto color = img.pixelColor(x, y);
#if 0
            color.setRed(255 - baseColor.red());
            color.setGreen(255 - baseColor.green());
            color.setBlue(255 - baseColor.blue());
#else
            //zybAdd 解决story6496:深色模式下，便签页头部按钮颜色显示异常，返回浅色模式后，新建的便签页也会显示异常的问题
            color.setRed(255 - whiteColor.red());
            color.setGreen(255 - whiteColor.green());
            color.setBlue(255 - whiteColor.blue());
#endif
            img.setPixelColor(x, y, color);
        }
    }

    return QPixmap::fromImage(img);
}


void noteHeadMenu::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::TabletStatus::PCMode:
        handlePCMode();
        break;
    case LingmoUISettingsMonitor::TabletStatus::PADHorizontalMode:
    case LingmoUISettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}
void noteHeadMenu::handlePADMode()
{
    ui->horizontalLayout->setSpacing(0);
    ui->horizontalLayout->setContentsMargins(16,0,12,0);
    ui->pushButtonExit->hide();
}

void noteHeadMenu::handlePCMode()
{
    ui->horizontalLayout->setSpacing(8);
    ui->horizontalLayout->setContentsMargins(8,8,8,8);
    ui->pushButtonExit->show();
}

void noteHeadMenu::showEvent(QShowEvent *event)
{
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
    QWidget::showEvent(event);
}
