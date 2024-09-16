// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sidebaritemwidget.h"
#include "accountmanager.h"
#include <DFontSizeManager>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QSizePolicy>

SidebarItemWidget::SidebarItemWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(170);
}

SidebarItemWidget *SidebarItemWidget::getAccountItemWidget(AccountItem::Ptr ptr)
{
    return new SidebarAccountItemWidget(ptr);
}

/**
 * @brief SidebarItemWidget::getLocalWidget
 * 获取子层本地日程item控件，有复选框
 * @param info 本地日程类型
 * @return 子层选项控件
 */
SidebarItemWidget *SidebarItemWidget::getTypeItemWidget(DScheduleType::Ptr ptr)
{
    return new SidebarTypeItemWidget(ptr);
}

/**
 * @brief SidebarItemWidget::setSelectStatus
 * 设置选中状态
 * @param status    将被设置的状态
 */
void SidebarItemWidget::setSelectStatus(bool status)
{
    if (status == m_selectStatus && m_item && status == m_item->isExpanded()) {
        return;
    }
    m_selectStatus = status;
    //根据控件类型设置响应控件状态
    updateStatus();

    if (m_item && m_item->isExpanded() != status) {
        //设置列表展开状态
        m_item->setExpanded(status);
    }
    emit signalStatusChange(m_selectStatus, m_id);
}

/**
 * @brief SidebarItemWidget::getSelectStatus
 * 获取当前状态
 * @return
 */
bool SidebarItemWidget::getSelectStatus()
{
    return m_selectStatus;
}

/**
 * @brief SidebarItemWidget::switchState
 * 切换状态
 */
void SidebarItemWidget::switchState()
{
    setSelectStatus(!m_selectStatus);
}

/**
 * @brief SidebarItemWidget::setItem
 * 设置关联的item
 * @param item 待关联的item
 */
void SidebarItemWidget::setItem(QTreeWidgetItem *item)
{
    m_item = item;
}

QTreeWidgetItem *SidebarItemWidget::getTreeItem()
{
    return m_item;
}

void SidebarItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    switchState();
}

void SidebarItemWidget::mouseReleaseEvent(QMouseEvent *)
{
    //屏蔽鼠标释放事件，解决双击时触发重复事件的问题
}

SidebarTypeItemWidget::SidebarTypeItemWidget(DScheduleType::Ptr ptr, QWidget *parent)
    : SidebarItemWidget(parent)
    , m_scheduleType(ptr)
{
    initView();
    m_id = m_scheduleType->accountID();
}

void SidebarTypeItemWidget::initView()
{
    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->setContentsMargins(5, 0, 0, 0);
    vLayout->setSpacing(0);
    m_checkBox = new QCheckBox(this);
    QPalette palette = m_checkBox->palette();
    palette.setBrush(QPalette::Highlight, QColor(m_scheduleType->getColorCode()));
    m_checkBox->setPalette(palette);
    m_checkBox->setFocusPolicy(Qt::NoFocus);
    setSelectStatus((m_scheduleType->showState() == DScheduleType::Show));
    connect(m_checkBox, &QCheckBox::clicked, this, [this]() {
        setSelectStatus(m_checkBox->isChecked());
    });

    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedHeight(30);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T6);
    m_titleLabel->setElideMode(Qt::ElideRight);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t6();
    labelF.setWeight(QFont::Normal);
    m_titleLabel->setFont(labelF);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setText(m_scheduleType->displayName());
    m_titleLabel->setToolTip(m_scheduleType->displayName());

    vLayout->addSpacing(2);
    vLayout->addWidget(m_checkBox);
    vLayout->addWidget(m_titleLabel, 1);
    this->setLayout(vLayout);

    setFixedHeight(40);
}

void SidebarTypeItemWidget::updateStatus()
{
    AccountItem::Ptr account = gAccountManager->getAccountItemByAccountId(m_scheduleType->accountID());
    if (account) {
        if (m_selectStatus != (m_scheduleType->showState() == DScheduleType::Show)) {
            m_scheduleType->setShowState(m_selectStatus ? DScheduleType::Show : DScheduleType::Hide);
            account->updateScheduleTypeShowState(m_scheduleType);
        }
        m_checkBox->setChecked(m_selectStatus);
    }
}

SidebarAccountItemWidget::SidebarAccountItemWidget(AccountItem::Ptr ptr, QWidget *parent)
    : SidebarItemWidget(parent)
    , m_accountItem(ptr)
{
    initView();
    initConnect();
}

