// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "qqmlintegration.h"

DS_USE_NAMESPACE

class ExampleApplet : public DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString mainText READ mainText NOTIFY mainTextChanged)
    Q_PROPERTY(bool userData READ userData WRITE setUserData NOTIFY userDataChanged)
public:
    explicit ExampleApplet(QObject *parent = nullptr);

    QString mainText() const;

    virtual bool load() override;
    virtual bool init() override;

    bool userData() const;
    void setUserData(bool newUserData);

Q_SIGNALS:
    void mainTextChanged();
    void userDataChanged();
private:
    QString m_mainText;
    bool m_userData;
};
