/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include <QTableWidget>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QStyleFactory>
#include <QFileDialog>
#include <QSizePolicy>
#include <QCheckBox>
#include <QSlider>
#include <QStringList>
#include <QProgressDialog>
#include <QMenu>
#include <QList>
#include <QProgressBar>
#include <QTranslator>

#include "ksearchlineedit.h"
#include "kborderbutton.h"
#include "kswitchbutton.h"
#include "ksearchlineedit.h"
#include "kaboutdialog.h"
#include "kborderlessbutton.h"
#include "ktabbar.h"
#include "kwindowbuttonbar.h"
#include "xatom-helper.h"
#include "kuninstalldialog.h"
#include "kprogressdialog.h"
#include "kinputdialog.h"
#include "kslider.h"
#include "knavigationbar.h"
#include "testwidget.h"
#include "ktoolbutton.h"
#include "kballontip.h"
#include "ksecuritylevelbar.h"
#include "kbadge.h"
#include "kpixmapcontainer.h"
#include "kpasswordedit.h"
#include "kprogressbar.h"

TestWidget::TestWidget(QWidget*parent):KWidget(parent)
{
    setMinimumSize(640,800);
    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    //窗体显示在中间
    QDesktopWidget *screenResolution = QApplication::desktop();
    this->move((screenResolution->width() - this->width())/2, (screenResolution->height() - this->height())/2);

    this->setWindowTitle("testWindow");

    KBorderButton *pLeftBtn = new KBorderButton(this);
    pLeftBtn->setText("Left");

    KSwitchButton *pSwitchBtn1 = new KSwitchButton(this);
    pSwitchBtn1->setFixedSize(100,26);
    pSwitchBtn1->setEnabled(false);
    KSwitchButton *pSwitchBtn2 = new KSwitchButton(this);
    pSwitchBtn2->setFixedSize(100,26);


    KBorderButton *pRightBtn = new KBorderButton(this);
    pRightBtn->setText("Right");
    //pRightBtn->setEnabled(false);

    QIcon icon =QIcon::fromTheme("edit-find-symbolic");
    pRightBtn->setIcon(icon);

    KSearchLineEdit *pSearchBar = new KSearchLineEdit(this);
    //默认是带completer的，如果不需要 可以设置为空
    //pSearchBar->setCompleter(nullptr);

    QVBoxLayout *vLayout = new QVBoxLayout();

    QHBoxLayout *hLayout = new QHBoxLayout();

    vLayout->addLayout(hLayout);
    vLayout->addStretch();

    vLayout->addWidget(pSearchBar);

    hLayout = new QHBoxLayout();
    KSlider *slider = new KSlider(Qt::Horizontal,this);
    slider->setSliderType(KSliderType::NodeSlider);
    slider->setRange(0,100);
    slider->setTickInterval(20);
    slider->setValue(33);
    slider->setTickPosition(QSlider::TicksBothSides);

    hLayout->addWidget(slider);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    hLayout->addWidget(pLeftBtn);
    hLayout->addWidget(pRightBtn);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    hLayout->addWidget(pSwitchBtn1);
    hLayout->addWidget(pSwitchBtn2);
    vLayout->addLayout(hLayout);

    KBorderlessButton *borderlessButton1 = new KBorderlessButton(this);
    borderlessButton1->setText("borderless1");

    KProgressDialog *progress1 = new KProgressDialog(this);
    connect(borderlessButton1,&KBorderlessButton::clicked,progress1,&QProgressDialog::show);
    progress1->setValue(20);
    progress1->setShowDetail(true);

    KBorderlessButton *borderlessButton2 = new KBorderlessButton(this);
    borderlessButton2->setText("borderless2");

    KProgressDialog *progress2 = new KProgressDialog(tr("下载"),tr("取消"),0,100,this);
    progress2->setSubContent("下载中...");
    progress2->setSuffix("MB");
    progress2->setWindowTitle("进度对话框");
    progress2->setWindowIcon("lingmo-music");
    progress2->setValue(50);
    progress2->setShowDetail(false);
    connect(borderlessButton2,&KBorderlessButton::clicked,progress2,&KProgressDialog::show);

    hLayout = new QHBoxLayout();
    hLayout->addWidget(borderlessButton1);
    hLayout->addWidget(borderlessButton2);
    vLayout->addLayout(hLayout);

    KBorderlessButton *borderlessButton3 = new KBorderlessButton(QIcon::fromTheme("edit-find-symbolic"),this);
    KBorderlessButton *borderlessButton4 = new KBorderlessButton(QIcon::fromTheme("edit-find-symbolic"),"borderless4",this);

    hLayout = new QHBoxLayout();
    hLayout->addWidget(borderlessButton3);
    hLayout->addWidget(borderlessButton4);
    vLayout->addLayout(hLayout);

    KPixmapContainer* container = new KPixmapContainer(this);
//    QPixmap pixmap;
//    pixmap.load("/home/sunzhen/my.jpg");
//    pixmap = pixmap.scaled(36,36);

//    container->setPixmap(pixmap);
    container->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(36,36));
    container->setValue(1000);
    hLayout = new QHBoxLayout();
    hLayout->addWidget(container);
    vLayout->addLayout(hLayout);

    KPasswordEdit* edit = new KPasswordEdit(this);
    edit->setFixedWidth(200);
    edit->setText("adad");
    //edit->setLoading(true);
    //edit->setState(LoginState::LoginFailed);
    edit->setClearButtonEnabled(true);
    KPasswordEdit* edit2 = new KPasswordEdit(this);
    edit2->setFixedWidth(200);
    edit2->setText("12313");
    edit2->setEnabled(false);
    edit2->setClearButtonEnabled(true);
    hLayout = new QHBoxLayout();
    hLayout->addWidget(edit);
    hLayout->addWidget(edit2);
    vLayout->addLayout(hLayout);

    KProgressBar* progressbar = new KProgressBar(this);
    progressbar->setRange(0,100);
    progressbar->setValue(100);
    progressbar->setTextVisible(true);
    progressbar->setAlignment(Qt::AlignRight);
    hLayout = new QHBoxLayout();
    hLayout->addWidget(progressbar);
    vLayout->addLayout(hLayout);

