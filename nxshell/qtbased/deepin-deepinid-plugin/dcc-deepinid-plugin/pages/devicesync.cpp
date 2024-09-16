// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "devicesync.h"
#include "utils.h"
#include "trans_string.h"

#include <widgets/switchwidget.h>

#include <DLabel>
#include <DTipLabel>
#include <DFontSizeManager>
#include <DListView>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DToolButton>

#include <QMouseEvent>
#include <QEnterEvent>


DWIDGET_USE_NAMESPACE
using namespace DCC_NAMESPACE;
Q_DECLARE_METATYPE(QMargins)

DeviceSyncPage::DeviceSyncPage(QWidget *parent) : QWidget(parent)
  , m_bindSwitch(new SwitchWidget(this))
  , m_devList(new DevListView(this))
  , m_devModel(new QStandardItemModel(this))
  , m_delDialog(new DDialog(TransString::getTransString(STRING_DELDLGTITLE), "", this))
{
    initUI();
    initConnection();
    initDelDialog();
}

void DeviceSyncPage::setSyncModel(SyncModel *syncModel)
{
    m_syncModel = syncModel;
    m_currDevice = m_syncModel->getDeviceCode();
    connect(m_syncModel, &SyncModel::userUnbindInfoChanged, this, &DeviceSyncPage::onUserUnbindInfoChanged);
    connect(m_syncModel, &SyncModel::addDeviceList, this, &DeviceSyncPage::onAddDeviceList);
    connect(m_syncModel, &SyncModel::userInfoChanged, this, &DeviceSyncPage::onUserInfoChanged);
}

void DeviceSyncPage::onLogin()
{
    onUserUnbindInfoChanged(m_syncModel->getUbID());
    onUserInfoChanged(m_syncModel->userinfo());
}

void DeviceSyncPage::onUserUnbindInfoChanged(const QString &ubid)
{
    qDebug() << " bind UBID " << ubid;
    m_ubID = ubid;
    onBindStateChanged(!ubid.isEmpty());
}

void DeviceSyncPage::onBindStateChanged(bool state)
{
    m_bindSwitch->setChecked(state);
    //m_bindedTips->setVisible(state);
}

void DeviceSyncPage::onUserInfoChanged(const QVariantMap &infos)
{
    const QString region = infos["Region"].toString();
    if(!region.isEmpty()) {
        if(region != "CN") {
            qDebug() << "not in cn region:" << region;
            makeContentDisable();
        }
        else {
            qDebug() << "now in cn region";
            makeContentDisable(true);
        }
    }

    // 登录后监测绑定
    if (!infos["Username"].toString().isEmpty())
        updateUserBindStatus();
}

void DeviceSyncPage::onCheckAuthorizationFinished(Authority::Result authResult)
{
    bool checked = m_bindSwitch->checked();

    if (Authority::Result::Yes != authResult) {
        m_bindSwitch->setChecked(!checked);
        return;
    }

    if (checked) {
        bindUserAccount();
    } else {
        unbindUserAccount();
    }
}

void DeviceSyncPage::onAddDeviceList(const QList<DeviceInfo> &devlist)
{
    m_devModel->removeRows(0, m_devModel->rowCount());
    QMargins itemMargin(10, 6, 30, 6);
    for (auto &devInfo: devlist)
    {
        DStandardItem *devItem = new DStandardItem();
        devItem->setText(devInfo.devName);
        devItem->setData(QVariant::fromValue(itemMargin), Dtk::MarginsRole);
        devItem->setIcon(QIcon::fromTheme(getDeviceIcon(devInfo.devType)).pixmap(32, 32));
        devItem->setData(QVariant::fromValue(devInfo.devId), Qt::UserRole + 102);
        DViewItemAction *textAction = new DViewItemAction;
        textAction->setText(TransString::getTransString(STRING_OSVER).arg(devInfo.osVersion));
        textAction->setFontSize(DFontSizeManager::T8);
        textAction->setTextColorRole(DPalette::TextTips);
        textAction->setParent(this);
        devItem->setTextActionList({textAction});
        DViewItemAction *rightAction;

        if(devInfo.devCode == m_currDevice)
        {
            rightAction = new DViewItemAction(Qt::AlignVCenter);
            devItem->setData(true, Qt::UserRole + 101);
            rightAction->setText(TransString::getTransString(STRING_CURRDEV));
            rightAction->setFontSize(DFontSizeManager::T8);
        }
        else
        {
            rightAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
            devItem->setData(false, Qt::UserRole + 101);
            QWidget *delWidget = new QWidget(m_devList->viewport());
            QVBoxLayout *delLayout = new QVBoxLayout;
            DLabel *removeLabel = new DLabel;
            removeLabel->setPixmap(QIcon::fromTheme("dcc_del_account").pixmap(12, 14));
            DTipLabel *removeTip = new DTipLabel(TransString::getTransString(STRING_REMOVE));
            delLayout->setSpacing(3);
            delLayout->setMargin(0);
            delLayout->addSpacing(8);
            delLayout->addWidget(removeLabel, 0, Qt::AlignHCenter);
            delLayout->addWidget(removeTip, 0, Qt::AlignHCenter);
            delWidget->setLayout(delLayout);
            DFontSizeManager::instance()->bind(removeTip, DFontSizeManager::T9);

            rightAction->setWidget(delWidget);
            rightAction->setVisible(false);

            connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
                this, [=]() {
                removeLabel->setPixmap(QIcon::fromTheme("dcc_del_account").pixmap(12, 14));
            });

            connect(rightAction, &QAction::triggered, this, [=]{
                int rowNum = 0;
                QString itemDevId = rightAction->data().toString();
                for (; rowNum < m_devModel->rowCount(); ++rowNum) {
                    auto rowItem = m_devModel->item(rowNum);
                    if(rowItem->data(Qt::UserRole + 102).toString() == itemDevId) {
                        break;
                    }
                }

                if(rowNum != m_devModel->rowCount()) {
                    auto item = this->m_devModel->item(rowNum);
                    QString strid = item->data(Qt::UserRole + 102).toString();
                    bool isCurrent = item->data(Qt::UserRole + 101).toBool();
                    if(!isCurrent) {
                        if(m_delDialog->exec() == QDialog::Accepted) {
                            Q_EMIT this->removeDevice(strid);
                            m_devModel->removeRow(rowNum);
                        }
                    }
                }
            });
        }

        rightAction->setData(devInfo.devId);
        devItem->setActionList(Qt::RightEdge, {rightAction});
        if(devInfo.devCode == m_currDevice) {
            m_devModel->insertRow(0, devItem);
        } else {
            m_devModel->appendRow(devItem);
        }
    }
}

