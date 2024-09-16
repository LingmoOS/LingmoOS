// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imaddwindow.h"
#include "availwidget.h"

#include "publisher/publisherdef.h"
#include "immodel/immodel.h"
#include "widgets/titlelabel.h"
#include "widgets/buttontuple.h"
#include <DCommandLinkButton>
#include <DSearchEdit>
#include <DFontSizeManager>
#include <DDBusSender>
#include <QProcess>
#include <QTimer>

using namespace dcc_fcitx_configtool::widgets;

IMAddWindow::IMAddWindow(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

IMAddWindow::~IMAddWindow()
{
    DeleteObject_Null(m_searchLEdit);
    DeleteObject_Null(m_availWidget);
    DeleteObject_Null(m_storeBtn);
    DeleteObject_Null(m_buttonTuple);
    DeleteObject_Null(m_mainLayout);
}

void IMAddWindow::initUI()
{
    //界面布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 10);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addSpacing(10);
    //添加输入法标题
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    FcitxTitleLabel *title = new FcitxTitleLabel(tr("Add Input Method"), this);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T3, QFont::DemiBold); // 设置label字体
    hlayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlayout->addWidget(title);
    hlayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    //搜索框
    QHBoxLayout *hlayout2 = new QHBoxLayout(this);
    m_searchLEdit = new DSearchEdit(this);
    m_searchLEdit->setText(tr("Search"));
    hlayout2->addSpacing(10);
    hlayout2->addWidget(m_searchLEdit);
    hlayout2->addSpacing(10);
    //可用输入法列表
    m_availWidget = new AvailWidget(this);
    m_availWidget->setAccessibleName("availWidget");
    //应用商店按钮
    QHBoxLayout *hlayout3 = new QHBoxLayout(this);
    hlayout3->addStretch();
    m_storeBtn = new DCommandLinkButton(tr("Find more in App Store"), this);
    m_storeBtn->setAccessibleName("Find more in App Store");
    hlayout3->addWidget(m_storeBtn);
    hlayout3->addSpacing(10);
    //添加 取消按钮
    m_buttonTuple = new FcitxButtonTuple(FcitxButtonTuple::Save);
    m_buttonTuple->rightButton()->setText(tr("Add"));
    m_buttonTuple->rightButton()->setAccessibleName("AddBtn");
    m_buttonTuple->leftButton()->setText(tr("Cancel"));
    m_buttonTuple->leftButton()->setAccessibleName("CancelBtn");
    QHBoxLayout *hlayout4 = new QHBoxLayout(this);
    hlayout4->addSpacing(10);
    hlayout4->addWidget(m_buttonTuple, 0, Qt::AlignBottom);
    hlayout4->addSpacing(10);
    //添加至主界面内
    m_mainLayout->addLayout(hlayout);
    m_mainLayout->addLayout(hlayout2);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(m_availWidget);
    m_mainLayout->addSpacing(6);
    m_mainLayout->addLayout(hlayout3);
    m_mainLayout->addSpacing(7);
    m_mainLayout->addLayout(hlayout4);
}

void IMAddWindow::initConnect()
{
    connect(m_storeBtn, &DCommandLinkButton::clicked, this, &IMAddWindow::onOpenStore);
    connect(m_buttonTuple->rightButton(), &QPushButton::clicked, this, &IMAddWindow::onAddIM);
    connect(m_buttonTuple->leftButton(), &QPushButton::clicked, this, &IMAddWindow::popSettingsWindow);
    connect(m_buttonTuple->leftButton(), &QPushButton::clicked, m_availWidget, &AvailWidget::clearItemStatus);
    connect(m_availWidget, &AvailWidget::seleteIM, m_buttonTuple->rightButton(), &QPushButton::setEnabled);
    connect(m_searchLEdit, &DSearchEdit::textChanged, m_availWidget, &AvailWidget::onSearchIM);
    connect(this, &IMAddWindow::addIM, IMModel::instance(), &IMModel::onAddIMItem);
    connect(this, &IMAddWindow::addIM, this, &IMAddWindow::doRemoveSeleteIm);
    connect(this,static_cast<void (IMAddWindow:: *)(const FcitxQtInputMethodItem &)>(&IMAddWindow::pushItemAvailwidget), [=](const FcitxQtInputMethodItem &item){
        QTimer::singleShot(1, this, [&]() {
                m_availWidget->addSeleteIm(item);
        });
    });
}

void IMAddWindow::doRemoveSeleteIm(const FcitxQtInputMethodItem & temp)
{
    QTimer::singleShot(1, this, [&]() {
        m_availWidget->removeSeleteIm(temp);
    });
}

void IMAddWindow::updateUI()
{
    m_buttonTuple->rightButton()->setEnabled(false);
    m_buttonTuple->leftButton()->setDefault(true);
    if (!m_searchLEdit->text().isEmpty()) {
        m_searchLEdit->clear();
    }
    QTimer::singleShot(1, this, [&]() {
        m_availWidget->onUpdateUI();
        m_availWidget->clearItemStatus();
    });
}

void IMAddWindow::onAddIM()
{
   emit addIM(m_availWidget->getSeleteIm());
   emit popSettingsWindow();
}

void IMAddWindow::onOpenStore()
{
    DDBusSender().service("com.home.appstore.client")
            .interface("com.home.appstore.client")
            .path("/com/home/appstore/client")
            .method("com.home.appstore.client.openBusinessUri")
            .arg(QString("searchApp/输入法"))
            .call();
}
