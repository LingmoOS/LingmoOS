/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QApplication>
#include <QVBoxLayout>

#include <kfontrequester.h>

class KFontRequesterTest : public QWidget
{
public:
    KFontRequesterTest(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        KFontRequester *test1 = new KFontRequester(this);
        mainLayout->addWidget(test1);

        KFontRequester *test2 = new KFontRequester(this);
        test2->setSampleText(QStringLiteral("This is different sample text"));
        test2->setTitle(QStringLiteral("A different title"));
        test2->setFont(QFont(QStringLiteral("comic-sans"), 12, 1));

        mainLayout->addWidget(test2);
    }
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kfontrequestertest"));
    QApplication app(argc, argv);

    KFontRequesterTest *mainWidget = new KFontRequesterTest;
    mainWidget->show();

    return app.exec();
}
