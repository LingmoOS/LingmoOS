// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appletbridge.h"
#include "applet.h"

#include <dobject_p.h>
#include <QObject>

DS_BEGIN_NAMESPACE
/**
 * @brief
 */
class DAppletBridgePrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DAppletBridgePrivate(DAppletBridge *qq);
    ~DAppletBridgePrivate() override;

    QList<DApplet *> applets() const;

    QString m_pluginId;

    D_DECLARE_PUBLIC(DAppletBridge);
};

DS_END_NAMESPACE
