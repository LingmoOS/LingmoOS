// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboard.h"
#include "dbusnotify.h"
#include "settings.h"
#include "utils.h"
#include <QApplication>
#include <QClipboard>
#include <QtDBus>
#include <QDebug>

Clipboard::Clipboard(QObject *parent) : QObject(parent)
{

}

Clipboard::~Clipboard()
{

}

void Clipboard::copyToClipboard(QColor color, QString colorType)
{
    // Save color type to config file.

    QString colorString;
    if (colorType == "HEX") {
        colorString = Utils::colorToHex(color);
    } else if (colorType == "RGB") {
        colorString = Utils::colorToRGB(color);
    } else if (colorType == "RGBA") {
        colorString = Utils::colorToRGBA(color);
    } else if (colorType == "Float_RGB") {
        colorString = Utils::colorToFloatRGB(color);
    } else if (colorType == "Float_RGBA") {
        colorString = Utils::colorToFloatRGBA(color);
    } else if (colorType == "CMYK") {
        colorString = Utils::colorToCMYK(color);
    } else if (colorType == "HSV") {
        colorString = Utils::colorToHSV(color);
    }

    Settings settings;
    settings.setOption("color_type", colorType);

    // Popup notify.
    DBusNotify *notifyDBus = new DBusNotify(this);
    notifyDBus->CloseNotification(0); // we need hide last time system notify first

    QStringList actions;
    QVariantMap hints;

    notifyDBus->Notify(
        QCoreApplication::applicationName(),
        0,
        "deepin-picker",
        "",
        QString(tr("Copy color %1 to clipboard")).arg(colorString), // body
        actions,
        hints,
        5000);

    // Copy to clipbard.
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(colorString);

    // Quit application. Delay exit by 100ms so that the color value can be stored in the clipboard normally
    QTimer::singleShot(100, this,[=](){
        QApplication::quit();
    });
}