void SidebarAccountItemWidget::initView()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    m_headIconButton = new DIconButton(this);
    m_headIconButton->setFlat(true);
    m_headIconButton->setFixedSize(16, 16);
    m_headIconButton->setIconSize(QSize(10, 10));
    m_headIconButton->setIcon(DStyle::SP_ArrowRight);

    m_headIconButton->setFocusPolicy(Qt::NoFocus);
    connect(m_headIconButton, &DIconButton::clicked, this, [this]() {
        setSelectStatus(!m_selectStatus);
    });
    m_ptrDoaNetwork = new DOANetWorkDBus(this);
    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedHeight(30);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T6);
    m_titleLabel->setElideMode(Qt::ElideMiddle);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t6();
    labelF.setWeight(QFont::Normal);
    m_titleLabel->setFont(labelF);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setTextFormat(Qt::PlainText);
    m_titleLabel->setText(m_accountItem->getAccount()->accountName());
    m_titleLabel->setToolTip("<p style='white-space:pre;'>" + m_accountItem->getAccount()->accountName().toHtmlEscaped());

    m_syncIconButton = new DIconButton(this);
    m_syncIconButton->setIcon(QIcon(":/icons/deepin/builtin/icons/icon_refresh.svg"));
    m_syncIconButton->setFixedSize(QSize(20, 20));
    qreal ratio = qApp->devicePixelRatio();
    if (ratio == 1){
        m_syncIconButton->setIconSize(QSize(10, 10));
    }
    m_syncIconButton->setFocusPolicy(Qt::NoFocus);

    m_warningLabel = new DLabel();
    m_warningLabel->setFixedSize(QSize(20, 20));
    m_warningLabel->setPixmap(QIcon(":/icons/deepin/builtin/icons/dde_calendar_warning_light_32px.svg").pixmap(18, 18));

    hLayout->addWidget(m_headIconButton);
    hLayout->addWidget(m_titleLabel, 1);
    hLayout->addWidget(m_syncIconButton);
    hLayout->addWidget(m_warningLabel);
    //给控件右部留出足够的距离，防止被滚动条覆盖无法被点击事件
    hLayout->addSpacing(15);

    this->setLayout(hLayout);
    if (m_accountItem->getAccount()->accountType() == DAccount::Account_UnionID) {
        resetRearIconButton();
    } else {
        //尾部控件隐藏
        m_syncIconButton->hide();
        m_warningLabel->hide();
    }
    setFixedHeight(36);
}

void SidebarAccountItemWidget::initConnect()
{
    connect(m_syncIconButton, &DIconButton::clicked, this, &SidebarAccountItemWidget::slotRearIconClicked);
    connect(m_accountItem.data(), &AccountItem::signalSyncStateChange, this, &SidebarAccountItemWidget::slotSyncStatusChange);
    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &SidebarAccountItemWidget::slotAccountStateChange);
    connect(m_ptrDoaNetwork, &DOANetWorkDBus::sign_NetWorkChange, this, &SidebarAccountItemWidget::slotNetworkStateChange);
}

void SidebarAccountItemWidget::slotNetworkStateChange(DOANetWorkDBus::NetWorkState state)
{
    //控件不显示则不处理
    if (!m_accountItem || m_accountItem->getAccount()->accountType() != DAccount::Account_UnionID) {
        return;
    }
    if (DOANetWorkDBus::NetWorkState::Disconnect == state) {
        m_syncIconButton->hide();
        m_warningLabel->show();
        QString msg = m_accountItem->getSyncMsg(DAccount::AccountSyncState::Sync_NetworkAnomaly);
        m_warningLabel->setToolTip(msg);
    } else if (DOANetWorkDBus::NetWorkState::Active == state) {
        m_warningLabel->hide();
        if (m_accountItem->isCanSyncSetting() || m_accountItem->isCanSyncShedule()) {
            m_syncIconButton->show();
        } else {
            m_syncIconButton->hide();
        }
    }
}

void SidebarAccountItemWidget::resetRearIconButton()
{
    //控件不显示则不处理
    if (m_accountItem->getAccount()->accountType() != DAccount::Account_UnionID) {
        return;
    }

    if (m_accountItem) {
        if (m_accountItem->getAccount()->syncState() == 0) {
            m_warningLabel->hide();
            if (m_accountItem->isCanSyncSetting() || m_accountItem->isCanSyncShedule()) {
                m_syncIconButton->show();
            } else {
                m_syncIconButton->hide();
            }
        } else {
            m_syncIconButton->hide();
            m_warningLabel->show();
            QString msg = m_accountItem->getSyncMsg(m_accountItem->getAccount()->syncState());
            m_warningLabel->setToolTip(msg);
        }
    }
}

AccountItem::Ptr SidebarAccountItemWidget::getAccountItem()
{
    return m_accountItem;
}

void SidebarAccountItemWidget::updateStatus()
{
    m_accountItem->setAccountExpandStatus(m_selectStatus);
    if (m_selectStatus) {
        m_headIconButton->setIcon(DStyle::SP_ArrowDown);
    } else {
        m_headIconButton->setIcon(DStyle::SP_ArrowRight);
    }
}

//尾部图标控件点击事件
void SidebarAccountItemWidget::slotRearIconClicked()
{
    gAccountManager->downloadByAccountID(m_accountItem->getAccount()->accountID());
}

//同步状态改变事件
void SidebarAccountItemWidget::slotSyncStatusChange(DAccount::AccountSyncState state)
{
    m_accountItem->getAccount()->setSyncState(state);
    resetRearIconButton();
}

void SidebarAccountItemWidget::slotAccountStateChange()
{
    resetRearIconButton();
}

