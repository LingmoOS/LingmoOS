// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSPOINTSINTER_H
#define ACCESSPOINTSINTER_H

#include "netinterface.h"

namespace dde {
namespace network {

class AccessPointsProxyInter : public AccessPointProxy
{
    Q_OBJECT

public:
    AccessPointsProxyInter(const QJsonObject &json, const QString &devicePath, QObject *parent = Q_NULLPTR);
    ~AccessPointsProxyInter() override;

    void updateAccessPoints(const QJsonObject &json);
    void updateConnectionStatus(ConnectionStatus status);

public:
    QString ssid() const override;
    int strength() const override;
    bool secured() const override;
    bool securedInEap() const override;
    int frequency() const override;
    QString path() const override;
    QString devicePath() const override;
    bool connected() const override;
    ConnectionStatus status() const override;
    bool hidden() const override;
    bool isWlan6() const override;

private:
    QString m_devicePath;
    QJsonObject m_json;
    ConnectionStatus m_status;
};

}
}

#endif // ACCESSPOINTSINTER_H
