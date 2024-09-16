// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "panel.h"
#include <QQuickItem>

namespace osd {

class OsdPanel : public DS_NAMESPACE::DPanel
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(QString osdType READ osdType NOTIFY osdTypeChanged FINAL)
    Q_CLASSINFO("D-Bus Interface", "org.deepin.osdService")
public:
    explicit OsdPanel(QObject *parent = nullptr);

    virtual bool load() override;
    virtual bool init() override;

    bool visible() const;
    QString osdType() const;

public Q_SLOTS:
    void showText(const QString &text);

Q_SIGNALS:
    void visibleChanged();
    void osdTypeChanged(QString osdType);

private Q_SLOTS:
    void hideOsd();
private:
    void showOsd();
    void setVisible(const bool visible);
    void setOsdType(const QString &osdType);

private:
    bool m_visible = false;
    QTimer *m_osdTimer = nullptr;
    QString m_osdType;
    int m_interval {2000};
};

};
