// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "appletproxy.h"

#include <QObject>
#include <DObject>

DS_BEGIN_NAMESPACE
/**
 * @brief Interacting with other applets
 */
class DAppletBridgePrivate;
class DS_SHARE DAppletBridge : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DAppletBridge)
public:
    explicit DAppletBridge(const QString &pluginId, QObject *parent = nullptr);
    virtual ~DAppletBridge() override;

    bool isValid() const;
    QString pluginId() const;

    QList<DAppletProxy *> applets() const;
    DAppletProxy *applet() const;

protected:
    explicit DAppletBridge(DAppletBridgePrivate &dd, QObject *parent = nullptr);
};

DS_END_NAMESPACE
