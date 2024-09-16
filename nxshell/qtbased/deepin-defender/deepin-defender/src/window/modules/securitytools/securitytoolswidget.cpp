// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytoolswidget.h"
#include "../common/compixmap.h"

#include <DFontSizeManager>
#include <DListView>
#include <DToolButton>

#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

SecurityToolsWidget::SecurityToolsWidget(DWidget *parent)
    : DWidget(parent)
    , m_guiHelper(nullptr)
    , m_dataInterFaceServer(new DataInterFaceServer("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
{
    this->setAccessibleName("rightWidget_securityToolsWidget");

    // 主题变换
    m_guiHelper = DGuiApplicationHelper::instance();
    connect(m_guiHelper, SIGNAL(themeTypeChanged(ColorType)), this, SLOT(changeThemeType(ColorType)));

    // 初始化UI
    initUI();
}

SecurityToolsWidget::~SecurityToolsWidget()
{
    m_buttonModel->clear();
    m_buttonModel->deleteLater();
    m_buttonModel = nullptr;
}

// 初始化界面
void SecurityToolsWidget::initUI()
{
    // 安全防护功能 垂直布局
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    // 加入DListView
    DListView *securityView = new DListView(this);
    securityView->setAccessibleName("securityToolsWidget_selectListView");
    m_buttonModel = new QStandardItemModel(securityView);
    securityView->setModel(m_buttonModel);

    // 形状
    securityView->setFrameShape(QFrame::Shape::NoFrame);
    // 不能编辑触发
    securityView->setEditTriggers(QListView::NoEditTriggers);
    securityView->setResizeMode(QListView::Adjust);
    securityView->setViewMode(QListView::IconMode);
    securityView->setViewportMargins(QMargins(0, 0, 0, 0));

    // 不可以拖拽
    securityView->setDragEnabled(false);
    securityView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    // 显示图表大小
    securityView->setIconSize(QSize(96, 96));
    securityView->setSpacing(46);
    securityView->setContentsMargins(0, 0, 0, 0);
    // 每个对象大小
    securityView->setItemSize(QSize(170, 170));

    // 设置其他属性
    securityView->clearSelection();
    securityView->clearFocus();
    securityView->clearMask();
    securityView->setSelectionMode(QAbstractItemView::NoSelection);
    securityView->setTextElideMode(Qt::ElideRight);
    securityView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground | DStyledItemDelegate::RoundedBackground));

    // 退出模态页面(子页面)后，清除选中效果
    connect(securityView, &QAbstractItemView::clicked, this, [=] {
        // 失去焦点
        securityView->clearFocus();
    });

    bool bStatus = m_dataInterFaceServer->GetUosResourceStatus();
    QStringList titles;
    QStringList tipText;
    if (bStatus) {
        // 按钮  中间标题
        titles << tr("Data Usage") << tr("Startup Programs") << tr("Internet Control")
               << tr("USB Connection") << tr("Login Safety");
        // 按钮 提示
        tipText << tr("View usage details of applications") << tr("Manage auto startup programs")
                << tr("Allow or disable the Internet access of applications")
                << tr("Control the connection of USB storage devices")
                << tr("Manage security levels of system login password");

        m_iconList << DATA_USAGE_ICON << START_PROGRAMS_ICON << INTERNET_CONTROL_ICON << USB_CONTROL_ICON << LOGIN_SAFETY_ICON;
    } else {
        // 按钮  中间标题
        titles << tr("Data Usage") << tr("Startup Programs")
               << tr("USB Connection") << tr("Login Safety");
        // 按钮 提示
        tipText << tr("View usage details of applications") << tr("Manage auto startup programs")
                << tr("Control the connection of USB storage devices")
                << tr("Manage security levels of system login password");

        m_iconList << DATA_USAGE_ICON << START_PROGRAMS_ICON << USB_CONTROL_ICON << LOGIN_SAFETY_ICON;
    }

    // 导入需要加载的控件
    for (int i = 0; i < titles.size(); ++i) {
        QStandardItem *item = new QStandardItem;
        item->setText(titles[i]);
        item->setToolTip(tipText[i]);
        item->setSizeHint(QSize(60, 80));
        m_buttonModel->appendRow(item);
    }

    connect(securityView, &QListView::clicked, this, &SecurityToolsWidget::doSecurityItemClicked);

    // 设置主题变化图片
    changeThemeType(ColorType::UnknownType);
    // 将DListView加入到垂直布局
    verticalLayout->addWidget(securityView);
}

// 流量/自启动/联网应用点击槽
void SecurityToolsWidget::doSecurityItemClicked(QModelIndex nIndex)
{
    int nType;
    bool bStatus = m_dataInterFaceServer->GetUosResourceStatus();
    if (!bStatus && nIndex.row() > 1) {
        nType = nIndex.row() + 1;
    } else {
        nType = nIndex.row();
    }

    switch (static_cast<SecurityClickType>(nType)) {
    case StartupItem:
        Q_EMIT notifyShowStartupWidget();
        break;
    case NetControlItem:
        Q_EMIT notifyShowNetControlWidget();
        break;
    case DataUsageItem:
        Q_EMIT notifyShowDataUsageWidget();
        break;
    case UsbControl:
        Q_EMIT notifyShowUsbControlWidget();
        break;
    case LoginSafety:
        Q_EMIT notifyShowLoginSafetyWidget();
        break;
    }
}

//设置因主题改变图标
void SecurityToolsWidget::changeThemeType(ColorType themeType)
{
    Q_UNUSED(themeType);

    // 给每一项根据主题设置相应图片
    for (int i = 0; i < m_buttonModel->rowCount(); i++) {
        QStandardItem *item = m_buttonModel->item(i);
        item->setIcon(QIcon::fromTheme(m_iconList.at(i)));
    }
}
