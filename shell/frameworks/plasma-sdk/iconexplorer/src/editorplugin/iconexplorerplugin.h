/*
    SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUTTLEFISHPLUGIN_H
#define CUTTLEFISHPLUGIN_H

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>

#include <QList>
#include <QMenu>
#include <QProcess>
#include <QVariant>

class CuttleFishPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit CuttleFishPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~CuttleFishPlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

private Q_SLOTS:
    void contextMenuAboutToShow(KTextEditor::View *view, QMenu *menu);
    void documentCreated(KTextEditor::Document *document);
    void viewCreated(KTextEditor::Document *document, KTextEditor::View *view);

private:
    QList<QMenu *> m_decorated;
};

#endif // CUTTLEFISHPLUGIN_H
