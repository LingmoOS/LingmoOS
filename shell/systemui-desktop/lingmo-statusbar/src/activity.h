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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QObject>
#include "capplications.h"

class Activity : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool launchPad READ launchPad NOTIFY launchPadChanged)

public:
    explicit Activity(QObject *parent = nullptr);

    bool launchPad() const;

    QString title() const;
    QString icon() const;

    Q_INVOKABLE void close();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void maximize();
    Q_INVOKABLE void toggleMaximize();
    Q_INVOKABLE void move();

    bool isAcceptableWindow(quint64 wid);

private slots:
    void onActiveWindowChanged();

    void clearTitle();
    void clearIcon();

signals:
    void titleChanged();
    void iconChanged();
    void launchPadChanged();

private:
    CApplications *m_cApps;
    QString m_title;
    QString m_icon;
    QString m_windowClass;
    quint32 m_pid;

    bool m_launchPad;
};

#endif // ACTIVITY_H
