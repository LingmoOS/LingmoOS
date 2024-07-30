/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpagedialogtest.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>

KPageDialogTest::KPageDialogTest(QWidget *parent)
    : KPageDialog(parent)
{
    setFaceType(Tabbed);

    QWidget *page = new QWidget(this);
    QBoxLayout *layout = new QVBoxLayout(page);

    QLabel *label = new QLabel(QStringLiteral("first page"));
    layout->addWidget(label);

    label = new QLabel(QStringLiteral("This is a very long text that is contained in a single string within a single label"));
    layout->addWidget(label);

    addPage(page, QStringLiteral("First"));

    page = new QWidget(this);
    layout = new QHBoxLayout(page);

    label = new QLabel(QStringLiteral("second page"));
    label->setMinimumSize(300, 200);
    layout->addWidget(label);

    addPage(page, QStringLiteral("Second"));
}

KPageDialogTest::~KPageDialogTest()
{
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KPageDialogTest"));

    QApplication app(argc, argv);

    KPageDialogTest testDialog(nullptr);
    testDialog.exec();

    return 0;
}

#include "moc_kpagedialogtest.cpp"
