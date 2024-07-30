/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "draganddropplugin.h"

#include "DeclarativeDragArea.h"
#include "DeclarativeDragDropEvent.h"
#include "DeclarativeDropArea.h"
#include "DeclarativeMimeData.h"
#include "MimeDataWrapper.h"

void DragAndDropPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.draganddrop"));

    qmlRegisterType<DeclarativeDropArea>(uri, 2, 0, "DropArea");
    qmlRegisterType<DeclarativeDragArea>(uri, 2, 0, "DragArea");
    qmlRegisterAnonymousType<QMimeData>(uri, 1);
    qmlRegisterUncreatableType<DeclarativeMimeData>(uri, 2, 0, "MimeData", QStringLiteral("MimeData cannot be created from QML."));
    qmlRegisterUncreatableType<DeclarativeDragDropEvent>(uri, 2, 0, "DragDropEvent", QStringLiteral("DragDropEvent cannot be created from QML."));
    qmlRegisterUncreatableType<MimeDataWrapper>(uri, 2, 0, "MimeDataWrapper", QStringLiteral("DragDropEvent cannot be created from QML."));
}

#include "moc_draganddropplugin.cpp"
