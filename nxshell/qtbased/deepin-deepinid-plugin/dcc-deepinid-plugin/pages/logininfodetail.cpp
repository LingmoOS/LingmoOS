// Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logininfodetail.h"
#include "utils.h"
#include <widgets/switchwidget.h>

#include <DTipLabel>
#include <DMessageManager>
#include <DPaletteHelper>

#include <QObject>
#include <QWidget>
#include <DListView>
#include <QScrollArea>
#include <QScroller>
#include <QPalette>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

#define LIST_View_SPACE 1

void SettingsItemEx::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QFrame::paintEvent(event);

    QRect drawRect = rect();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_listView && m_listView->isVisible()) {
        const DPalette &dp = DPaletteHelper::instance()->palette(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(dp.brush(DPalette::ItemBackground));

        QPainterPath topPath;
        topPath.moveTo(drawRect.left() + 8, drawRect.top());
        topPath.arcTo(drawRect.left(), drawRect.top(), 16, 16, 90, 90);
        topPath.lineTo(drawRect.left(), m_listView->geometry().top() - LIST_View_SPACE);
        topPath.lineTo(drawRect.right(), m_listView->geometry().top() - LIST_View_SPACE);
        topPath.lineTo(drawRect.right(), drawRect.top() - 16);
        topPath.arcTo(drawRect.right() - 16, drawRect.top(), 16, 16, 0, 90);
        painter.fillPath(topPath, dp.brush(DPalette::ItemBackground));

        QPainterPath bottomPath;
        bottomPath.moveTo(drawRect.left(), drawRect.bottom() - 7);
        bottomPath.arcTo(drawRect.left(), drawRect.bottom() - 16, 16 ,16, 180, 90);
        bottomPath.lineTo(drawRect.right() - 8, drawRect.bottom());
        bottomPath.arcTo(drawRect.right() - 16, drawRect.bottom() - 15, 16, 16, 270, 90);
        painter.fillPath(bottomPath, dp.brush(DPalette::ItemBackground));
    } else {
        const DPalette &dp = DPaletteHelper::instance()->palette(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(dp.brush(DPalette::ItemBackground));
        painter.drawRoundedRect(drawRect, 8, 8);
    }
}

LoginInfoDetailPage::LoginInfoDetailPage(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QStackedLayout(this))
    , m_listModel(new QStandardItemModel(this))
    , m_stateIcon(new SyncStateIcon)
    , m_authority(Authority::instance())
    , m_cloudPage(new CloudSyncPage)
    , m_devPage(new DeviceSyncPage)
    , m_securityPage(new SecurityPage)
{
    initUI();
    initConnection();
}

LoginInfoDetailPage::~LoginInfoDetailPage()
{
}

void LoginInfoDetailPage::setModel(SyncModel *model)
{
    m_model = model;
    m_cloudPage->setSyncModel(m_model);
    m_devPage->setSyncModel(m_model);
    m_securityPage->setSyncModel(m_model);
}

void LoginInfoDetailPage::setWorker(SyncWorker *worker)
{
    m_cloudPage->setSyncWorker(worker);
    m_securityPage->setSyncWorker(worker);
}

void LoginInfoDetailPage::initUI()
{
    setBackgroundRole(QPalette::Base);
    m_mainLayout->addWidget(m_cloudPage);
    m_mainLayout->addWidget(m_devPage);
    m_mainLayout->addWidget(m_securityPage);
    setLayout(m_mainLayout);
}

void LoginInfoDetailPage::initConnection()
{
    if (m_authority) {
        connect(m_authority, &Authority::checkAuthorizationFinished, m_devPage, &DeviceSyncPage::onCheckAuthorizationFinished);
    }

    connect(m_cloudPage, &CloudSyncPage::requestSetModuleState, this, &LoginInfoDetailPage::requestSetModuleState);
    connect(m_cloudPage, &CloudSyncPage::requestSetUtcloudModuleState, this, &LoginInfoDetailPage::requestSetUtcloudModuleState);
    connect(m_cloudPage, &CloudSyncPage::requestSetAutoSync, this, &LoginInfoDetailPage::requestSetAutoSync);
    connect(m_cloudPage, &CloudSyncPage::clearCloudData, this, &LoginInfoDetailPage::clearCloudData);
    connect(m_devPage, &DeviceSyncPage::bindChanged, this, &LoginInfoDetailPage::onBindUserAccountChanged);
    connect(m_devPage, &DeviceSyncPage::requestBindAccount, this, &LoginInfoDetailPage::requestBindAccount);
    connect(m_devPage, &DeviceSyncPage::requestUnBindAccount, this, &LoginInfoDetailPage::requestUnBindAccount);
    connect(m_devPage, &DeviceSyncPage::removeDevice, this, &LoginInfoDetailPage::removeDevice);
    connect(m_devPage, &DeviceSyncPage::requestLocalBindCheck, this, &LoginInfoDetailPage::requestLocalBindCheck);

    connect(this, &LoginInfoDetailPage::selectIndex, m_mainLayout, &QStackedLayout::setCurrentIndex);
    connect(this, &LoginInfoDetailPage::onUserLogout, m_cloudPage, &CloudSyncPage::onUserLogout);
    connect(this, &LoginInfoDetailPage::onUserLogout, m_devPage, &DeviceSyncPage::onUserLogout);
    connect(this, &LoginInfoDetailPage::onUserLogout, m_securityPage, &SecurityPage::onUserLogout);
    // 为避免存在安全漏洞，账户安全信息跳转到web端更改
    connect(m_securityPage, &SecurityPage::onChangeInfo, this, &LoginInfoDetailPage::requestOpenWeb);
}

void LoginInfoDetailPage::onLogin()
{
    m_cloudPage->onLogin();
    m_devPage->onLogin();
    m_securityPage->onLogin();
}

void LoginInfoDetailPage::updateItemCheckStatus(const QString &name, bool visible)
{
    for (int i = 0; i < m_listModel->rowCount(); ++i) {
        auto item = static_cast<DStandardItem *>(m_listModel->item(i));
        if (item->text() != name || item->actionList(Qt::Edge::RightEdge).size() < 1)
            continue;

        auto action = item->actionList(Qt::Edge::RightEdge).first();
        auto checkstatus = visible ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked ;
        auto icon = qobject_cast<DStyle *>(style())->standardIcon(checkstatus);
        action->setIcon(icon);
        m_listView->update(item->index());
        break;
    }
}

void LoginInfoDetailPage::onModuleStateChanged(std::pair<SyncType, bool> state)
{
    auto item = static_cast<DStandardItem *>(m_itemMap[state.first]);
    Q_ASSERT(item);

    auto action = item->actionList(Qt::Edge::RightEdge).first();
    auto checkstatus = state.second ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked;
    auto icon = qobject_cast<DStyle *>(style())->standardIcon(checkstatus);
    action->setIcon(icon);
    m_listView->update(item->index());
}

void LoginInfoDetailPage::onStateChanged(const std::pair<qint32, QString> &state)
{
    qDebug() << " SyncState： " << state.first << state.second;
    if (!SyncModel::isSyncStateValid(state)) {
        return;
    }

    SyncState syncState;

    do {
        // check is sync succeed
        if (SyncModel::isSyncSucceed(state)) {
            syncState = SyncState::Succeed;
            break;
        }

        // check is syncing
        if (SyncModel::isSyncing(state)) {
            syncState = SyncState::Syncing;
            break;
        }

        // check is sync faild
        if (SyncModel::isSyncFailed(state)) {
            syncState = SyncState::Failed;
            break;
        }
    } while (false);

    switch (syncState) {
    case SyncState::Succeed:
        SyncTimeLblVisible(m_model->lastSyncTime() > 0);
        if (m_model->lastSyncTime() > 0) {
            m_stateIcon->setRotatePixmap(QIcon::fromTheme("dcc_sync_ok").pixmap(QSize(16, 16)));
        } else {
            m_stateIcon->setRotatePixmap(QPixmap());
        }
        m_stateIcon->stop();
        break;
    case SyncState::Syncing:
        m_lastSyncTimeLbl->hide();
        //m_lastSyncTimeLbl->setText(tr("Syncing..."));
        m_stateIcon->setRotatePixmap(QIcon::fromTheme("dcc_syncing").pixmap(QSize(16, 16)));
        m_stateIcon->play();
        break;
    case SyncState::Failed:
        SyncTimeLblVisible(m_model->lastSyncTime() > 0);
        m_stateIcon->setRotatePixmap(QPixmap());
        m_stateIcon->stop();
        break;
    }
}

void LoginInfoDetailPage::onCheckAuthorizationFinished(Authority::Result authResult)
{
//    bool checked = m_bindSwitch->checked();

//    if (Authority::Result::Yes != authResult) {
//        m_bindSwitch->setChecked(!checked);
//        return;
//    }

//    if (checked) {
//        bindUserAccount();
//    } else {
//        unbindUserAccount();
//    }
}

void LoginInfoDetailPage::onListViewClicked(const QModelIndex &index)
{
    if (index.row() < 0 || index.row() >= m_listModel->rowCount()) {
        return;
    }

    DStandardItem *item = static_cast<DStandardItem *>(m_listModel->item(index.row()));
    if (!item) {
        return;
    }

    DViewItemActionList actionList = item->actionList(Qt::Edge::RightEdge);
    if (actionList.size() < 1) {
        return;
    }

    Q_EMIT actionList.at(0)->trigger();
}

void LoginInfoDetailPage::onBindUserAccountChanged(bool checked)
{
    // 异步调用polkit鉴权，避免阻塞主界面刷新
    if (m_authority) {
        m_authority->checkAuthorization(QString("com.deepin.daemon.accounts.user-administration"),
                                        UnixProcessSubject(getpid()), Authority::AllowUserInteraction);
    }
}

void LoginInfoDetailPage::showItemDisabledStatus(LoginInfoDetailPage::InfoType status)
{
    if (status == InfoType::Available) {
//        m_bindSwitch->switchButton()->setEnabled(true);
//        m_autoSyncSwitch->switchButton()->setEnabled(true);
        //onAutoSyncChanged(m_syncState);

        m_disabledTips->setVisible(false);
    }

    if (status == InfoType::NoService) {
//        m_bindSwitch->setChecked(false);
//        m_bindSwitch->switchButton()->setEnabled(false);
//        m_autoSyncSwitch->setChecked(false);
//        m_autoSyncSwitch->switchButton()->setEnabled(false);

        m_listView->setVisible(false);
        SyncTimeLblVisible(false);

        m_disabledTips->setVisible(true);
        //m_disabledTips->setText(tr("UOS Cloud is currently unavailable in your region"));
    }

    if (status == InfoType::NoActive) {
//        m_bindSwitch->setChecked(false);
//        m_bindSwitch->switchButton()->setEnabled(false);
//        m_autoSyncSwitch->setChecked(false);
//        m_autoSyncSwitch->switchButton()->setEnabled(false);

//        m_bindSwitch->setEnabled(false);
//        m_autoSyncSwitch->setEnabled(false);
        //m_disabledTips->setText(tr("The feature is not available at present, please activate your system first"));
    }
}

void LoginInfoDetailPage::SyncTimeLblVisible(bool isVisible)
{
//    if (!m_autoSyncSwitch->checked() || !m_autoSyncSwitch->switchButton()->isEnabled()) {
//        m_lastSyncTimeLbl->setVisible(false);
//        return;
//    }
    m_lastSyncTimeLbl->setVisible(isVisible);
}
