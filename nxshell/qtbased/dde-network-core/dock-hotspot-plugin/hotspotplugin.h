// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTSPOTPLUGIN_H
#define HOTSPOTPLUGIN_H

#include "hotspotplugin_utils.h"
#include "pluginsiteminterface.h"
#include "widgets/tipswidget.h"
#include "quickpanel.h"
#include <networkmanagerqt/manager.h>

HOTSPOTPLUGIN_BEGIN_NAMESPACE

static constexpr auto hotspot_key = "hotspot";

class HotspotPlugin : public QObject, PluginsItemInterface {
  Q_OBJECT
  Q_INTERFACES(PluginsItemInterface)
  Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE
                        "hotspot.json")

 public:
  enum class State {
    On,Off,Unavailable,Unsupported
  };
  explicit HotspotPlugin(QObject *parent = nullptr);
  ~HotspotPlugin() override;

  const QString pluginName() const override;
  const QString pluginDisplayName() const override;
  void init(PluginProxyInterface *proxyInter) override;
  void pluginStateSwitched() override;
  bool pluginIsAllowDisable() override { return true; }
  bool pluginIsDisable() override;
  const QString itemCommand(const QString &itemKey) override;
  QWidget *itemWidget(const QString &itemKey) override;
  QWidget *itemTipsWidget(const QString &itemKey) override;
  QIcon icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType) override;
  int itemSortKey(const QString &itemKey) override;
  void setSortKey(const QString &itemKey, const int order) override;
  PluginFlags flags() const override;

  private Q_SLOTS:
   void onStateChanged(State state);

  private:
   void initConnection();
   bool checkDeviceAvailability(const NetworkManager::Device::Ptr &dev) const;
   void initDevConnection(const NetworkManager::Device::Ptr &dev);
   QPixmap getIcon(const DGuiApplicationHelper::ColorType type, const QSize& size) const;
   void updateState(const NetworkManager::Device::Ptr& dev);
   void onQuickPanelClicked();
   void updateLatestHotSpot();
    void onHotspotEnabledChanged();
    void updateDockItemEnabled(bool isEnabled);

   bool hotspotEnabled{false};
   QScopedPointer<networkplugin::TipsWidget> m_tipsLabel{nullptr};
   NetworkManager::Device::List m_wirelessDev{};
   QScopedPointer<QuickPanel> m_quickPanel{nullptr};
   NetworkManager::Connection::Ptr m_latestHotSpot{nullptr};
   NetworkManager::Device::Ptr m_latestDevice{nullptr};
};

HOTSPOTPLUGIN_END_NAMESPACE

#endif
