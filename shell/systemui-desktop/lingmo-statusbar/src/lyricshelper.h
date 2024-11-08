/*
 * Copyright (C) 2024 - 2022 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#ifndef LYRICSHELPER_H
#define LYRICSHELPER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>

class LyricsHelper : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.Statusbar")
    Q_PROPERTY(QString lyrics READ getLyrics NOTIFY lyricsChanged)
    Q_PROPERTY(bool lyricsVisible READ getLyricsVisible NOTIFY lyricsVisibleChanged)

public:
    explicit LyricsHelper(QObject *parent = nullptr);
    QString getLyrics();
    bool getLyricsVisible();

public slots:
    void sendLyrics(QString text);

private slots:
    void onLyricsVisibleChanged();

private:
    QTimer *Timer=new QTimer;
    QString m_Lyrics;
    bool m_LyricsVisible;

signals:
    void lyricsChanged();
    void lyricsVisibleChanged();

};

#endif // LYRICSHELPER_H
