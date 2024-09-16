// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <DObject>
#include <QObject>

DS_BEGIN_NAMESPACE

class DApplet;
class DAppletLoaderPrivate;
class DAppletLoader : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DAppletLoader)
public:
    explicit DAppletLoader(DApplet *applet, QObject *parent = nullptr);
    virtual ~DAppletLoader() override;

    void exec();
    DApplet *applet() const;

Q_SIGNALS:
    void failed(const QString &pluginId);
};

DS_END_NAMESPACE