void DeviceSyncPage::initUI()
{
    QVBoxLayout *mainlayout = new QVBoxLayout;
    //head part
    QWidget *headWidget = new QWidget;
    QVBoxLayout *headLayout = new QVBoxLayout;
    headLayout->setContentsMargins(0, 0, 0, 0);
    headLayout->setSpacing(5);
    DTipLabel *bindtip = new DTipLabel(TransString::getTransString(STRING_LOCALMSG), this);
    bindtip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bindtip->setWordWrap(true);
    QHBoxLayout *tiplayout = new QHBoxLayout();
    tiplayout->addWidget(bindtip);
    QHBoxLayout *firstlayout = new QHBoxLayout;
    firstlayout->setContentsMargins(0, 0, 0, 0);
    firstlayout->setSpacing(0);

    m_labelWarn = new WarnLabel("");
    m_labelWarn->SetTipText(TransString::getTransString(STRING_CANOTUSE));
    m_labelWarn->setPixmap(QIcon::fromTheme("dcc_not_use").pixmap(24, 24));
    m_labelWarn->setMouseTracking(true);
    m_labelWarn->setVisible(false);
    m_bindSwitch->layout()->setContentsMargins(0, 0, 0, 0);
    DLabel *labelBind = new DLabel(TransString::getTransString(STRING_LOCALTITLE), this);
    firstlayout->addWidget(labelBind, 0, Qt::AlignLeft);
    firstlayout->addStretch();
    firstlayout->addWidget(m_labelWarn);
    firstlayout->addSpacing(10);
    firstlayout->addWidget(m_bindSwitch, 0, Qt::AlignRight);
    headLayout->addLayout(firstlayout);
    headLayout->addSpacing(-5);
    headLayout->addLayout(tiplayout);
    headWidget->setLayout(headLayout);

    DFontSizeManager::instance()->bind(labelBind, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(bindtip, DFontSizeManager::T8, QFont::Thin);
    //device part
    QVBoxLayout *deviceLayout = new QVBoxLayout;
    deviceLayout->setSpacing(5);
    DLabel *trustDevice = new DLabel(TransString::getTransString(STRING_DEVTITLE), this);
    DTipLabel *deviceTip = new DTipLabel(TransString::getTransString(STRING_DEVMSG), this);

    deviceLayout->addWidget(trustDevice, 0, Qt::AlignLeft);
    deviceLayout->addWidget(deviceTip, 0, Qt::AlignLeft);
    DFontSizeManager::instance()->bind(trustDevice, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(deviceTip, DFontSizeManager::T8, QFont::Thin);
    //device list
    m_devList->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_devList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_devList->setSelectionMode(QListView::SelectionMode::NoSelection);
    m_devList->setEditTriggers(DListView::NoEditTriggers);
    m_devList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_devList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_devList->setFrameShape(DListView::NoFrame);
    m_devList->setItemSpacing(10);
    m_devList->setViewportMargins(0, 0, 0, 0);
    m_devList->setModel(m_devModel);
    m_devList->setMouseTracking(true);
    m_devList->setAttribute(Qt::WA_Hover);
    m_devList->setIconSize(QSize(32, 32));

    QHBoxLayout *exheadLayout = new QHBoxLayout;
    exheadLayout->setSpacing(0);
    exheadLayout->setContentsMargins(0, 0, 0, 0);
    exheadLayout->addWidget(headWidget);
    mainlayout->setMargin(20);
    mainlayout->setSpacing(20);
    mainlayout->addLayout(exheadLayout);
    mainlayout->addLayout(deviceLayout);
    mainlayout->addWidget(m_devList);
    mainlayout->addStretch();
    setLayout(mainlayout);
    //
    if (!IsProfessionalSystem) {
        m_bindSwitch->setChecked(false);
        headWidget->hide();
    }
}

void DeviceSyncPage::initConnection()
{
    connect(m_bindSwitch, &SwitchWidget::checkedChanged, this, &DeviceSyncPage::bindChanged);
}

void DeviceSyncPage::updateUserBindStatus()
{
    // 用于监测用户登录后的 绑定登录状态
    QString uuid = m_syncModel->getUUID();
    qDebug() << " updateUserBindInfo : " << uuid;
    if (uuid.isEmpty()) {
        return;
    }

    Q_EMIT requestLocalBindCheck(uuid);
}

void DeviceSyncPage::makeContentDisable(bool bEnable)
{
    if(bEnable) {
        m_bindSwitch->setEnabled(true);
        m_labelWarn->setVisible(false);
    }
    else {
        m_bindSwitch->setEnabled(false);
        m_labelWarn->setVisible(true);
    }
}

void DeviceSyncPage::initDelDialog()
{
    m_delDialog->setIcon(QIcon::fromTheme(QStringLiteral("deepin-browser")));
    m_delDialog->resize(400, 140);
    m_delDialog->addButton(TransString::getTransString(STRING_CANCEL));
    m_delDialog->addButton(TransString::getTransString(STRING_CONFIRM), true, DDialog::ButtonWarning);
    m_delDialog->setOnButtonClickedClose(true);
    connect(this, &DeviceSyncPage::onUserLogout, m_delDialog, &QDialog::reject);
}

QString DeviceSyncPage::getDeviceIcon(DeviceType devType)
{
    QString strType;
    switch (devType)
    {
    case DEVICETYPE_PC:
        strType = QStringLiteral("dcc_pc");
        break;
    case DEVICETYPE_TABLET:
        strType = QStringLiteral("dcc_pad");
        break;
    case DEVICETYPE_NOTEBOOK:
        strType = QStringLiteral("dcc_laptop");
        break;
    case DEVICETYPE_UNKNOWN:
        qWarning() << "unknown device type";
        break;
    }

    return strType;
}

void DeviceSyncPage::bindUserAccount()
{
    QString uuid = m_syncModel->getUUID();
    QString hostName = m_syncModel->getHostName();
    if (uuid.isEmpty() || hostName.isEmpty())
        return;

    Q_EMIT requestBindAccount(uuid, hostName);
}

void DeviceSyncPage::unbindUserAccount()
{
    if (m_ubID.isEmpty()) {
        qWarning() << "ubid is empty";
        return;
    }

    Q_EMIT requestUnBindAccount(m_ubID);
}

DevListView::DevListView(QWidget *parent):DListView(parent)
{
    ;
}

bool DevListView::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    {
        QHoverEvent *hoverEv = dynamic_cast<QHoverEvent*>(e);
        if(hoverEv) {
            hoverEnterEvent(hoverEv->pos());
        }
    }
        break;
    case QEvent::HoverLeave:
        hoverLeaveEvent();
        break;
    default:
        break;
    }

    return QListView::event(e);
}

void DevListView::hoverEnterEvent(const QPoint &p)
{
    QModelIndex itemIndex = indexAt(p);
    QStandardItemModel *devModel = qobject_cast<QStandardItemModel*>(model());
    for (int i = 1; i < model()->rowCount(); ++i) {
        DStandardItem *item = dynamic_cast<DStandardItem*>(devModel->itemFromIndex(devModel->index(i, 0)));
        if(item) {
            DViewItemAction *rightAction = item->actionList(Qt::RightEdge).at(0);
            if(itemIndex.isValid() && itemIndex.row() == i) {
                rightAction->setVisible(true);
            } else {
                rightAction->setVisible(false);
            }
        }
    }
}

void DevListView::hoverLeaveEvent()
{
    QStandardItemModel *devModel = qobject_cast<QStandardItemModel*>(model());
    for (int i = 1; i < model()->rowCount(); ++i) {
        DStandardItem *item = dynamic_cast<DStandardItem*>(devModel->itemFromIndex(devModel->index(i, 0)));
        if(item) {
            DViewItemAction *rightAction = item->actionList(Qt::RightEdge).at(0);
            rightAction->setVisible(false);
        }
    }
}
