/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADDPRINTER_H
#define ADDPRINTER_H

#include <QApplication>

class AddPrinter : public QApplication
{
    Q_OBJECT
public:
    AddPrinter(int &argc, char **argv);
    ~AddPrinter() override;

    /**
     * This method allows to browse discovered printers and add them
     */
    void addPrinter(qulonglong wid);

    /**
     * This method allows to browse printers and create a class
     */
    void addClass(qulonglong wid);

    /**
     * This method allows to change the PPD of an existing printer
     */
    void changePPD(qulonglong wid, const QString &name);

    /**
     * This method allows to browse the PPD list,
     * and adding the printer described by device_id
     */
    void newPrinterFromDevice(qulonglong wid, const QString &name, const QString &device_id);

private:
    void show(QWidget *widget, qulonglong wid) const;
};

#endif // ADDPRINTER_H
