// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NETHOTSPOTCONTROLLER_H
#define NETHOTSPOTCONTROLLER_H

#include <QObject>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace ocean {
namespace network {
class HotspotController;
class WirelessDevice;

class NetHotspotController : public QObject
{
    Q_OBJECT
public:
    explicit NetHotspotController(QObject *parent = nullptr);
    bool isEnabled() const;
    bool enabledable() const;
    const QVariantMap &config() const;
    const QStringList &optionalDevice() const;
    const QStringList &shareDevice() const;

Q_SIGNALS:
    void enabledChanged(const bool);
    void enabledableChanged(const bool);
    void configChanged(const QVariantMap &config);
    void optionalDeviceChanged(const QStringList &optionalDevice);
    void shareDeviceChanged(const QStringList &shareDevice);

private Q_SLOTS:
    void updateEnabled();
    void updateEnabledable();
    void updateData();
    void updateConfig();

private:
    HotspotController *m_hotspotController;
    bool m_isEnabled;
    bool m_enabledable;
    QVariantMap m_config;
    QStringList m_shareDevice;
    QStringList m_optionalDevice;
    QTimer *m_updatedTimer;
};
} // namespace network
} // namespace ocean
#endif // NETHOTSPOTCONTROLLER_H
