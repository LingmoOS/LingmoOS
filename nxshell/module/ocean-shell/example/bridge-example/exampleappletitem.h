// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIdentityProxyModel>

#include <applet.h>
#include <dsglobal.h>

class ExampleAppletItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QIdentityProxyModel * applicationsModel MEMBER m_appsModel NOTIFY applicationsModelChanged FINAL)
public:
    explicit ExampleAppletItem(QObject *parent = nullptr);
    virtual bool init() override;

signals:
    void applicationsModelChanged();

private:
    QIdentityProxyModel * m_appsModel;
};
