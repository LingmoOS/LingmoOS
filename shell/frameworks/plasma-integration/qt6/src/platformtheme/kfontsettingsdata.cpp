/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfontsettingsdata.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QString>
#include <QVariant>
#include <qpa/qwindowsysteminterface.h>

#include <KSandbox>
#include <kconfiggroup.h>

KFontSettingsData::KFontSettingsData()
    : QObject(nullptr)
    , mUsePortal(KSandbox::isInside())
    , mKdeGlobals(KSharedConfig::openConfig())
{
    QMetaObject::invokeMethod(this, "delayedDBusConnects", Qt::QueuedConnection);

    for (int i = 0; i < FontTypesCount; ++i) {
        mFonts[i] = nullptr;
    }
}

KFontSettingsData::~KFontSettingsData()
{
    for (int i = 0; i < FontTypesCount; ++i) {
        delete mFonts[i];
    }
}

// NOTE: keep in sync with plasma-desktop/kcms/fonts/fonts.cpp
static const char GeneralId[] = "General";
static const char DefaultFont[] = "Noto Sans";

static const KFontData DefaultFontData[KFontSettingsData::FontTypesCount] = {
    {GeneralId, "font", DefaultFont, 10, QFont::Normal, QFont::SansSerif, "Regular"},
    {GeneralId, "fixed", "Hack", 10, QFont::Normal, QFont::Monospace, "Regular"},
    {GeneralId, "toolBarFont", DefaultFont, 10, QFont::Normal, QFont::SansSerif, "Regular"},
    {GeneralId, "menuFont", DefaultFont, 10, QFont::Normal, QFont::SansSerif, "Regular"},
    {"WM", "activeFont", DefaultFont, 10, QFont::Normal, QFont::SansSerif, "Regular"},
    {GeneralId, "taskbarFont", DefaultFont, 10, QFont::Normal, QFont::SansSerif, "Regular"},
    {GeneralId, "smallestReadableFont", DefaultFont, 8, QFont::Normal, QFont::SansSerif, "Regular"},
};

QFont *KFontSettingsData::font(FontTypes fontType)
{
    QFont *cachedFont = mFonts[fontType];

    if (!cachedFont) {
        const KFontData &fontData = DefaultFontData[fontType];
        cachedFont = new QFont(QLatin1String(fontData.FontName), fontData.Size, fontData.Weight);
        cachedFont->setStyleHint(fontData.StyleHint);

        const QString fontInfo = readConfigValue(QLatin1String(fontData.ConfigGroupKey), QLatin1String(fontData.ConfigKey));

        // If we have serialized information for this font, restore it
        // NOTE: We are not using KConfig directly because we can't call QFont::QFont from here
        if (!fontInfo.isEmpty()) {
            cachedFont->fromString(fontInfo);
        }
        // Don't set default font style names, as it prevents different font weights from being used (the QFont::Normal weight should work)

        mFonts[fontType] = cachedFont;
    }

    return cachedFont;
}

void KFontSettingsData::dropFontSettingsCache()
{
    mKdeGlobals->reparseConfiguration();
    for (int i = 0; i < FontTypesCount; ++i) {
        delete mFonts[i];
        mFonts[i] = nullptr;
    }

    QWindowSystemInterface::handleThemeChange(nullptr);

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QApplication::setFont(*font(KFontSettingsData::GeneralFont));
    } else {
        QGuiApplication::setFont(*font(KFontSettingsData::GeneralFont));
    }
}

void KFontSettingsData::delayedDBusConnects()
{
    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KDEPlatformTheme"),
                                          QStringLiteral("org.kde.KDEPlatformTheme"),
                                          QStringLiteral("refreshFonts"),
                                          this,
                                          SLOT(dropFontSettingsCache()));

    if (mUsePortal) {
        QDBusConnection::sessionBus().connect(QString(),
                                              QStringLiteral("/org/freedesktop/portal/desktop"),
                                              QStringLiteral("org.freedesktop.portal.Settings"),
                                              QStringLiteral("SettingChanged"),
                                              this,
                                              SLOT(slotPortalSettingChanged(QString, QString, QDBusVariant)));
    }
}

void KFontSettingsData::slotPortalSettingChanged(const QString &group, const QString &key, const QDBusVariant &value)
{
    Q_UNUSED(value);

    if (group == QLatin1String("org.kde.kdeglobals.General") && key == QLatin1String("font")) {
        dropFontSettingsCache();
    }
}

QString KFontSettingsData::readConfigValue(const QString &group, const QString &key, const QString &defaultValue) const
{
    if (mUsePortal) {
        const QString settingName = QStringLiteral("org.kde.kdeglobals.%1").arg(group);
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                              QStringLiteral("/org/freedesktop/portal/desktop"),
                                                              QStringLiteral("org.freedesktop.portal.Settings"),
                                                              QStringLiteral("Read"));
        message << settingName << key;

        // FIXME: async?
        QDBusReply<QVariant> reply = QDBusConnection::sessionBus().call(message);
        if (reply.isValid()) {
            QDBusVariant result = qvariant_cast<QDBusVariant>(reply.value());
            const QString resultStr = result.variant().toString();

            if (!resultStr.isEmpty()) {
                return resultStr;
            }
        }
    }

    const KConfigGroup configGroup(mKdeGlobals, group);
    return configGroup.readEntry(key, defaultValue);
}

#include "moc_kfontsettingsdata.cpp"
