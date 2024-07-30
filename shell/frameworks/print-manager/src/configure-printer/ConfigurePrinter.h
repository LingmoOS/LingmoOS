/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGURE_PRINTER_H
#define CONFIGURE_PRINTER_H

#include <QApplication>

class ConfigurePrinterInterface;
class ConfigurePrinter : public QApplication
{
    Q_OBJECT
public:
    ConfigurePrinter(int &argc, char **argv);
    ~ConfigurePrinter() override;

    void configurePrinter(const QString &printer);

private:
    ConfigurePrinterInterface *m_cpInterface = nullptr;
};

#endif // CONFIGURE_PRINTER_H
