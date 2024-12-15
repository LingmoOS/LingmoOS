// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDHCP4CONFIGINTERFACE_H
#define DDHCP4CONFIGINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DDHCP4ConfigInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDHCP4ConfigInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DDHCP4ConfigInterface() override = default;

    Q_PROPERTY(Config options READ options NOTIFY OptionsChanged)

    Config options() const;

Q_SIGNALS:
    void OptionsChanged(const Config &option);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE
#endif
