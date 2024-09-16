// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datausagewidget.h"
#include "datausagemodel.h"
#include "speedwidget.h"
#include "appdatadetaildialog.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DListView>
#include <DSimpleListView>
#include <DTipLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QComboBox>
#include <QTableView>
#include <QHeaderView>
#include <functional>
#include <QToolButton>
#include <QMenu>

DWIDGET_USE_NAMESPACE
DEF_USING_ANTIVIRUS_NAMESPACE

DataUsageWidget::DataUsageWidget(DataUsageModel *model, QWidget *parent)
    : DFrame(parent)
    , m_model(model)
    , m_speedwidget(nullptr)
    , m_flowwidget(nullptr)
    , m_layout(new QVBoxLayout)
    , m_widget(new QWidget(this))
    , m_topSwitchWidgetBtn(new Dtk::Widget::DButtonBox)
    , m_tablelayout(new QVBoxLayout)
{
    this->setAccessibleName("rightWidget_dataUsageWidget");
    m_topSwitchWidgetBtn->setAccessibleName("dataUsageWidget_switchButton");

    initUI();
}

DataUsageWidget::~DataUsageWidget()
{
}

//初始化ui界面
void DataUsageWidget::initUI()
{
    //将　widget　加入到一个新建的　DBackgroundGroup中
    QVBoxLayout *bglayout = new QVBoxLayout;
    bglayout->setContentsMargins(0, 0, 0, 0);
    m_bgGroup = new DBackgroundGroup(bglayout, this);
    m_bgGroup->setBackgroundRole(QPalette::Window);
    bglayout->addWidget(m_widget);

    //将 m_bgGroup 沉底
    m_bgGroup->lower();

    //设置m_bgGroup 的大小
    m_bgGroup->setFixedSize(size());

    // 标题栏布局
    QVBoxLayout *tileLayout = new QVBoxLayout;
    tileLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *title = new QLabel(tr("Data Usage"), this);
    title->setAccessibleName("dataUsageWidget_titleLable");
    QFont font1 = title->font();
    font1.setBold(true);
    title->setFont(font1);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5);
    tileLayout->addWidget(title, 0, Qt::AlignTop);

    //按钮item
    DButtonBoxButton *btnUpdate = new DButtonBoxButton(tr("Apps and Services"), this);
    btnUpdate->setAccessibleName("switchButton_currentButton");
    DButtonBoxButton *btnSetting = new DButtonBoxButton(tr("Rankings"), this);
    btnSetting->setAccessibleName("switchButton_rankingsButton");
    btnUpdate->setFixedWidth(160);
    btnSetting->setFixedWidth(160);
    m_btnlist.append(btnUpdate);
    m_btnlist.append(btnSetting);
    m_topSwitchWidgetBtn->setButtonList(m_btnlist, true);
    m_topSwitchWidgetBtn->setMaximumSize(320, 36);
    m_btnlist.first()->setChecked(true);
    m_topSwitchWidgetBtn->setId(btnUpdate, 0);
    m_topSwitchWidgetBtn->setId(btnSetting, 1);
    m_topSwitchWidgetBtn->setMinimumSize(320, 36);
    m_btnlist.at(0)->setChecked(true);
    connect(m_topSwitchWidgetBtn, &DButtonBox::buttonClicked, this, [this](QAbstractButton *value) {
        settype(static_cast<TabelType>(m_topSwitchWidgetBtn->id(value)));
    });

    m_layout->setMargin(0);
    m_layout->addLayout(tileLayout);
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->addWidget(m_topSwitchWidgetBtn, 0, Qt::AlignHCenter);

    setLayout(m_layout);

    m_layout->addLayout(m_tablelayout);

    showAppsServices();
}

//根据按钮显示对应界面
void DataUsageWidget::settype(TabelType type)
{
    m_tabeltype = type;
    switch (type) {
    case AppsServices:
        showAppsServices();
        break;
    case Trafficranking:
        showTrafficranking();
        break;
    }
}
//网速界面
void DataUsageWidget::showAppsServices()
{
    if (m_flowwidget != nullptr) {
        m_flowwidget->hide();
        m_tablelayout->removeWidget(m_flowwidget);
        disconnect(m_flowwidget);
        m_flowwidget->deleteLater();
        m_flowwidget = nullptr;
    }

    if (nullptr == m_speedwidget) {
        m_speedwidget = new SpeedWidget(m_model, this);
        m_speedwidget->setContentsMargins(0, 0, 0, 0);
        m_tablelayout->addWidget(m_speedwidget);
        m_speedwidget->show();
    }
}

//流量界面
void DataUsageWidget::showTrafficranking()
{
    if (m_speedwidget != nullptr) {
        m_speedwidget->hide();
        m_tablelayout->removeWidget(m_speedwidget);
        disconnect(m_speedwidget);
        m_speedwidget->deleteLater();
        m_speedwidget = nullptr;
    }

    if (nullptr == m_flowwidget) {
        m_flowwidget = new FlowWidget(m_model, this);
        m_flowwidget->setContentsMargins(0, 0, 0, 0);
        m_tablelayout->addWidget(m_flowwidget);
        m_flowwidget->show();
    }
}

//设置背景大小
void DataUsageWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    //设置m_bgGroup 的大小
    if (m_bgGroup)
        m_bgGroup->setFixedSize(size());
}