//    KProgressBar* progressbar2 = new KProgressBar(this);
//    progressbar2->setOrientation(Qt::Vertical);
//    progressbar2->setValue(50);
//    progressbar2->setTextVisible(true);
//    progressbar2->setAlignment(Qt::AlignRight);
//    hLayout = new QHBoxLayout();
//    hLayout->addWidget(progressbar2);
//    vLayout->addLayout(hLayout);

    KBallonTip* ballonTip = new KBallonTip("这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字,这是一段操作信息描述的文字",TipType::Error,this);
    ballonTip->setFixedWidth(400);
    hLayout = new QHBoxLayout();
    hLayout->addWidget(ballonTip);
    vLayout->addLayout(hLayout);
    //ballonTip->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    KSecurityLevelBar* barrrr = new KSecurityLevelBar(this);
    barrrr->setSecurityLevel(SecurityLevel::Medium);
    hLayout = new QHBoxLayout();
    hLayout->addWidget(barrrr);
    vLayout->addLayout(hLayout);

    KBadge* badge1 = new KBadge(this);
    badge1->setValue(-9);
    //badge1->setFontSize(20);
    KBadge* badge2 = new KBadge(this);
    badge2->setValue(99);
    //badge2->setFontSize(10);
    KBadge* badge3 = new KBadge(this);
    badge3->setValue(999);
    //badge3->setMinimumHeight(50);
    badge3->setFontSize(15);
    KBadge* badge4 = new KBadge(this);
    badge4->setValue(9991);
//    badge4->setColor(QColor(255,0,0));
    hLayout = new QHBoxLayout();
    hLayout->addWidget(badge1);
    hLayout->addWidget(badge2);
    hLayout->addWidget(badge3);
    hLayout->addWidget(badge4);
    vLayout->addLayout(hLayout);

    KTabBar *tabBar1 = new KTabBar(KTabBarStyle::SegmentLight,this);
    tabBar1->addTab("first");
    tabBar1->addTab("second");
    tabBar1->addTab("third");

    KTabBar *tabBar2 = new KTabBar(KTabBarStyle::SegmentDark,this);
    tabBar2->addTab("first");
    tabBar2->addTab("second");
    tabBar2->addTab("third");

    KTabBar *tabBar3 = new KTabBar(KTabBarStyle::Sliding,this);
    tabBar3->addTab("first");
    tabBar3->addTab("second");
    tabBar3->addTab("third");

    KUninstallDialog *uninstallDialog = new KUninstallDialog("browser360-cn-stable","104",this);
    KAboutDialog *aboutDialog = new KAboutDialog(this,QIcon::fromTheme("edit-find-symbolic"),"灵墨引导修复工具",tr("Version：2020.1.0"));

    connect(pLeftBtn,&KBorderButton::clicked,uninstallDialog,&KUninstallDialog::show);
    connect(pRightBtn,&KBorderButton::clicked,aboutDialog,&KAboutDialog::show);

    vLayout->addWidget(tabBar1);
    vLayout->addWidget(tabBar2);
    vLayout->addWidget(tabBar3);
    vLayout->addStretch();
    baseBar()->setLayout(vLayout);

    vLayout = new QVBoxLayout();
    KNavigationBar* bar = new KNavigationBar(this);

    QStandardItem * item1 = new QStandardItem(QIcon::fromTheme("system-computer-symbolic"),tr("一级导航"));
    QStandardItem * item2 = new QStandardItem(QIcon::fromTheme("stock-people-symbolic"),tr("二级导航"));
    QList<QStandardItem *> list;
    QStandardItem * item3 = new QStandardItem(QIcon::fromTheme("camera-switch-symbolic"),tr("一组一级导航"));
    QStandardItem * item4 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("一组一级导航"));
    list<<item3<<item4;
    bar->addItem(item1);
    bar->addSubItem(item2);
    bar->addGroupItems(list,"测试一组");
    QStandardItem * item5 = new QStandardItem(QIcon::fromTheme("camera-switch-symbolic"),tr("二组一级导航"));
    QStandardItem * item6 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("二组一级导航"));
    QStandardItem * item7 = new QStandardItem(QIcon::fromTheme("media-eq-symbolic"),tr("二组二级导航"));
    QList<QStandardItem *> list2;
    list2<<item5<<item6;
    bar->addGroupItems(list2,"测试二组");
    bar->addSubItem(item7);

    vLayout->addWidget(bar);
    sideBar()->setLayout(vLayout);
    this->setLayoutType(HorizontalType);
}
TestWidget::~TestWidget()
{

}

