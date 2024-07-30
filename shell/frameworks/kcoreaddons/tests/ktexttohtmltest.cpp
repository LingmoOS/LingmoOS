/*
    SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KTextToHTML>

#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QTextBrowser>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    auto *window = new QMainWindow;

    auto *mainWidget = new QWidget;
    window->setCentralWidget(mainWidget);

    auto *layout = new QHBoxLayout;
    mainWidget->setLayout(layout);

    auto *plaintextEditor = new QTextEdit;
    plaintextEditor->setAcceptRichText(false);
    layout->addWidget(plaintextEditor);

    auto *htmlView = new QTextBrowser;
    layout->addWidget(htmlView);

    auto *updateTimer = new QTimer(&app);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(1000);
    QObject::connect(updateTimer, &QTimer::timeout, plaintextEditor, [plaintextEditor, htmlView]() {
        const QString html = KTextToHTML::convertToHtml(plaintextEditor->toPlainText(), KTextToHTML::Options());
        htmlView->setHtml(html);
    });

    QObject::connect(plaintextEditor, &QTextEdit::textChanged, updateTimer, qOverload<>(&QTimer::start));

    window->show();

    return app.exec();
}
