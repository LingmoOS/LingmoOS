/*
    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ksplittercollapserbuttongui_test.h"
#include "ksplittercollapserbutton.h"
#include <QSplitter>
#include <qapplication.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QHBoxLayout>
#include <QTextEdit>

KSplitterCollapserButtonGui_test::KSplitterCollapserButtonGui_test(int indexOfWidgetAssociateToSplitterCollapser, Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    QSplitter *splitter = new QSplitter(orientation);
    lay->addWidget(splitter);
    QTextEdit *firstTextEdit = new QTextEdit;
    splitter->addWidget(firstTextEdit);
    QTextEdit *secondTextEdit = new QTextEdit;
    splitter->addWidget(secondTextEdit);
    if (indexOfWidgetAssociateToSplitterCollapser == 0) {
        new KSplitterCollapserButton(firstTextEdit, splitter);
    } else {
        new KSplitterCollapserButton(secondTextEdit, splitter);
    }
}

KSplitterCollapserButtonGui_test::~KSplitterCollapserButtonGui_test()
{
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    KSplitterCollapserButtonGui_test *w = new KSplitterCollapserButtonGui_test(0, Qt::Horizontal);
    w->resize(800, 600);
    w->show();

    KSplitterCollapserButtonGui_test *w2 = new KSplitterCollapserButtonGui_test(1, Qt::Horizontal);
    w2->resize(800, 600);
    w2->show();

    KSplitterCollapserButtonGui_test *w3 = new KSplitterCollapserButtonGui_test(0, Qt::Vertical);
    w3->resize(800, 600);
    w3->show();

    KSplitterCollapserButtonGui_test *w4 = new KSplitterCollapserButtonGui_test(1, Qt::Vertical);
    w4->resize(800, 600);
    w4->show();

    app.exec();
    delete w;
    delete w2;
    delete w3;
    delete w4;
    return 0;
}

#include "moc_ksplittercollapserbuttongui_test.cpp"
