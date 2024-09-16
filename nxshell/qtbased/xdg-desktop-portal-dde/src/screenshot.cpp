// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "screenshot.h"

#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QUrl>
#include <QStandardPaths>
#include <QDateTime>
#include <qloggingcategory.h>
#include <qstringliteral.h>
#include <QColor>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopDDEScreenShot, "xdg-dde-screenshot")
Q_DECLARE_METATYPE(ScreenshotPortal::ColorRGB)

QDBusArgument &operator<<(QDBusArgument &arg, const ScreenshotPortal::ColorRGB &color)
{
    arg.beginStructure();
    arg << color.red << color.green << color.blue;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ScreenshotPortal::ColorRGB &color)
{
    double red, green, blue;
    arg.beginStructure();
    arg >> red >> green >> blue;
    color.red = red;
    color.green = green;
    color.blue = blue;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator<<(QDBusArgument &argument, const QColor &color)
{
    argument.beginStructure();
    argument << color.rgba();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QColor &color)
{
    argument.beginStructure();
    QRgb rgba;
    argument >> rgba;
    argument.endStructure();
    color = QColor::fromRgba(rgba);
    return argument;
}
ScreenshotPortal::ScreenshotPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<QColor>();
    qDBusRegisterMetaType<ColorRGB>();
    qCDebug(XdgDesktopDDEScreenShot) << "Screenshot and ColorPicker init";
}

uint ScreenshotPortal::PickColor(const QDBusObjectPath &handle,
                                 const QString &app_id,
                                 const QString &parent_window,
                                 const QVariantMap &options,
                                 QVariantMap &results)
{
    qCDebug(XdgDesktopDDEScreenShot) << "Start ColorPicker";
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/ColorPicker"),
                                                      QStringLiteral("org.kde.kwin.ColorPicker"),
                                                      QStringLiteral("pick"));
    QDBusPendingReply<QColor> pcall = QDBusConnection::sessionBus().call(msg);
    if (pcall.isValid()) {
        QColor selectedColor = pcall.value();
        ColorRGB color;
        color.red = selectedColor.redF();
        color.green = selectedColor.greenF();
        color.blue = selectedColor.blueF();
        results.insert(QStringLiteral("color"), QVariant::fromValue<ScreenshotPortal::ColorRGB>(color));
        return 0;
    }
    qCDebug(XdgDesktopDDEScreenShot) << "ColorPicker Failed";
    return 1;
}

// TODO: maybe need update
uint ScreenshotPortal::Screenshot(const QDBusObjectPath &handle,
                                  const QString &app_id,
                                  const QString &parent_window,
                                  const QVariantMap &options,
                                  QVariantMap &results)
{
    qCDebug(XdgDesktopDDEScreenShot) << "Start screenshot";
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/Screenshot"),
                                                      QStringLiteral("org.kde.kwin.Screenshot"),
                                                      QStringLiteral("screenshotFullscreen"));
    QDBusPendingReply<QString> pcall = QDBusConnection::sessionBus().call(msg);
    pcall.waitForFinished();
    if (pcall.isValid()) {
        auto filepath = pcall.value();
        qCDebug(XdgDesktopDDEScreenShot) << "Succeed" << QString("Filepath is %1").arg(filepath);
        results.insert(QStringLiteral("uri"), QUrl::fromLocalFile(filepath).toString(QUrl::FullyEncoded));
        return 0;
    }
    qCDebug(XdgDesktopDDEScreenShot) << "Screenshot Failed";
    return 1;
}
