/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEW_PRINTER_NOTIFICATION_H
#define NEW_PRINTER_NOTIFICATION_H

#include <QDBusContext>
#include <QObject>

class KNotification;
class NewPrinterNotification : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.redhat.NewPrinterNotification")
public:
    explicit NewPrinterNotification(QObject *parent);
    ~NewPrinterNotification() override;

public:
    void GetReady();
    void NewPrinter(int status, const QString &name, const QString &make, const QString &model, const QString &des, const QString &cmd);

private:
    bool registerService();
    void configurePrinter();
    void printTestPage();
    void findDriver();

    void setupPrinterNotification(KNotification *notify, const QString &make, const QString &model, const QString &description, const QString &arg);
    void getMissingExecutables(KNotification *notify, int status, const QString &name, const QString &ppdFileName);
    void checkPrinterCurrentDriver(KNotification *notify, const QString &name);
    void printerReadyNotification(KNotification *notify, const QString &name);
};

#endif // NEW_PRINTER_NOTIFICATION_H
