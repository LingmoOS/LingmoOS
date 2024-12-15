// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearanceproperty.h"
#include "modules/common/commondefine.h"

#include <QDBusMessage>
#include <QDBusConnection>

void AppearancePropertiesChanged(const QString &property, const QVariant &value)
{
    QVariantMap properties;
    properties.insert(property, value);

    QList<QVariant> arguments;
    arguments.push_back(APPEARANCE_INTERFACE);
    arguments.push_back(properties);
    arguments.push_back(QStringList());

    QDBusMessage msg = QDBusMessage::createSignal(APPEARANCE_PATH, "org.freedesktop.DBus.Properties", "PropertiesChanged");
    msg.setArguments(arguments);
    APPEARANCEDBUS.send(msg);
}

AppearanceProperty::AppearanceProperty()
    : background("Background")
    , cursorTheme("CursorTheme")
    , fontSize("FontSize")
    , globalTheme("GlobalTheme")
    , gtkTheme("GtkTheme")
    , iconTheme("IconTheme")
    , monospaceFont("MonospaceFont")
    , opacity("Opacity")
    , qtActiveColor("QtActiveColor")
    , standardFont("StandardFont")
    , wallpaperSlideShow("WallpaperSlideShow")
    , wallpaperURls("WallpaperURls")
    , windowRadius("WindowRadius")
    , dtkSizeMode("DTKSizeMode")
    , qtScrollBarPolicy("QtScrollBarPolicy")
{
    fontSize.init(0.0);
    opacity.init(0.0);
    windowRadius.init(0);
}
