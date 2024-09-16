// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETWORKDETAILINTERREALIZE_H
#define NETWORKDETAILINTERREALIZE_H

#include "netinterface.h"

class QJsonObject;

namespace dde {
namespace network {

class NetworkDetailInterRealize : public NetworkDetailRealize
{
    Q_OBJECT

private:
    enum InfoType {
        Ip,
        Gateway
    };

public:
    explicit NetworkDetailInterRealize(QObject *parent = Q_NULLPTR);

    QString name() override;
    QList<QPair<QString, QString>> items() override;

    void updateData(const QJsonObject &info);

protected:
    ~NetworkDetailInterRealize() override;

    void appendInfo(const QString &title, const QString &value);
    QString ipv6Information(const QJsonObject &connectInfo, InfoType type);
    QString devicePath();

    QString prefixToNetMask(int prefixLength);

private:
    QString m_name;
    QString m_devicePath;
    QList<QPair<QString, QString>> m_items;
};

}
}

#endif // NETWORKDETAILINTERREALIZE_H
