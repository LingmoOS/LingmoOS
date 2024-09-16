// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

namespace osd {

class BrightnessApplet : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged FINAL)
    Q_PROPERTY(double brightness READ brightness NOTIFY brightnessChanged FINAL)
public:
    explicit BrightnessApplet(QObject *parent = nullptr);

    double brightness() const;
    QString iconName() const;

    Q_INVOKABLE void sync();

Q_SIGNALS:
    void brightnessChanged();
    void iconNameChanged();

private:
    void setIconName(const QString &newIconName);
    void setBrightness(double newBrightness);

    QString fetchIconName() const;
    double fetchBrightness() const;
private:
    double m_brightness;
    QString m_iconName;
};

}
