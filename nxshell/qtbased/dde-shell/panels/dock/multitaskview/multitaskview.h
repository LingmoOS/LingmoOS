// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"
#include "../dockiteminfo.h"

namespace dock {

class MultiTaskView : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged FINAL)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit MultiTaskView(QObject *parent = nullptr);
    virtual bool init() override;

    QString iconName() const;
    void setIconName(const QString& iconName);
    bool hasComposite();

    Q_INVOKABLE void openWorkspace();
    Q_INVOKABLE DockItemInfo dockItemInfo();

    Q_INVOKABLE bool visible() const;
    Q_INVOKABLE void setVisible(bool visible);

Q_SIGNALS:
    void iconNameChanged();
    void visibleChanged();

private:
    bool m_visible;
    QString m_iconName;
};

}
