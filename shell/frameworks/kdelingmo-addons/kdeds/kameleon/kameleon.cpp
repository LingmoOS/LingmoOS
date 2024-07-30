/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleon.h"
#include "kameleon_debug.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <KAuth/Action>
#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KAuth/HelperSupport>

#include <QCollator>
#include <QColor>
#include <QDir>
#include <QFileInfo>

K_PLUGIN_CLASS_WITH_JSON(Kameleon, "kameleon.json")

Kameleon::Kameleon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_config(KSharedConfig::openConfig("kdeglobals"))
    , m_configWatcher(KConfigWatcher::create(m_config))
{
    findRgbLedDevices();
    if (!isSupported()) {
        qCInfo(KAMELEON) << "found no RGB LED devices";
        return;
    }

    loadConfig();
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, [this]() {
        loadConfig();
    });
}

void Kameleon::findRgbLedDevices()
{
    QDir ledsDir(LED_SYSFS_PATH);
    ledsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    auto ledDevices = ledsDir.entryList();

    // Sort to ensure keys light up nicely one next to the other
    QCollator coll;
    coll.setNumericMode(true);
    std::sort(ledDevices.begin(), ledDevices.end(), [&](const QString &s1, const QString &s2) {
        return coll.compare(s1, s2) < 0;
    });

    for (const QString &ledDevice : std::as_const(ledDevices)) {
        // Get multicolor index (= RGB capability with order of colors)
        QFile indexFile(LED_SYSFS_PATH + ledDevice + LED_INDEX_FILE);
        if (!QFileInfo(indexFile).exists()) {
            // Not a RGB capable device
            continue;
        }
        if (!indexFile.open(QIODevice::ReadOnly)) {
            qCWarning(KAMELEON) << "failed to open" << indexFile.fileName() << indexFile.error() << indexFile.errorString();
            continue;
        }
        QString colorIndexStr = indexFile.readAll().trimmed();
        indexFile.close();
        QString colorIndex =
            colorIndexStr.toLower().replace("red", "r").replace("green", "g").replace("blue", "b").replace(" ", ""); // eg "red green blue" -> "rgb"
        if (!(colorIndex.length() == 3 && colorIndex.contains("r") && colorIndex.contains("g") && colorIndex.contains("b"))) {
            qCWarning(KAMELEON) << "invalid color index" << colorIndexStr << "read from" << LED_INDEX_FILE << "for device" << ledDevice;
            continue;
        }

        qCInfo(KAMELEON) << "found RGB LED device" << ledDevice;
        m_rgbLedDevices.append(ledDevice);
        m_deviceRgbIndices.append(colorIndex);
    }
}

bool Kameleon::isSupported()
{
    return !m_rgbLedDevices.isEmpty();
}

bool Kameleon::isEnabled()
{
    return m_enabled;
}

void Kameleon::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        qCInfo(KAMELEON) << "enabled changed" << enabled;
        m_enabled = enabled;
        m_config->group("General").writeEntry<bool>("DeviceLedsAccentColored", enabled);

        if (enabled) {
            applyColor(m_accentColor);
        } else {
            applyColor(QColor(QColorConstants::White));
        }
    }
}

void Kameleon::loadConfig()
{
    m_enabled = m_config->group("General").readEntry<bool>("DeviceLedsAccentColored", true);

    QColor customAccentColor = m_config->group("General").readEntry<QColor>("AccentColor", QColor::Invalid);
    QColor schemeAccentColor = m_config->group("Colors:View").readEntry<QColor>("ForegroundActive", QColor::Invalid);
    QColor activeAccentColor = customAccentColor.isValid() ? customAccentColor
        : schemeAccentColor.isValid()                      ? schemeAccentColor
                                                           : QColor(QColorConstants::White);

    if (activeAccentColor != m_accentColor) {
        m_accentColor = activeAccentColor;
        if (m_enabled) {
            applyColor(m_accentColor);
        }
    }
}

void Kameleon::applyColor(QColor color)
{
    QStringList colorStrs;
    for (const QString &colorIndex : std::as_const(m_deviceRgbIndices)) {
        QStringList colorStrList = {QString(), QString(), QString()};
        colorStrList[colorIndex.indexOf("r")] = QString::number(color.red());
        colorStrList[colorIndex.indexOf("g")] = QString::number(color.green());
        colorStrList[colorIndex.indexOf("b")] = QString::number(color.blue());
        QString colorStr = colorStrList.join(" ");
        colorStrs.append(colorStr);
    }

    KAuth::Action action("org.kde.kameleonhelper.writecolor");
    action.setHelperId("org.kde.kameleonhelper");
    action.addArgument("devices", m_rgbLedDevices);
    action.addArgument("colors", colorStrs);
    auto *job = action.execute();

    connect(job, &KAuth::ExecuteJob::result, this, [job, color] {
        if (job->error()) {
            qCWarning(KAMELEON) << "failed to write color to devices" << job->errorText();
            return;
        }
        qCInfo(KAMELEON) << "wrote color" << color.name() << "to LED devices";
    });
    job->start();
}

#include "kameleon.moc"
