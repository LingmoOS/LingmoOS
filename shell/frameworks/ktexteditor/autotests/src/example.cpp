/*
    This file is part of the Kate project.

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: MIT
*/
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow m;

    auto e = KTextEditor::Editor::instance();
    auto doc = e->createDocument(nullptr);

    if (argc > 1) {
        doc->openUrl(QUrl::fromLocalFile(app.arguments()[1]));
    }

    doc->setModifiedOnDiskWarning(true);

    //     auto docConfig = qobject_cast<KTextEditor::ConfigInterface*>(doc);
    //     docConfig->setConfigValue(QStringLiteral("replace-tabs"), false);

    auto v = doc->createView(&m);
    v->setContextMenu(v->defaultContextMenu());
    //     auto vConfig = qobject_cast<KTextEditor::ConfigInterface*>(v);
    //     vConfig->setConfigValue(QStringLiteral("auto-brackets"), true);

    //     v->setCursorPosition({6, 16});

    QToolBar tb(&m);
    tb.addAction(QStringLiteral("Config..."), &m, [e, &m] {
        e->configDialog(&m);
    });

    m.addToolBar(&tb);

    m.setCentralWidget(v);
    m.showMaximized();

    return app.exec();
}
