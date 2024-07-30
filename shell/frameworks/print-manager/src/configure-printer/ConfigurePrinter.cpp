/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConfigurePrinter.h"
#include "ConfigurePrinterInterface.h"

#include <QTimer>

ConfigurePrinter::ConfigurePrinter(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

void ConfigurePrinter::configurePrinter(const QString &printer)
{
    m_cpInterface = new ConfigurePrinterInterface(this);
    connect(m_cpInterface, &ConfigurePrinterInterface::quit, this, &ConfigurePrinter::quit);

    if (!printer.isEmpty()) {
        m_cpInterface->ConfigurePrinter(printer);
    } else {
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, m_cpInterface, &ConfigurePrinterInterface::RemovePrinter);
    }
}

ConfigurePrinter::~ConfigurePrinter()
{
}

#include "moc_ConfigurePrinter.cpp"
