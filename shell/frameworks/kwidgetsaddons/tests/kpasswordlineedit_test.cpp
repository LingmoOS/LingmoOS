/*
    SPDX-FileCopyrightText: 2017 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QVBoxLayout>
#include <QWidget>

#include <QApplication>
#include <QCommandLineParser>

#include <KPasswordLineEdit>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(w);

    KPasswordLineEdit *passwordLineEdit = new KPasswordLineEdit(w);
    vbox->addWidget(passwordLineEdit);
    vbox->addStretch();

    w->resize(400, 400);
    w->show();

    return app.exec();
}
