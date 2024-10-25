/*
 * Copyright (C) 2021 LingmoOS Team.
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

#include "openwithdialog.h"
#include "../mimetype/mimeappmanager.h"
#include "../helper/filelauncher.h"

#include <QQmlEngine>
#include <QQmlContext>

OpenWithDialog::OpenWithDialog(const QUrl &url, QQuickView *parent)
    : QQuickView(parent)
    , m_url(url.toLocalFile())
{
    setFlag(Qt::Dialog);
    setTitle(tr("Open With"));
    setResizeMode(QQuickView::SizeViewToRootObject);

    engine()->rootContext()->setContextProperty("main", this);
    engine()->rootContext()->setContextProperty("mimeAppManager", MimeAppManager::self());
    engine()->rootContext()->setContextProperty("launcher", FileLauncher::self());

    setSource(QUrl("qrc:/qml/Dialogs/OpenWithDialog.qml"));

    QRect rect = geometry();
    setMinimumSize(rect.size());
    setMaximumSize(rect.size());

    connect(this, &QQuickView::visibleChanged, this, [=] {
        if (!this->isVisible())
            this->deleteLater();
    });
}

QString OpenWithDialog::url() const
{
    return m_url;
}
