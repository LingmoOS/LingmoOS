/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "abstract-window-manager.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
namespace LingmoUIQuick {
static const QString HIGHLIGHT_WINDOW_SERVICE = QStringLiteral("org.kde.KWin.HighlightWindow");
static const QString HIGHLIGHT_WINDOW_PATH = QStringLiteral("/org/kde/KWin/HighlightWindow");
static const QString &HIGHLIGHT_WINDOW_INTERFACE = HIGHLIGHT_WINDOW_SERVICE;

void AbstractWindowManager::activateWindowView(const QStringList &wids)
{
    auto message = QDBusMessage::createMethodCall(HIGHLIGHT_WINDOW_SERVICE, HIGHLIGHT_WINDOW_PATH, HIGHLIGHT_WINDOW_INTERFACE, QStringLiteral("highlightWindows"));
    message << wids;
    QDBusConnection::sessionBus().asyncCall(message);
}
}