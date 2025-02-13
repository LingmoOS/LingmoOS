/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2024 Reion Wong <aj@lingmo.org>                     *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *   Copyright (C) 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef STATUSNOTIFIERITEMSOURCE_H
#define STATUSNOTIFIERITEMSOURCE_H

#include <QObject>
#include <QMenu>
#include <QDBusPendingCallWatcher>

#include <QQuickWindow>
#include <QQuickItem>
#include <QWindow>

#include "statusnotifieritem_interface.h"

class DBusMenuImporter;
class StatusNotifierItemSource : public QObject
{
    Q_OBJECT

public:
    explicit StatusNotifierItemSource(const QString &service, QObject *parent = nullptr);
    ~StatusNotifierItemSource();

    QString id() const;
    QString appId() const;
    QString title() const;
    QString tooltip() const;
    QString subtitle() const;
    QString iconName() const;
    QIcon icon() const;

    void activate(int x, int y);
    void secondaryActivate(int x, int y);
    void scroll(int delta, const QString &direction);
    void contextMenu(int x, int y, QQuickItem *item);

signals:
    void contextMenuReady(QMenu *menu);
    void activateResult(bool success);
    void updated(StatusNotifierItemSource *);

private slots:
    void contextMenuReady();
    void refreshTitle();
    void refreshIcons();
    void refreshToolTip();
    void refresh();
    void performRefresh();
    void syncStatus(QString);
    void refreshCallback(QDBusPendingCallWatcher *);
    void activateCallback(QDBusPendingCallWatcher *);

private:
    QPixmap KDbusImageStructToPixmap(const KDbusImageStruct &image) const;
    QIcon imageVectorToPixmap(const KDbusImageVector &vector) const;

private:
    bool m_valid;
    QString m_name;
    QTimer m_refreshTimer;
    DBusMenuImporter *m_menuImporter;
    org::kde::StatusNotifierItem *m_statusNotifierItemInterface;
    bool m_refreshing : 1;
    bool m_needsReRefreshing : 1;
    bool m_titleUpdate : 1;
    bool m_iconUpdate : 1;
    bool m_tooltipUpdate : 1;
    bool m_statusUpdate : 1;

    QString m_appId;
    QString m_id;
    QString m_title;
    QString m_tooltip;
    QString m_subTitle;
    QString m_iconName;
    QIcon m_icon;
};

#endif // STATUSNOTIFIERITEMSOURCE_H
