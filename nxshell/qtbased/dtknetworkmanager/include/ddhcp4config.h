// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDHCP4CONFIG_H
#define DDHCP4CONFIG_H

#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DDHCP4ConfigPrivate;

class DDHCP4Config : public QObject
{
    Q_OBJECT
public:
    explicit DDHCP4Config(const quint64 id, QObject *parent = nullptr);
    ~DDHCP4Config() override;
    Q_PROPERTY(Config options READ options NOTIFY optionsChanged)

    Config options() const;

Q_SIGNALS:
    void optionsChanged(const Config &option);

private:
    QScopedPointer<DDHCP4ConfigPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DDHCP4Config)
};
DNETWORKMANAGER_END_NAMESPACE

#endif
