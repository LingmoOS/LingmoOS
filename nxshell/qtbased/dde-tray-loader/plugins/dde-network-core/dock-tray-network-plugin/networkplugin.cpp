// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkplugin.h"

#include "dockcontentwidget.h"
#include "netmanager.h"
#include "netstatus.h"
#include "netview.h"
#include "quickpanelwidget.h"

#include <DGuiApplicationHelper>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QMouseEvent>
#include <QTime>

#include <unistd.h>

#define STATE_KEY "enabled"

using namespace dde::network;
DGUI_USE_NAMESPACE

Q_LOGGING_CATEGORY(DNC, "org.deepin.dde.dock.network");

namespace dde {
namespace network {
NetworkPlugin::NetworkPlugin(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_tipsWidget(nullptr)
    , m_manager(nullptr)
    , m_netView(nullptr)
    , m_netStatus(nullptr)
    , m_isLockScreen(false)
    , m_replacesId(0)
    , m_dockContentWidget(nullptr)
    , m_netCheckAvailable(false)
    , m_netLimited(false)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(QString("/usr/share/dock-tray-network-plugin/translations/dock-network-plugin_%1").arg(QLocale().name()));
    QCoreApplication::installTranslator(translator);
}

NetworkPlugin::~NetworkPlugin()
{
    if (m_netView)
        m_netView->deleteLater();

    if (m_netStatus)
        delete m_netStatus;

    if (m_manager)
        delete m_manager;

    if (m_tipsWidget)
        m_tipsWidget->deleteLater();

    if (m_trayIcon)
        m_trayIcon->deleteLater();
}

const QString NetworkPlugin::pluginName() const
{
    return "network";
}

const QString NetworkPlugin::pluginDisplayName() const
{
    return tr("Network");
}

void NetworkPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    if (m_manager)
        return;

    m_manager = new NetManager(this);
    m_netView = new NetView(m_manager);
    m_netStatus = new NetStatus(m_manager);
    m_dockContentWidget = new DockContentWidget(m_netView, m_manager);

    m_manager->setServiceLoadForNM(false);
    m_manager->setMonitorNetworkNotify(false);
    m_manager->setUseSecretAgent(true);
    m_manager->setNetwork8021XMode(NetManager::ToControlCenterUnderConnect);
    m_manager->setServerKey("dock");
    m_manager->init();
    m_netStatus->setDirection(position() == Dock::Top || position() == Dock::Bottom ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);

    if (!pluginIsDisable())
        loadPlugin();

    connect(m_netStatus, &NetStatus::networkStatusChanged, this, &NetworkPlugin::onNetworkStatusChanged);
    connect(m_manager, &NetManager::netCheckAvailableChanged, this, &NetworkPlugin::onNetCheckAvailableChanged);
    connect(m_netView, &NetView::requestShow, this, &NetworkPlugin::showNetworkDialog);
    connect(m_manager, &NetManager::toControlCenter, this, [ = ] {
        m_proxyInter->requestSetAppletVisible(this, NETWORK_KEY, false);
    });

    m_netCheckAvailable = m_manager->netCheckAvailable();

    connect(m_manager,
            &NetManager::networkNotify,
            this,
            [ this ](const QString &inAppName,
                   int replacesId,
                   const QString &appIcon,
                   const QString &summary,
                   const QString &body,
                   const QStringList &actions,
                   const QVariantMap &hints,
                   int expireTimeout) {
                QDBusMessage notify = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
                uint id = replacesId == -1 ? m_replacesId : static_cast<uint>(replacesId);
                notify << inAppName << id << appIcon << summary << body << actions << hints << expireTimeout;
                QDBusConnection::sessionBus().callWithCallback(notify, this, SLOT(onNotify(uint)));
            });
    connect(m_netStatus, &NetStatus::hasDeviceChanged, this, &NetworkPlugin::refreshPluginItemsVisible);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &NetworkPlugin::updateIconColor);
    QDBusConnection::sessionBus().connect("com.deepin.dde.lockFront", "/com/deepin/dde/lockFront", "com.deepin.dde.lockFront", "Visible", this, SLOT(updateLockScreenStatus(bool)));
}

