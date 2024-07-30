/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documenthandler.h"
#include "note.h"
#include "notemanager.h"

#include <QFile>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class NotesHelper : public QObject
{
    Q_OBJECT

public:
    explicit NotesHelper(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~NotesHelper() override = default;

    Q_INVOKABLE QString fileContents(const QString &path) const
    {
        const QUrl &url = QUrl::fromUserInput(path);
        if (!url.isValid()) {
            return QString();
        }

        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            return QString();
        }

        return QString::fromUtf8(file.readAll());
    }
};

class NotesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override

    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.notes"));
        qmlRegisterType<DocumentHandler>(uri, 0, 1, "DocumentHandler");
        qmlRegisterType<NoteManager>(uri, 0, 1, "NoteManager");
        qmlRegisterUncreatableType<Note>(uri, 0, 1, "Note", QStringLiteral("Create through NoteManager"));
        qmlRegisterSingletonType<NotesHelper>(uri, 0, 1, "NotesHelper", [](QQmlEngine *, QJSEngine *) {
            return new NotesHelper();
        });
    }
};

#include "notesplugin.moc"
