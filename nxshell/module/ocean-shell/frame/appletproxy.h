// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>
#include <DObject>

DS_BEGIN_NAMESPACE
/**
 * @brief Expose own interfaces for other applets.
 */
class DAppletProxyPrivate;
class DS_SHARE DAppletProxy : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    D_DECLARE_PRIVATE(DAppletProxy)
public:
    ~DAppletProxy() override;

protected:
    explicit DAppletProxy(QObject *parent = nullptr);
    explicit DAppletProxy(DAppletProxyPrivate &dd, QObject *parent = nullptr);
};

DS_END_NAMESPACE
