// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

#include <dobject_p.h>
#include <QVariant>
#include <QPointer>

DS_BEGIN_NAMESPACE
/**
 * @brief 插件项，单个插件实例
 */
class DAppletPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DAppletPrivate(DApplet *qq);
    ~DAppletPrivate() override;

    DPluginMetaData m_metaData;
    DAppletData m_data;
    QPointer<QObject> m_rootObject;

    D_DECLARE_PUBLIC(DApplet);
};

DS_END_NAMESPACE
