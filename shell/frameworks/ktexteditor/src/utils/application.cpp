/*
    SPDX-FileCopyrightText: 2013 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/plugin.h>

#include "kateglobal.h"

namespace KTextEditor
{
Application::Application(QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

Application::~Application() = default;

bool Application::quit()
{
    // dispatch to parent
    bool success = false;
    QMetaObject::invokeMethod(parent(), "quit", Qt::DirectConnection, Q_RETURN_ARG(bool, success));

    return success;
}

QList<KTextEditor::MainWindow *> Application::mainWindows()
{
    // dispatch to parent
    QList<KTextEditor::MainWindow *> mainWindow;
    QMetaObject::invokeMethod(parent(), "mainWindows", Qt::DirectConnection, Q_RETURN_ARG(QList<KTextEditor::MainWindow *>, mainWindow));
    return mainWindow;
}

KTextEditor::MainWindow *Application::activeMainWindow()
{
    // dispatch to parent
    KTextEditor::MainWindow *window = nullptr;
    QMetaObject::invokeMethod(parent(), "activeMainWindow", Qt::DirectConnection, Q_RETURN_ARG(KTextEditor::MainWindow *, window));

    // always return some kind of window to not need to check for valid pointer
    return window ? window : KTextEditor::EditorPrivate::self()->dummyMainWindow();
}

QList<KTextEditor::Document *> Application::documents()
{
    // dispatch to parent
    QList<KTextEditor::Document *> documents;
    QMetaObject::invokeMethod(parent(), "documents", Qt::DirectConnection, Q_RETURN_ARG(QList<KTextEditor::Document *>, documents));
    return documents;
}

KTextEditor::Document *Application::findUrl(const QUrl &url)
{
    // dispatch to parent
    KTextEditor::Document *document = nullptr;
    QMetaObject::invokeMethod(parent(), "findUrl", Qt::DirectConnection, Q_RETURN_ARG(KTextEditor::Document *, document), Q_ARG(QUrl, url));
    return document;
}

KTextEditor::Document *Application::openUrl(const QUrl &url, const QString &encoding)
{
    // dispatch to parent
    KTextEditor::Document *document = nullptr;
    QMetaObject::invokeMethod(parent(),
                              "openUrl",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(KTextEditor::Document *, document),
                              Q_ARG(QUrl, url),
                              Q_ARG(QString, encoding));
    return document;
}

bool Application::closeDocument(KTextEditor::Document *document)
{
    // dispatch to parent
    bool success = false;
    QMetaObject::invokeMethod(parent(), "closeDocument", Qt::DirectConnection, Q_RETURN_ARG(bool, success), Q_ARG(KTextEditor::Document *, document));
    return success;
}

bool Application::closeDocuments(const QList<KTextEditor::Document *> &documents)
{
    // dispatch to parent
    bool success = false;
    QMetaObject::invokeMethod(parent(), "closeDocuments", Qt::DirectConnection, Q_RETURN_ARG(bool, success), Q_ARG(QList<KTextEditor::Document *>, documents));
    return success;
}

KTextEditor::Plugin *Application::plugin(const QString &name)
{
    // dispatch to parent
    Plugin *plugin = nullptr;
    QMetaObject::invokeMethod(parent(), "plugin", Qt::DirectConnection, Q_RETURN_ARG(KTextEditor::Plugin *, plugin), Q_ARG(QString, name));
    return plugin;
}

} // namespace KTextEditor
