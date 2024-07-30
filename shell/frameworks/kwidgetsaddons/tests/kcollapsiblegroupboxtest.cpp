/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>
    Based on test program by Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "kdatepicker.h"
#include <kcollapsiblegroupbox.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget mainWindow;

    QVBoxLayout mainWindowLayout(&mainWindow);

    // add a layout with many items to make sure it doesn't shuffle them weirdly when animating
    {
        auto groupBox = new KCollapsibleGroupBox;
        groupBox->setTitle(QStringLiteral("&Advanced Options"));

        auto *innerLayout = new QVBoxLayout(groupBox);
        for (int i = 0; i < 6; i++) {
            auto checkBox = new QCheckBox(QStringLiteral("Some text"));
            innerLayout->addWidget(checkBox);
        }
        auto checkBox = new QCheckBox(QStringLiteral("Some really long text that goes on and on and on for ever and ever"));
        innerLayout->addWidget(checkBox);

        auto label = new QLabel(groupBox);
        label->setText(QStringLiteral("Some input field:"));
        // Word-wrapping in labels triggers a bug in the layout positioning.
        label->setWordWrap(true);
        innerLayout->addWidget(label);

        auto lineEdit = new QLineEdit(groupBox);
        innerLayout->addWidget(lineEdit);

        auto hiddenCheckBox = new QCheckBox(QStringLiteral("This will be always hidden"));
        innerLayout->addWidget(hiddenCheckBox);
        hiddenCheckBox->hide();

        mainWindowLayout.addWidget(groupBox);
    }

    // another item which should expand to fill width
    {
        auto groupBox = new KCollapsibleGroupBox;
        groupBox->setTitle(QStringLiteral("Pick a &date"));

        auto innerLayout = new QVBoxLayout(groupBox);
        auto datePicker = new KDatePicker();
        innerLayout->addWidget(datePicker);
        mainWindowLayout.addWidget(groupBox);
    }

    mainWindowLayout.addStretch();

    mainWindow.resize(400, 300);
    mainWindow.show();

    return app.exec();
}

// kate: replace-tabs on;
