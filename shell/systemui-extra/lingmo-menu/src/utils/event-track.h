/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_MENU_EVENT_TRACK_H
#define LINGMO_MENU_EVENT_TRACK_H

#include <QObject>
#include <QVariant>
#include <qqml.h>

namespace LingmoUIMenu {

class EventTrack : public QObject
{
    Q_OBJECT
public:
    static EventTrack *qmlAttachedProperties(QObject *object);
    static EventTrack *instance();

    explicit EventTrack(QObject *parent = nullptr);
    Q_INVOKABLE void sendClickEvent(const QString& event, const QString& page, const QVariantMap &map = {});
    Q_INVOKABLE void sendDefaultEvent(const QString& event, const QString& page, const QVariantMap &map = {});
    Q_INVOKABLE void sendSearchEvent(const QString& event, const QString& page, const QString& content);
};

} // Sidebar

QML_DECLARE_TYPEINFO(LingmoUIMenu::EventTrack, QML_HAS_ATTACHED_PROPERTIES)

#endif //LINGMO_MENU_EVENT_TRACK_H
