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

// From https://invent.kde.org/qt/qt/qtbase/blob/6.7/src/gui/text/qfont.cpp#L146
static int convertWeights(int weight, bool inverted)
{
    static constexpr int legacyToOpenTypeMap[][2] = {
        {0, 100},
        {12, 200},
        {25, 300},
        {50, 400},
        {57, 500},
        {63, 600},
        {75, 700},
        {81, 800},
        {87, 900},
    };

    int closestDist = INT_MAX;
    int result = -1;

    // Go through and find the closest mapped value
    for (const auto &mapping : legacyToOpenTypeMap) {
        const int weightOld = mapping[inverted];
        const int weightNew = mapping[!inverted];
        const int dist = qAbs(weightOld - weight);
        if (dist < closestDist) {
            result = weightNew;
            closestDist = dist;
        } else {
            // Break early since following values will be further away
            break;
        }
    }

    return result;
}

// Qt5: https://invent.kde.org/qt/qt/qtbase/blob/5.15/src/gui/text/qfont.cpp#L2110
// Qt6: https://invent.kde.org/qt/qt/qtbase/blob/6.7/src/gui/text/qfont.cpp#L2135
static QString convertQt6FontStringToQt5(const QString &fontInfo)
{
    const auto parts = fontInfo.trimmed().split(QLatin1Char(','));
    const int count = parts.count();

    if (count != 16 && count != 17) {
        return fontInfo;
    }

    auto result = parts.mid(0, 10);
    result[4] = QString::number(convertWeights(parts[4].toInt(), true));

    if (count == 17) {
        result << parts.last();
    }
    return result.join(QLatin1Char(','));
}

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
            cachedFont->fromString(convertQt6FontStringToQt5(fontInfo));
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
