// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

#include "dbusmediaplayer2.h"

#include <QWidget>
#include <dimagebutton.h>
#include <dmpriscontrol.h>

DWIDGET_USE_NAMESPACE

class MediaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MediaWidget(QWidget *parent = nullptr);
    void initMediaPlayer();

private slots:
    void changeVisible();

private:
    void initUI();
    void initConnect();

private:
    DMPRISControl *m_dmprisWidget;
    DBusMediaPlayer2 *m_dbusInter;
};

#endif // MEDIAWIDGET_H