void NetworkPlugin::positionChanged(const Dock::Position position)
{
    m_proxyInter->itemUpdate(this, pluginName());
    if (m_netStatus)
        m_netStatus->setDirection(position == Dock::Top || position == Dock::Bottom ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
}

void NetworkPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(checked)

    if (itemKey == NETWORK_KEY)
        m_netStatus->invokeMenuItem(menuId);
}

void NetworkPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == NETWORK_KEY)
        m_netStatus->refreshIcon();
}

void NetworkPlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, STATE_KEY, pluginIsDisable());

    refreshPluginItemsVisible();
}

bool NetworkPlugin::pluginIsAllowDisable()
{
    return true;
}

bool NetworkPlugin::pluginIsDisable()
{
    return !m_proxyInter->getValue(this, STATE_KEY, true).toBool();
}

const QString NetworkPlugin::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey)
    if (m_netStatus->needShowControlCenter()) {
        return QString("dbus-send --print-reply "
                       "--dest=com.deepin.dde.ControlCenter "
                       "/com/deepin/dde/ControlCenter "
                       "com.deepin.dde.ControlCenter.ShowModule "
                       "\"string:network\"");
    }

    return QString();
}

const QString NetworkPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey == NETWORK_KEY)
        return m_netStatus->contextMenu(true);

    return QString();
}

QWidget *NetworkPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == NETWORK_KEY) {
        if (m_trayIcon.isNull()) {
            m_trayIcon = m_netStatus->createDockIconWidget();
            positionChanged(position());
            updateIconColor();
            m_trayIcon->installEventFilter(this);
        }
        return m_trayIcon.data();
    }
    if (itemKey == "quick_item_key") { // QUICK_ITEM_KEY
        if (m_quickPanel.isNull()) {
            m_quickPanel = new QuickPanelWidget();
            m_netStatus->initQuickData();
            m_quickPanel->setActive(m_netStatus->networkActive());
            m_quickPanel->setText(m_netStatus->quickTitle());
            m_quickPanel->setDescription(m_netStatus->quickDescription());
            m_quickPanel->setIcon(m_netStatus->quickIcon());
            connect(m_quickPanel.data(), &QuickPanelWidget::iconClicked, this, &NetworkPlugin::onQuickIconClicked);
            connect(m_quickPanel.data(), &QuickPanelWidget::panelClicked, this, &NetworkPlugin::onQuickPanelClicked);
            connect(m_netStatus, &NetStatus::quickTitleChanged, m_quickPanel.data(), &QuickPanelWidget::setText);
            connect(m_netStatus, &NetStatus::quickDescriptionChanged, m_quickPanel.data(), &QuickPanelWidget::setDescription);
            connect(m_netStatus, &NetStatus::quickIconChanged, m_quickPanel.data(), &QuickPanelWidget::setIcon);
            connect(m_netStatus, &NetStatus::networkActiveChanged, m_quickPanel.data(), &QuickPanelWidget::setActive);
        }
        return m_quickPanel.data();
    }
    return Q_NULLPTR;
}

QWidget *NetworkPlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey == NETWORK_KEY && !m_netView->isVisible()) {
        if (m_tipsWidget.isNull())
            m_tipsWidget = m_netStatus->createDockItemTips();

        return m_tipsWidget.data();
    }

    return Q_NULLPTR;
}

QWidget *NetworkPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_dockContentWidget;
}

int NetworkPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

void NetworkPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

void NetworkPlugin::pluginSettingsChanged()
{
    refreshPluginItemsVisible();
}

void NetworkPlugin::loadPlugin()
{
    if (m_netStatus && m_netStatus->hasDevice()) {
        m_proxyInter->itemAdded(this, NETWORK_KEY);
    }
}

void NetworkPlugin::refreshPluginItemsVisible()
{
    if (pluginIsDisable() || !m_netStatus->hasDevice())
        m_proxyInter->itemRemoved(this, NETWORK_KEY);
    else
        m_proxyInter->itemAdded(this, NETWORK_KEY);
}

