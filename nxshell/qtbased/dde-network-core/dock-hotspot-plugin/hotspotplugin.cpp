// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotspotplugin.h"
#include <QScreen>
#include <DGuiApplicationHelper>
#include <QVBoxLayout>
#include <DDBusSender>
#include <networkmanagerqt/accesspoint.h>
#include <networkmanagerqt/settings.h>
#include <networkmanagerqt/device.h>
#include <algorithm>
#include "networkcontroller.h"
#include "hotspotcontroller.h"
HOTSPOTPLUGIN_BEGIN_NAMESPACE

DWIDGET_USE_NAMESPACE
static constexpr auto state_key = "enabled";

HotspotPlugin::HotspotPlugin(QObject *parent)
    : QObject(parent)
{
  QTranslator *trs = new QTranslator(this);
  trs->load(QString("/usr/share/dock-hotspot-plugin/translations/dock-hotspot-plugin_%1.qm").arg(QLocale::system().name()));
  QCoreApplication::installTranslator(trs);
}

void HotspotPlugin::init(PluginProxyInterface *proxyInter) {
  m_proxyInter = proxyInter;
  m_tipsLabel.reset(new networkplugin::TipsWidget());
  m_quickPanel.reset(new QuickPanel());

  onStateChanged(State::Off);
  for (const auto &dev : NetworkManager::networkInterfaces()) {
    if (dev->type() == NetworkManager::Device::Wifi)
      m_wirelessDev.append(dev);
  }
  initConnection();
  updateLatestHotSpot();
  for (const auto &dev : m_wirelessDev)
    updateState(dev);

  onHotspotEnabledChanged();
  auto controller = network::NetworkController::instance()->hotspotController();
  connect(controller, &network::HotspotController::enabledChanged,
            this, &HotspotPlugin::onHotspotEnabledChanged, Qt::UniqueConnection);
}

void HotspotPlugin::initConnection() {
  connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::themeTypeChanged,this,[this](){
      m_quickPanel->updateState(DGuiApplicationHelper::instance()->themeType(),hotspotEnabled);
  });

  connect(m_quickPanel.data(),&QuickPanel::iconClicked,this,&HotspotPlugin::onQuickPanelClicked);

  connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this,
          [this](const QString &uni) {
            const auto dev = NetworkManager::Device::Ptr{new NetworkManager::Device{uni}};
            if (dev->type() == NetworkManager::Device::Wifi){
              m_wirelessDev.append(dev);
              initDevConnection(dev);
            }
          });

  connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this,
          [this](const QString &uni) {
            const auto devices = m_wirelessDev;
            for (const auto &dev : devices) {
              if (dev->uni() == uni) {
                m_wirelessDev.removeOne(dev);
                if(!m_latestDevice.isNull() and (dev->uni() == m_latestDevice->uni())){
                  m_latestDevice.reset(nullptr);
                  m_latestHotSpot.reset(nullptr);
                  updateLatestHotSpot();
                  if (m_latestHotSpot) {
                    auto reply = NetworkManager::activateConnection(
                        m_latestHotSpot->path(), m_latestDevice->uni(), "/");
                    reply.waitForFinished();  // force sync
                    if (reply.isError())
                      qWarning() << "activate failed:" << reply.error();
                  } else {
                    onStateChanged(State::Off);
                  }
                }
              }
            }
          });
  connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionRemoved,this,
          [this](const QString& path) {
            if(m_latestHotSpot.isNull()){
              return;
            }
            qInfo() << "connection was removed:" << path;  
            if(path == m_latestHotSpot->path())
              updateLatestHotSpot();
          });

  for (const auto &dev : m_wirelessDev)
    initDevConnection(dev);
}

void HotspotPlugin::initDevConnection(const NetworkManager::Device::Ptr &dev) {
    connect(dev.data(), &NetworkManager::Device::activeConnectionChanged, this,
        [this, dev]() {
            updateState(dev);
          },Qt::QueuedConnection);
#ifdef USE_DEEPIN_NMQT
  connect(dev.data(), &NetworkManager::Device::interfaceFlagsChanged, this,
          [this, dev]() {
            if (m_latestDevice.isNull() or dev->uni() != m_latestDevice->uni()){
              bool allDisable{false};
              for (auto dev : m_wirelessDev)
                allDisable |= checkDeviceAvailability(dev);
              if (allDisable)
                onStateChanged(State::Off);
              else
                onStateChanged(State::Unavailable);
              return;
            }

            auto curCon = m_latestDevice->activeConnection();
            if(curCon.isNull() or curCon->state() != NetworkManager::ActiveConnection::State::Activated)
              return;
              
            if (checkDeviceAvailability(dev)){
              onStateChanged(State::Off);
            } else {
              onStateChanged(State::Unavailable);
            }

            if (curCon->connection()->path() == m_latestHotSpot->path()) {
              auto reply = NetworkManager::deactivateConnection(curCon->path());
              reply.waitForFinished();  // force sync
              if (reply.isError())
                qWarning() << reply.error() << curCon->path();
            }
      },Qt::QueuedConnection);
#endif
}

