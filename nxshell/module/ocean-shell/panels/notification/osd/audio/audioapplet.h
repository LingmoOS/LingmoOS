// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

namespace osd {

class AudioApplet : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged FINAL)
    Q_PROPERTY(double volumeValue READ volumeValue NOTIFY volumeValueChanged FINAL)
    Q_PROPERTY(bool increaseVolume READ increaseVolume WRITE setIncreaseVolume NOTIFY increaseVolumeChanged FINAL)
public:
    explicit AudioApplet(QObject *parent = nullptr);

    double volumeValue() const;
    void setVolumeValue(double newVolumeValue);

    QString iconName() const;
    void setIconName(const QString &newIconName);

    double volumeRatio() const;
    void setVolumeRatio(double newVolumeRatio);

    bool increaseVolume() const;
    void setIncreaseVolume(bool newIncreaseVolume);

    Q_INVOKABLE void sync();

Q_SIGNALS:
    void volumeValueChanged();
    void iconNameChanged();
    void volumeRatioChanged();
    void increaseVolumeChanged();

private:
    QString fetchIconName() const;
    double fetchVolume() const;
    bool fetchIncreaseVolume() const;
private:
    double m_increaseVolume = 1.0;
    double m_volumeValue = 1.0;
    QString m_iconName;
    double m_volumeRatio;
};

}
