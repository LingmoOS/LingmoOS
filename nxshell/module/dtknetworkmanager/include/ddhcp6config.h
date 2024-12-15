// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDHCP6CONFIG_H
#define DDHCP6CONFIG_H

#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DDHCP6ConfigPrivate;

class DDHCP6Config : public QObject
{
    Q_OBJECT
public:
    explicit DDHCP6Config(const quint64 id, QObject *parent = nullptr);
    ~DDHCP6Config() override;
    Q_PROPERTY(Config options READ options NOTIFY optionsChanged)

    Config options() const;

Q_SIGNALS:
    void optionsChanged(const Config &option);

private:
    QScopedPointer<DDHCP6ConfigPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DDHCP6Config)
};
DNETWORKMANAGER_END_NAMESPACE

#endif
