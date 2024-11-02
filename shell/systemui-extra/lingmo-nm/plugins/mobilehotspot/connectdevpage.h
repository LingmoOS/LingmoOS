/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef CONNECTDEVPAGE_H
#define CONNECTDEVPAGE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>

#include "titlelabel.h"
#include "connectdevlistitem.h"

using namespace kdk;

class ConnectdevPage : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectdevPage(QWidget *parent = nullptr);

    void refreshStalist();

    inline void setInterface(QDBusInterface *activeInterface) {
            m_activePathInterface = activeInterface;
    }

private:
    QFrame* myLine();

    void getConnectStaDevice(QMap<QString, QString> &blacklistMap);
    bool removeStaFromBlacklist(QString staMac);
    void initStaDev();
    void addStaDevFrame(QString staMac, QString staName);
    void clearStaListLayout();
    QString getActivePathByUuid(QDBusInterface *interface);
    void initNmDbus(QDBusInterface *interface);

    void onStaDevChanged(bool istrue, QString staMac, QString staName);
    void resetLayoutHight();

private:
    QFrame *m_staistFrame = nullptr;

    TitleLabel *m_titleLabel = nullptr;
    QVBoxLayout *m_staListLayout = nullptr;
    QMap<QString, QString> m_staMap;

    QDBusInterface  *m_activePathInterface = nullptr;

signals:
    void setStaIntoBlacklist(QString staMac, QString staName);

public slots:
    void onStaDevAdded(bool istrue, QString staMac, QString staName);
    void onStaDevRemoved(bool istrue, QString staMac, QString staName);

private slots:
    void onDropIntoBlacklistBtnClicked(QString staMac, QString staName);
};

#endif // MOBILEHOTSPOTWIDGET_H
