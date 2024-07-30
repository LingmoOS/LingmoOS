/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kbusyindicatorwidget.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget window;
    window.setBaseSize(128, 128);
    auto layout = new QVBoxLayout(&window);

    auto busyWidget = new QWidget(&window);
    auto busyLayout = new QHBoxLayout(busyWidget);
    auto busyIndicator = new KBusyIndicatorWidget(&window);
    auto busyLabel = new QLabel(QStringLiteral("Busy..."), &window);
    busyLayout->addWidget(busyIndicator);
    busyLayout->addWidget(busyLabel);

    auto toggle = new QPushButton(QStringLiteral("Toggle Visible"), &window);

    QObject::connect(toggle, &QPushButton::clicked, busyWidget, [=] {
        busyWidget->setVisible(!busyWidget->isVisible());
    });

    layout->addWidget(toggle);
    layout->addWidget(busyWidget);
    layout->setAlignment(Qt::AlignTop);

    window.show();
    return app.exec();
}
