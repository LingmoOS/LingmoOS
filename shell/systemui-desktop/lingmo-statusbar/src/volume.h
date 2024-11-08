/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VOLUMEMANAGER_H
#define VOLUMEMANAGER_H

#include <QObject>

class VolumeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY validChanged)
    Q_PROPERTY(bool isMute READ isMute NOTIFY muteChanged)
    Q_PROPERTY(int volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY volumeChanged)

public:
    static VolumeManager *self();

    explicit VolumeManager(QObject *parent = nullptr);

    bool isValid() const;
    bool isMute() const;
    int volume() const;

    QString iconName() const;

    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setMute(bool mute);
    Q_INVOKABLE void setVolume(int value);

signals:
    void validChanged();
    void muteChanged();
    void volumeChanged();

private:
    void init();
    void initStatus();
    void connectDBusSignals();

private slots:
    void onDBusVolumeChanged(int volume);
    void onDBusMuteChanged(bool mute);

private:
    bool m_isValid;
    bool m_isMute;
    int m_volume;
};

#endif // VOLUMEMANAGER_H
