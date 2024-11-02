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
#ifndef BLACKLISTITEM_H
#define BLACKLISTITEM_H
#include <QFrame>
#include <QEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QLabel>
#include "kborderlessbutton.h"

using namespace kdk;

class BlacklistItem : public QFrame
{
    Q_OBJECT
public:
    BlacklistItem(QString staMac, QString staName, QWidget *parent = nullptr);

protected:
    KBorderlessButton *m_removeFromBlacklistBtn = nullptr;

    QString m_mac;
    QString m_hostName;
    bool eventFilter(QObject *w, QEvent *e);

signals:
    void onBtnClicked(QString staMac, QString staName);


};

#endif // LISTITEM_H