HotspotPlugin::~HotspotPlugin() {
  if(m_proxyInter)
    m_proxyInter->itemRemoved(this, hotspot_key);
}

void HotspotPlugin::updateState(const NetworkManager::Device::Ptr &dev) {
  auto con = dev->activeConnection();
  State enabled{State::Off};
  if (!checkDeviceAvailability(dev)) {
    qInfo() << "device is Unavailable" << dev->uni();
    enabled = State::Unavailable;
  } else {
    if (!con.isNull()) {
      auto connectionPtr = con->connection();
      const auto &settingMap = connectionPtr->settings()->toMap();
      const auto &wirelessMap = qAsConst(settingMap).find("802-11-wireless");
      if (wirelessMap != qAsConst(settingMap).cend()) {
        const auto &mode = (*wirelessMap).find("mode");
        if (mode != qAsConst(*wirelessMap).cend()) {
          const auto &modeStr = (*mode).toString();
          if (modeStr == "ap" or modeStr == "adhoc")
            enabled = State::On;
        }
      }
    }
  }

  if (enabled == State::On) {
    m_latestDevice = dev;
    m_latestHotSpot = con->connection();
  } else {
    if (!m_latestDevice.isNull() and dev->uni() != m_latestDevice->uni())
      return;
  }

  qInfo() << "hotspot update state to:" << static_cast<int>(enabled);

  onStateChanged(enabled);
}

bool HotspotPlugin::checkDeviceAvailability(const NetworkManager::Device::Ptr& dev) const
{
#ifdef USE_DEEPIN_NMQT
  return !!dev->interfaceFlags();
#else
  return false;
#endif
}

const QString HotspotPlugin::pluginName() const
{
    return "hotspot";
}

const QString HotspotPlugin::pluginDisplayName() const
{
    return tr("hotspot");
}

bool HotspotPlugin::pluginIsDisable() {
  return !m_proxyInter->getValue(this, state_key, true).toBool();
}

void HotspotPlugin::pluginStateSwitched() {
    const bool disabled = pluginIsDisable();
    m_proxyInter->saveValue(this, state_key, disabled);
    updateDockItemEnabled(!disabled);
}

const QString HotspotPlugin::itemCommand(const QString &itemKey) {
  Q_UNUSED(itemKey)
  if(m_wirelessDev.isEmpty()){
      qWarning() << "hotspot is unsupported. ignore...";
      return {};
  }
  if (hotspotEnabled) {
    auto curCon = m_latestDevice->activeConnection();
    auto reply = NetworkManager::deactivateConnection(curCon->path());
    reply.waitForFinished(); // force sync
    if(reply.isError())
      qWarning() << reply.error() << m_latestHotSpot->path();
    return {};
  } else {
    if (!m_latestDevice.isNull() and !m_latestHotSpot.isNull()) {
      if (!checkDeviceAvailability(m_latestDevice))
        return {};
      auto reply = NetworkManager::activateConnection(m_latestHotSpot->path(),m_latestDevice->uni(),"/");
      reply.waitForFinished(); // force sync
      if(reply.isError())
        qWarning() << "activate failed:" << reply.error();
      return {};
    }
  }
  return QString("dbus-send --print-reply --dest=org.deepin.dde.ControlCenter1 /org/deepin/dde/ControlCenter1 org.deepin.dde.ControlCenter1.ShowPage string:network/personalHotspot");
}

QWidget *HotspotPlugin::itemWidget(const QString &itemKey) {
    if(itemKey == QUICK_ITEM_KEY)
        return m_quickPanel.data();
    return nullptr;
}

QWidget *HotspotPlugin::itemTipsWidget(const QString &itemKey) {
  if (itemKey == hotspot_key)
    return m_tipsLabel.data();
  return nullptr;
}

PluginFlags HotspotPlugin::flags() const {
  return PluginFlag::Type_Common | PluginFlag::Attribute_CanDrag |
         PluginFlag::Attribute_CanInsert | PluginFlag::Attribute_CanSetting |
         PluginFlag::Quick_Single;
}

