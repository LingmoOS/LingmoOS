/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGURE_PRINTER_INTERFACE_H
#define CONFIGURE_PRINTER_INTERFACE_H

#include <QDBusContext>

class QTimer;
class QWidget;
class ConfigurePrinterInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ConfigurePrinter")
public:
    explicit ConfigurePrinterInterface(QObject *parent = nullptr);
    ~ConfigurePrinterInterface() override;

signals:
    void quit();

public slots:
    void ConfigurePrinter(const QString &destName);
    void RemovePrinter();

private:
    QTimer *m_updateUi = nullptr;
    QHash<QString, QWidget *> m_uis;
};

#endif