bool NetworkPlugin::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::ParentChange: {
        if (watched == m_trayIcon && m_trayIcon->parentWidget()) {
            m_trayIcon->parentWidget()->setMouseTracking(true);
            m_trayIcon->parentWidget()->installEventFilter(this);
        }
    } break;
    case QEvent::Resize: {
        if (m_trayIcon && watched == m_trayIcon->parentWidget())
            updateIconColor();
    } break;
    case QEvent::Enter:
    case QEvent::MouseMove: {
        const QPoint &p = m_trayIcon->mapFromGlobal(QCursor::pos());
        const bool isHorizontal = position() == Dock::Top || position() == Dock::Bottom;
        m_netStatus->setHoverTips((isHorizontal ? p.x() : p.y()) > ((isHorizontal ? m_trayIcon->width() : m_trayIcon->height()) / 2) && m_netStatus->vpnAndProxyIconVisibel()
                    ? NetStatus::HoverType::vpnAndProxy : NetStatus::HoverType::Network);
    } break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void NetworkPlugin::updateLockScreenStatus(bool visible)
{
    m_isLockScreen = visible;
    m_manager->setEnabled(!m_isLockScreen);
}

void NetworkPlugin::updateIconColor()
{
    if (m_trayIcon.isNull())
        return;

    Qt::GlobalColor color = Qt::white;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        color = Qt::black;
    }
    QPalette p = m_trayIcon->palette();
    if (p.brightText() != color) {
        p.setColor(QPalette::BrightText, color);
        m_trayIcon->setPalette(p);
    }
}

void NetworkPlugin::onNotify(uint replacesId)
{
    m_replacesId = replacesId;
}

void NetworkPlugin::onNetworkStatusChanged(NetStatus::NetworkStatus networkStatus)
{
    switch (networkStatus) {
    case NetStatus::NetworkStatus::ConnectNoInternet:
    case NetStatus::NetworkStatus::WirelessConnectNoInternet:
    case NetStatus::NetworkStatus::WiredConnectNoInternet:
    case NetStatus::NetworkStatus::WiredIpConflicted:
    case NetStatus::NetworkStatus::WirelessIpConflicted:
        m_netLimited = true;
        break;
    default:
        m_netLimited = false;
        break;
    }
    updateNetCheckVisible();
}

void NetworkPlugin::onNetCheckAvailableChanged(const bool &netCheckAvailable)
{
    if (m_netCheckAvailable != netCheckAvailable) {
        m_netCheckAvailable = netCheckAvailable;
        updateNetCheckVisible();
    }
}

void NetworkPlugin::updateNetCheckVisible()
{
    m_dockContentWidget->setNetCheckBtnVisible(m_netLimited && m_netCheckAvailable);
}


void NetworkPlugin::onQuickIconClicked()
{
    if (m_netStatus->needShowControlCenter()) {
        m_manager->gotoControlCenter();
    } else {
        m_netStatus->toggleNetworkActive();
    }
}

void NetworkPlugin::onQuickPanelClicked()
{
    if (m_netStatus->needShowControlCenter()) {
        m_manager->gotoControlCenter();
    } else {
        showNetworkDialog();
    }
}

void NetworkPlugin::showNetworkDialog()
{
    // 第一次连接隐藏网络会回弹，此时锁屏和任务栏都会收到信号
    if (m_isLockScreen || m_netView->isVisible())
        return;
    m_proxyInter->requestSetAppletVisible(this, NETWORK_KEY, true);
}

#ifdef USE_NEW_DOCK_API

QString NetworkPlugin::message(const QString &msg)
{
    QJsonParseError jsonParseError;
    const QJsonDocument &resultDoc = QJsonDocument::fromJson(msg.toLocal8Bit(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || resultDoc.isEmpty()) {
        qCWarning(DNC) << "Result json parse error";
        return "{}";
    }

    const auto &msgObj = resultDoc.object();
    if (msgObj.value(Dock::MSG_TYPE).toString() == Dock::MSG_SET_APPLET_MIN_HEIGHT) {
        const int minHeight = msgObj.value(Dock::MSG_DATA).toInt(-1);
        if (m_dockContentWidget && minHeight > 0)
            m_dockContentWidget->setMinHeight(minHeight);
    }

    if (msgObj.value(Dock::MSG_TYPE).toString() == Dock::MSG_APPLET_CONTAINER && m_dockContentWidget) {
        m_dockContentWidget->setMainLayoutMargins(QMargins(0, msgObj.value(Dock::MSG_DATA).toInt(-1) ==
            Dock::APPLET_CONTAINER_QUICK_PANEL ? 6 : 10, 0, 0));
    }

    return "{}";
}

#endif //USE_NEW_DOCK_API

} // namespace network
} // namespace dde
