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
#ifndef LISTITEM_H
#define LISTITEM_H
#include <QFrame>
#include <QEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include "radioitembutton.h"
#include "infobutton.h"
#include "netdetails/netdetail.h"

typedef enum{
    UnknownState = 0, /**< The active connection is in an unknown state */
    Activating, /**< The connection is activating */
    Activated, /**< The connection is activated */
    Deactivating, /**< The connection is being torn down and cleaned up */
    Deactivated /**< The connection is no longer active */
}ConnectState;

class FreqLabel : public QLabel
{
    Q_OBJECT
public:
    FreqLabel(QWidget *parent = nullptr);
    ~FreqLabel() = default;
protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void changedFontSlot();
};

class FixPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FixPushButton(QWidget *parent = 0);

public:
    void setButtonText(QString text);
    QString getText();

private Q_SLOTS:
    void changedLabelSlot();
private:
    QString mStr;

};

class NameLabel : public QLabel
{
    Q_OBJECT
public:
    explicit NameLabel(QWidget *parent = 0);
    ~NameLabel() = default;
    void setLabelText(QString text);
    void setLabelMaximumWidth(int width);

private:
    QString m_name;
    int m_maximumWidth = 120;

private Q_SLOTS:
    void changedLabelSlot();

};

class ListItem : public QFrame
{
    Q_OBJECT
public:
    ListItem(QWidget *parent = nullptr);
    ~ListItem();
    void setName(const QString &name);
    void setActive(const bool &isActive);
    void setConnectState(ConnectState state);
    static void showDesktopNotify(const QString &message, QString soundName);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
    virtual void onRightButtonClicked() = 0;

protected:
    QFrame * m_itemFrame = nullptr;

    NameLabel * m_nameLabel = nullptr;
    RadioItemButton * m_netButton = nullptr;
//    InfoButton * m_infoButton = nullptr;

    bool m_isActive = false;
    ConnectState m_connectState;

    QMenu *m_menu = nullptr;
public:
    QVBoxLayout * m_mainLayout = nullptr;
    QHBoxLayout * m_hItemLayout = nullptr;

    QLabel *m_lbLoadDown = nullptr;
    QLabel *m_lbLoadUp = nullptr;
    QLabel *m_lbLoadDownImg = nullptr;
    QLabel *m_lbLoadUpImg = nullptr;
    QLabel *m_freq = nullptr;

    NetDetail *netDetail = nullptr;

    FixPushButton *m_hoverButton = nullptr;
private:
    void initUI();
    void initConnection();

public Q_SLOTS:
    virtual void onNetButtonClicked() = 0;
    void onPaletteChanged();
    virtual void onMenuTriggered(QAction *action)=0;

Q_SIGNALS:
    void detailShow(QString, QString);
};

#endif // LISTITEM_H
