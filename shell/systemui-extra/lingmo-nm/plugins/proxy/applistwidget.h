/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef APPLISTWIDGET_H
#define APPLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QToolButton>

class AppListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppListWidget(QString path, QWidget *parent = nullptr);
    ~AppListWidget();

    void setAppChecked(bool flag);
    void setAppIcon(const QIcon &icon);
    void setAppName(const QString &text);
    void onAppCheckStateChanged();
    void AddAppProxyConfig();
    void RemoveAppProxyConfig();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void initUI();
    void initDbus();

    QCheckBox *m_checkBox = nullptr;
    QToolButton *m_iconBtn = nullptr;
    QLabel    *m_nameLabel = nullptr;
    QString    m_path = nullptr;
    QDBusInterface *m_dbusInterface = nullptr;
};

#endif // APPLISTWIDGET_H
