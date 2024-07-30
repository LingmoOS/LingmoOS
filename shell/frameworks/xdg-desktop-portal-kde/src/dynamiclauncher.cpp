// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "dynamiclauncher.h"

#include <optional>

#include <QFile>
#include <QGuiApplication>
#include <QUrl>
#include <QWindow>

#include <KIconLoader>
#include <KLocalizedString>

#include "dynamiclauncher_debug.h"
#include "dynamiclauncherdialog.h"
#include "portalicon.h"
#include "request.h"
#include "utils.h"

DynamicLauncherPortal::DynamicLauncherPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    PortalIcon::registerDBusType();
}

// We want explicit support for types lest we incorrectly demarshal something.
template<typename T>
std::optional<T> readOption(const QString &key, const QVariantMap &options) = delete;

template<>
std::optional<bool> readOption(const QString &key, const QVariantMap &options)
{
    if (options.contains(key)) {
        return options.value(key).toBool();
    }
    return std::nullopt;
}

template<>
std::optional<QString> readOption(const QString &key, const QVariantMap &options)
{
    if (options.contains(key)) {
        return options.value(key).toString();
    }
    return std::nullopt;
}

template<>
std::optional<uint> readOption(const QString &key, const QVariantMap &options)
{
    if (options.contains(key)) {
        return options.value(key).toUInt();
    }
    return std::nullopt;
}

QIcon static extractIcon(const QDBusVariant &iconVariant)
{
    auto icon = qdbus_cast<PortalIcon>(iconVariant.variant());
    const QVariant iconData = icon.data.variant();
    // NB: The DynamicLauncher portal only accept GByteIcons, i.e. the only type we'll ever get are bytes.
    if (icon.str == QStringLiteral("bytes") && iconData.type() == QVariant::ByteArray) {
        QPixmap pixmap;
        pixmap.loadFromData(iconData.toByteArray());
        return pixmap;
    }
    return {};
}

static QString typeToTitle(uint type)
{
    switch (static_cast<DynamicLauncherPortal::Type>(type)) {
    case DynamicLauncherPortal::Type::Webapp:
        return i18nc("@title", "Add Web Application…");
    case DynamicLauncherPortal::Type::Application:
        break;
    }
    // Default value is Application; we treat all unmapped, possibly future, types as generic Application.
    return i18nc("@title", "Add Application…");
}

static QByteArray iconFromName(const QString &name)
{
    static constexpr auto maxSize = 512; // the portal never deals with larger icons; they'd likely be SVGs at that point anyway.
    const auto iconPath = KIconLoader::global()->iconPath(name, -maxSize, false);
    QFile iconFile(iconPath);
    if (!iconFile.open(QFile::ReadOnly)) {
        qWarning() << "Failed to read icon" << iconPath;
        return {};
    }
    return iconFile.readAll();
}

uint DynamicLauncherPortal::PrepareInstall(const QDBusObjectPath &handle,
                                           const QString &app_id,
                                           const QString &parent_window,
                                           const QString &name,
                                           const QDBusVariant &iconVariant,
                                           const QVariantMap &options,
                                           QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "PrepareInstall called with parameters:";
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    name: " << name;
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    iconVariant: " << iconVariant.variant();
    qCDebug(XdgDesktopPortalKdeDynamicLauncher) << "    options: " << options;

    const auto modal = readOption<bool>(QStringLiteral("modal"), options).value_or(true);
    const auto launcherType = readOption<uint>(QStringLiteral("launcher_type"), options).value_or(uint(Type::Application));
    const auto optionalTarget = readOption<QString>(QStringLiteral("target"), options);
    const auto editableName = readOption<bool>(QStringLiteral("editable_name"), options).value_or(true);
    const auto editableIcon = readOption<bool>(QStringLiteral("editable_icon"), options).value_or(false);

    static const QString nameKey = QStringLiteral("name");
    static const QString iconKey = QStringLiteral("icon");
    results[nameKey] = name;
    results[iconKey] = QVariant::fromValue(iconVariant);

    const auto icon = extractIcon(iconVariant);

    DynamicLauncherDialog dialog(typeToTitle(launcherType), icon, name, optionalTarget ? QUrl(optionalTarget.value()) : QUrl());
    dialog.windowHandle()->setModality(modal ? Qt::WindowModal : Qt::NonModal);
    Utils::setParentWindow(dialog.windowHandle(), parent_window);
    Request::makeClosableDialogRequest(handle, &dialog);

    const bool result = dialog.exec();

    if (editableName) {
        results[nameKey] = dialog.m_name;
    }

    if (editableIcon && dialog.m_icon != icon && dialog.m_icon.type() == QVariant::String) {
        const auto data = iconFromName(dialog.m_icon.toString());
        if (!data.isEmpty()) {
            const PortalIcon portalIcon{"bytes", QDBusVariant(data)};
            results[iconKey] = QVariant::fromValue(QDBusVariant(QVariant::fromValue(portalIcon)));
        }
    }

    return result ? 0 : 1;
}

uint DynamicLauncherPortal::RequestInstallToken(const QString &app_id, const QVariantMap &options)
{
    Q_UNUSED(options);

    // Blanket allow certain apps to create app entries. Ported from GTK portal.
    // Perhaps this should also include browsers? Unclear at the time of writing.

    static QStringList allowedIDs = {
        QStringLiteral("org.gnome.Software"),
        QStringLiteral("org.gnome.SoftwareDevel"),
        QStringLiteral("io.elementary.appcenter"),
        QStringLiteral("org.kde.discover"),
    };

    return allowedIDs.contains(app_id) ? 0 : 2;
}
