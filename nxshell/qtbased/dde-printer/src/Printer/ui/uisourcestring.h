// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UISOURCESTRING_H
#define UISOURCESTRING_H
#include <QObject>
#include <QMargins>
#include <QVariant>
#define UI_PRINTERSEARCH_MANUAL QObject::tr("Select a driver")
#define UI_PRINTERSEARCH_URITIP "ipp://printer.mydomain/ipp\n" \
    "ipp://cups-sever/printers/printer-queue\n" \
    "smb://[username:password@][workgroup/]server/printer\n" \
    "lpd://sever/printer-queue\n" \
    "socket://server[:port]"
#define UI_PRINTERSEARCH_DRIVER QObject::tr("Driver")
#define UI_PRINTERSEARCH_INSTALLDRIVER_NEXT QObject::tr("Next","button")
#define UI_PRINTERSEARCH_INSTALLDRIVER_AUTO QObject::tr("Install Driver","button")

#define UI_PRINTERDRIVER_PPDLABEL_NORMAL QObject::tr("Drag a PPD file here \n or")
#define UI_PRINTERDRIVER_PPDBTN_NORMAL QObject::tr("Select a PPD file","button")

#define UI_PRINTERSHOW_TROUBLE QObject::tr("Troubleshoot")
#define UI_PRINTERSHOW_CANCEL QObject::tr("Cancel")

#define UI_PRINTER_DRIVER_WEBSITE "https://faq.uniontech.com/desktop/peripheral"
#define UI_PRINTER_DRIVER_MESSAGE QObject::tr("Printer problem help: ")
#define UI_PRINTER_DRIVER_WEB_LINK "<a href='%1' style='text-decoration: none; '>%1</a>"

#define UI_PRINTER_HPLIP_TIPINFO QObject::tr("After the driver is successfully installed, you need to configure the printer. Please do so in the navigation window.")

#define UI_PRINTER_MANUAL_TIPINFO QObject::tr("Unable to connect to printer. Please check the IP address or hostname and try again. Make sure the printer is turned on and the IP address or hostname is correct.")
Q_DECLARE_METATYPE(QMargins)
const QMargins ListViweItemMargin(10, 8, 10, 8);
const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);
#endif // UISOURCESTRING_H