QPixmap HotspotPlugin::getIcon(const DGuiApplicationHelper::ColorType type,const QSize &size) const{
    QString iconStr = "network-hotspot";
    if (type == DGuiApplicationHelper::ColorType::LightType)
      iconStr = iconStr + "-dark";
    const auto screen = qApp->primaryScreen();
    QPixmap pix = QIcon::fromTheme(iconStr).pixmap(size);
    pix.setDevicePixelRatio(screen->devicePixelRatio());
    return pix;
}

QIcon HotspotPlugin::icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType) {
  if(dockPart == DockPart::QuickPanel)
      return QIcon{};
  auto pix = getIcon(themeType,QSize(16,16));
  return QIcon{pix};
}

int HotspotPlugin::itemSortKey(const QString &itemKey) {
  const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
  return m_proxyInter->getValue(this, key, 5).toInt();
}

void HotspotPlugin::setSortKey(const QString &itemKey, const int order) {
  const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
  m_proxyInter->saveValue(this, key, order);
}

void HotspotPlugin::onStateChanged(State state) {

  QString text;

  if (m_wirelessDev.isEmpty())
    state = State::Unsupported;

  switch (state) {
    case State::Unavailable:
      text = tr("Wireless Device is Unavailable");
      break;
    case State::On:
      text = tr("Personal Hotspot On");
      break;
    case State::Off:
      text = tr("Personal Hotspot Off");
      break;
    case State::Unsupported:
      text = tr("Hotspot is unsupported");
      break;
    default:
      qWarning() << "Internal error";
  }

  hotspotEnabled = state == State::On;
  m_quickPanel->updateState(DGuiApplicationHelper::instance()->themeType(), hotspotEnabled);
  m_quickPanel->setToolTip(text);
  m_tipsLabel->setContext({{text, {}}});
}

void HotspotPlugin::onHotspotEnabledChanged()
{
    updateDockItemEnabled(network::NetworkController::instance()->hotspotController()->supportHotspot());
}

void HotspotPlugin::updateDockItemEnabled(bool isEnabled)
{
    if (isEnabled)
        m_proxyInter->itemAdded(this, hotspot_key);
    else
        m_proxyInter->itemRemoved(this, hotspot_key);
}

void HotspotPlugin::updateLatestHotSpot(){
    uint maxTimeStamp{0};
    for(const auto& dev : m_wirelessDev){
        const auto& connections = dev->availableConnections();
        //mayby we can use async method to reduce update time
        for(const auto& elem : connections){
            const auto& elemSettings = elem->settings()->toMap();
            const auto& mode = elemSettings["802-11-wireless"]["mode"];
            if(mode == "infrastructure")
                continue;
            const auto elemTimeStamp = elemSettings["connection"]["timestamp"].toUInt();
            if(elemTimeStamp >= maxTimeStamp){
              maxTimeStamp = elemTimeStamp;
              m_latestDevice = dev;
              m_latestHotSpot = elem;
            }
        }
    }

    if(maxTimeStamp == 0){
      m_latestDevice.reset(nullptr);
      m_latestHotSpot.reset(nullptr);
      qInfo() << "now cache is empty";
      onStateChanged(State::Off);
    } else {
      if (!checkDeviceAvailability(m_latestDevice))
        onStateChanged(State::Unavailable);
    }
}

void HotspotPlugin::onQuickPanelClicked(){
    if (hotspotEnabled) {
        auto curCon = m_latestDevice->activeConnection();
        auto reply = NetworkManager::deactivateConnection(curCon->path());
        reply.waitForFinished(); // force sync
        if(reply.isError())
            qWarning() << reply.error() << m_latestHotSpot->path();
    } else {
        if(m_wirelessDev.isEmpty()){
            qWarning() << "there is no wireless device to enable hotspot.";
            return;
        }
        if(!m_latestDevice.isNull() and !m_latestHotSpot.isNull()){
            if(!checkDeviceAvailability(m_latestDevice))
              return;
            auto reply = NetworkManager::activateConnection(m_latestHotSpot->path(),m_latestDevice->uni(),"/");
            reply.waitForFinished(); // force sync
            if(reply.isError())
                qWarning() << "activate failed:" << reply.error();
            return;
        }
        qInfo() << "no hotspot connection exists";
        DDBusSender()
            .service("org.deepin.dde.ControlCenter1")
            .interface("org.deepin.dde.ControlCenter1")
            .path("/org/deepin/dde/ControlCenter1")
            .method(QString("ShowPage"))
            .arg(QString("network/personalHotspot"))
            .call();
        return;
    }
}

HOTSPOTPLUGIN_END_NAMESPACE
