/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmath>

#include "generator.h"
#include "device.h"
#include "kscreen_daemon_debug.h"
#include "output.h"
#include <QRect>

#include <kscreen/screen.h>

#if defined(QT_NO_DEBUG)
#define ASSERT_OUTPUTS(outputs)
#else
#define ASSERT_OUTPUTS(outputs)                                                                                                                                \
    while (true) {                                                                                                                                             \
        Q_ASSERT(!outputs.isEmpty());                                                                                                                          \
        for (const KScreen::OutputPtr &output : qAsConst(outputs)) {                                                                                           \
            Q_ASSERT(output);                                                                                                                                  \
            Q_ASSERT(output->isConnected());                                                                                                                   \
        }                                                                                                                                                      \
        break;                                                                                                                                                 \
    }
#endif

// The industry-standard "normal" 1x scale desktop monitor DPI value since forever
static const int targetDpiDesktop = 96;

// Higher because laptop screens are smaller and used closer to the face
static const int targetDpiLaptop = 125;

// Because phone and tablet screens are even smaller and used even closer
static const int targetDpiMobile = 136;

// Round calculated ideal scale factor to the nearest quarter
static const int scaleRoundingness = 4;

Generator *Generator::instance = nullptr;

bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

Generator *Generator::self()
{
    if (!Generator::instance) {
        Generator::instance = new Generator();
    }
    return Generator::instance;
}

Generator::Generator()
    : QObject()
    , m_forceLaptop(false)
    , m_forceLidClosed(false)
    , m_forceNotLaptop(false)
    , m_forceDocked(false)
{
    connect(Device::self(), &Device::ready, this, &Generator::ready);
}

void Generator::destroy()
{
    delete Generator::instance;
    Generator::instance = nullptr;
}

Generator::~Generator()
{
}

void Generator::setCurrentConfig(const KScreen::ConfigPtr &currentConfig)
{
    m_currentConfig = currentConfig;
}

KScreen::ConfigPtr Generator::idealConfig(const KScreen::ConfigPtr &currentConfig)
{
    Q_ASSERT(currentConfig);

    KScreen::ConfigPtr config = currentConfig->clone();

    disableAllDisconnectedOutputs(config->outputs());

    KScreen::OutputList connectedOutputs = config->connectedOutputs();
    qCDebug(KSCREEN_KDED) << "Connected outputs: " << connectedOutputs.count();

    if (connectedOutputs.isEmpty()) {
        return config;
    }

    for (const auto &output : connectedOutputs) {
        initializeOutput(output, config->supportedFeatures());
        output->setExplicitLogicalSize(config->logicalSizeForOutput(*output));
    }

    if (connectedOutputs.count() == 1) {
        singleOutput(config);
        return config;
    }

    if (isLaptop()) {
        laptop(config);
        return fallbackIfNeeded(config);
    }

    qCDebug(KSCREEN_KDED) << "Extend to Right";
    extendToRight(config, connectedOutputs);
    return fallbackIfNeeded(config);
}

KScreen::ConfigPtr Generator::fallbackIfNeeded(const KScreen::ConfigPtr &config)
{
    qCDebug(KSCREEN_KDED) << "fallbackIfNeeded()";

    KScreen::ConfigPtr newConfig;

    // If the ideal config can't be applied, try clonning
    if (!KScreen::Config::canBeApplied(config)) {
        if (isLaptop()) {
            newConfig = displaySwitch(Generator::Clone); // Try to clone at our best
        } else {
            newConfig = config;
            KScreen::OutputList connectedOutputs = config->connectedOutputs();
            if (connectedOutputs.isEmpty()) {
                return config;
            } else {
                config->setPrimaryOutput(connectedOutputs.first());
                cloneScreens(config);
            }
        }
    } else {
        newConfig = config;
    }

    // If after trying to clone at our best, we fail... return current
    if (!KScreen::Config::canBeApplied(newConfig)) {
        qCDebug(KSCREEN_KDED) << "Config cannot be applied";
        newConfig = config;
    }

    return config;
}

KScreen::ConfigPtr Generator::displaySwitch(DisplaySwitchAction action)
{
    KScreen::ConfigPtr config = m_currentConfig;
    Q_ASSERT(config);

    KScreen::OutputList connectedOutputs = config->connectedOutputs();

    for (const auto &output : connectedOutputs) {
        initializeOutput(output, config->supportedFeatures());
    }

    // There's not much else we can do with only one output
    if (connectedOutputs.count() < 2) {
        singleOutput(config);
        return config;
    }

    // We cannot try all possible combinations with two and more outputs
    if (connectedOutputs.count() > 2) {
        extendToRight(config, connectedOutputs);
        return config;
    }

    KScreen::OutputPtr embedded, external;
    embedded = embeddedOutput(connectedOutputs);
    // If we don't have an embedded output (desktop with two external screens
    // for instance), then pretend the current primary one is embedded
    if (!embedded) {
        // Find primary screen
        for (auto &screen : connectedOutputs) {
            if (screen->isPrimary()) {
                embedded = screen;
                break;
            }
        }
        if (!embedded) {
            // If all else fail take the first screen
            embedded = connectedOutputs.first();
        }
    }
    // Just to be sure
    if (embedded->modes().isEmpty()) {
        return config;
    }

    if (action == Generator::Clone) {
        qCDebug(KSCREEN_KDED) << "Cloning";
        config->setPrimaryOutput(embedded);
        cloneScreens(config);
        return config;
    }

    connectedOutputs.remove(embedded->id());
    external = connectedOutputs.constBegin().value();
    // Just to be sure
    if (external->modes().isEmpty()) {
        return config;
    }

    Q_ASSERT(embedded->currentMode());
    Q_ASSERT(external->currentMode());

    // Change action to be relative to embedded screen
    if (!embedded->isPrimary()) {
        switch (action) {
        case Generator::ExtendToLeft:
            action = Generator::ExtendToRight;
            break;
        case Generator::ExtendToRight:
            action = Generator::ExtendToLeft;
            break;
        default:
            break;
        }
    }

    switch (action) {
    case Generator::ExtendToLeft: {
        qCDebug(KSCREEN_KDED) << "Extend to left";
        external->setPos(QPoint(0, 0));
        external->setEnabled(true);

        const QSize size = external->geometry().size();
        embedded->setPos(QPoint(size.width(), 0));
        embedded->setEnabled(true);

        return config;
    }
    case Generator::TurnOffEmbedded: {
        qCDebug(KSCREEN_KDED) << "Turn off embedded (laptop)";
        embedded->setEnabled(false);
        external->setEnabled(true);
        config->setPrimaryOutput(external);
        return config;
    }
    case Generator::TurnOffExternal: {
        qCDebug(KSCREEN_KDED) << "Turn off external screen";
        embedded->setPos(QPoint(0, 0));
        embedded->setEnabled(true);
        external->setEnabled(false);
        config->setPrimaryOutput(embedded);
        return config;
    }
    case Generator::ExtendToRight: {
        qCDebug(KSCREEN_KDED) << "Extend to the right";
        embedded->setPos(QPoint(0, 0));
        embedded->setEnabled(true);

        Q_ASSERT(embedded->currentMode()); // we must have a mode now
        const QSize size = embedded->geometry().size();
        external->setPos(QPoint(size.width(), 0));
        external->setEnabled(true);

        return config;
    }
    case Generator::None: // just return config
    case Generator::Clone: // handled above
        break;
    } // switch

    return config;
}

uint qHash(const QSize &size)
{
    return size.width() * size.height();
}

void Generator::cloneScreens(const KScreen::ConfigPtr &config)
{
    KScreen::OutputList connectedOutputs = config->connectedOutputs();

    ASSERT_OUTPUTS(connectedOutputs);
    if (connectedOutputs.isEmpty()) {
        return;
    }

    QSet<QSize> commonSizes;
    const QSize maxScreenSize = config->screen()->maxSize();

    Q_FOREACH (const KScreen::OutputPtr &output, connectedOutputs) {
        QSet<QSize> modeSizes;
        Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
            const QSize size = mode->size();
            if (size.width() > maxScreenSize.width() || size.height() > maxScreenSize.height()) {
                continue;
            }
            modeSizes.insert(mode->size());
        }

        // If we have nothing to compare against
        if (commonSizes.isEmpty()) {
            commonSizes = modeSizes;
        } else {
            commonSizes.intersect(modeSizes);
        }

        // If there's already nothing in common, bail
        if (commonSizes.isEmpty()) {
            break;
        }
    }

    qCDebug(KSCREEN_KDED) << "Common sizes: " << commonSizes;
    // fallback to biggestMode if no commonSizes have been found
    if (commonSizes.isEmpty()) {
        for (KScreen::OutputPtr &output : connectedOutputs) {
            if (output->modes().isEmpty()) {
                continue;
            }
            output->setEnabled(true);
            output->setPos(QPoint(0, 0));
            const KScreen::ModePtr mode = biggestMode(output->modes());
            Q_ASSERT(mode);
            output->setCurrentModeId(mode->id());
        }
        return;
    }

    // At this point, we know we have common sizes, let's get the biggest on
    QList<QSize> commonSizeList = commonSizes.values();
    std::sort(commonSizeList.begin(), commonSizeList.end());
    const QSize biggestSize = commonSizeList.last();

    // Finally, look for the mode with biggestSize and biggest refreshRate and set it
    qCDebug(KSCREEN_KDED) << "Biggest Size: " << biggestSize;
    KScreen::ModePtr bestMode;
    for (KScreen::OutputPtr &output : connectedOutputs) {
        if (output->modes().isEmpty()) {
            continue;
        }
        bestMode = bestModeForSize(output->modes(), biggestSize);
        Q_ASSERT(bestMode); // we resolved this mode previously, so it better works
        output->setEnabled(true);
        output->setPos(QPoint(0, 0));
        output->setCurrentModeId(bestMode->id());
    }
}

void Generator::singleOutput(KScreen::ConfigPtr &config)
{
    const KScreen::OutputList connectedOutputs = config->connectedOutputs();

    ASSERT_OUTPUTS(connectedOutputs);
    if (connectedOutputs.isEmpty()) {
        return;
    }

    KScreen::OutputPtr output = connectedOutputs.first();
    if (output->modes().isEmpty()) {
        return;
    }

    config->setPrimaryOutput(output);
    output->setPos(QPoint(0, 0));
}

void Generator::laptop(KScreen::ConfigPtr &config)
{
    KScreen::OutputList usableOutputs = config->connectedOutputs();

    ASSERT_OUTPUTS(usableOutputs)
    if (usableOutputs.isEmpty()) {
        return;
    }

    KScreen::OutputPtr embedded = embeddedOutput(usableOutputs);
    /* Apparently older laptops use "VGA-*" as embedded output ID, so embeddedOutput()
     * will fail, because it looks only for modern "LVDS", "EDP", etc. If we
     * fail to detect which output is embedded, just use the one  with the lowest
     * ID. It's a wild guess, but I think it's highly probable that it will work.
     * See bug #318907 for further reference. -- dvratil
     */
    if (!embedded) {
        QList<int> keys = usableOutputs.keys();
        std::sort(keys.begin(), keys.end());
        embedded = usableOutputs.value(keys.first());
    }
    usableOutputs.remove(embedded->id());

    if (usableOutputs.isEmpty() || embedded->modes().isEmpty()) {
        qCWarning(KSCREEN_KDED) << "No external outputs found, going for singleOutput()";
        return singleOutput(config);
    }

    if (isLidClosed() && usableOutputs.count() == 1) {
        qCDebug(KSCREEN_KDED) << "With lid closed";
        embedded->setEnabled(false);

        KScreen::OutputPtr external = usableOutputs.first();
        if (external->modes().isEmpty()) {
            return;
        }
        config->setPrimaryOutput(external);
        external->setPos(QPoint(0, 0));
        return;
    }

    if (isLidClosed() && usableOutputs.count() > 1) {
        qCDebug(KSCREEN_KDED) << "Lid is closed, and more than one output";
        embedded->setEnabled(false);

        extendToRight(config, usableOutputs);
        return;
    }

    qCDebug(KSCREEN_KDED) << "Lid is open";

    // If lid is open, laptop screen should be primary
    embedded->setPos(QPoint(0, 0));
    embedded->setEnabled(true);
    int globalWidth = embedded->geometry().width();

    KScreen::OutputPtr biggest = biggestOutput(usableOutputs);
    Q_ASSERT(biggest);
    usableOutputs.remove(biggest->id());

    biggest->setPos(QPoint(globalWidth, 0));
    biggest->setEnabled(true);
    globalWidth += biggest->geometry().width();

    for (KScreen::OutputPtr output : qAsConst(usableOutputs)) {
        output->setEnabled(true);
        output->setPos(QPoint(globalWidth, 0));
        globalWidth += output->geometry().width();
    }

    if (isDocked()) {
        qCDebug(KSCREEN_KDED) << "Docked";
        config->setPrimaryOutput(biggest);
    } else {
        config->setPrimaryOutput(embedded);
    }
}

void Generator::extendToRight(KScreen::ConfigPtr &config, KScreen::OutputList usableOutputs)
{
    ASSERT_OUTPUTS(usableOutputs);
    if (usableOutputs.isEmpty()) {
        return;
    }

    qCDebug(KSCREEN_KDED) << "Extending to the right";

    KScreen::OutputPtr biggest = biggestOutput(usableOutputs);
    Q_ASSERT(biggest);
    usableOutputs.remove(biggest->id());

    biggest->setEnabled(true);
    biggest->setPos(QPoint(0, 0));
    int globalWidth = biggest->geometry().width();

    for (KScreen::OutputPtr output : qAsConst(usableOutputs)) {
        output->setEnabled(true);
        output->setPos(QPoint(globalWidth, 0));
        globalWidth += output->geometry().width();
    }

    config->setPrimaryOutput(biggest);
}

void Generator::initializeOutput(const KScreen::OutputPtr &output, KScreen::Config::Features features)
{
    if (output->modes().empty()) {
        output->setEnabled(false);
        return;
    }
    Output::GlobalConfig config = Output::readGlobal(output);
    output->setCurrentModeId(config.modeId.value_or(bestModeForOutput(output)->id()));
    output->setRotation(config.rotation.value_or(output->rotation()));
    if (features & KScreen::Config::Feature::PerOutputScaling) {
        output->setScale(config.scale.value_or(bestScaleForOutput(output)));
    }
}

KScreen::ModePtr Generator::biggestMode(const KScreen::ModeList &modes)
{
    Q_ASSERT(!modes.isEmpty());

    int modeArea, biggestArea = 0;
    KScreen::ModePtr biggestMode;
    for (const KScreen::ModePtr &mode : modes) {
        modeArea = mode->size().width() * mode->size().height();
        if (modeArea < biggestArea) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() < biggestMode->refreshRate()) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() > biggestMode->refreshRate()) {
            biggestMode = mode;
            continue;
        }

        biggestArea = modeArea;
        biggestMode = mode;
    }

    return biggestMode;
}

KScreen::ModePtr Generator::bestModeForSize(const KScreen::ModeList &modes, const QSize &size)
{
    KScreen::ModePtr bestMode;
    for (const KScreen::ModePtr &mode : modes) {
        if (mode->size() != size) {
            continue;
        }

        if (!bestMode) {
            bestMode = mode;
            continue;
        }

        if (mode->refreshRate() > bestMode->refreshRate()) {
            bestMode = mode;
        }
    }

    return bestMode;
}

qreal Generator::bestScaleForOutput(const KScreen::OutputPtr &output)
{
    // Sanity check outputs that tell us they have no physical size
    if (output->sizeMm().height() <= 0) {
        return 1.0;
    }

    /* The eye's ability to perceive detail diminishes with distance, so objects
     * that are closer can be smaller and their details remain equally
     * distinguishable. As a result, each device type has its own ideal physical
     * size of items on its screen based on how close the user's eyes are
     * expected to be from it on average, and its target DPI value needs to be
     * changed accordingly.
     */
    int outputTargetDpi;

    if (output->type() != KScreen::Output::Panel) {
        outputTargetDpi = targetDpiDesktop;
    } else {
        if (isLaptop()) {
            outputTargetDpi = targetDpiLaptop;
        } else {
            outputTargetDpi = targetDpiMobile;
        }
    }

    const qreal outputPixelHeight = output->currentMode()->size().height();
    const qreal outputPhysicalHeight = output->sizeMm().height() / 25.4; // convert mm to inches
    const qreal outputDpi = outputPixelHeight / outputPhysicalHeight;

    const qreal scale = round(outputDpi / outputTargetDpi * scaleRoundingness) / scaleRoundingness;

    // Sanity check for outputs with such a low pixel density that the calculated
    // scale would be less than 1; this isn't well supported, so just use 1
    if (scale < 1) {
        return 1.0;
    }
    // The KCM doesn't support manually setting the scale higher than 3x, so limit
    // the auto-calculated value to that
    else if (scale > 3) {
        return 3.0;
    }

    return scale;
}

KScreen::ModePtr Generator::bestModeForOutput(const KScreen::OutputPtr &output)
{
    if (KScreen::ModePtr outputMode = output->preferredMode()) {
        return outputMode;
    }

    return biggestMode(output->modes());
}

KScreen::OutputPtr Generator::biggestOutput(const KScreen::OutputList &outputs)
{
    ASSERT_OUTPUTS(outputs)

    int area, total = 0;
    KScreen::OutputPtr biggest;
    for (const KScreen::OutputPtr &output : outputs) {
        const KScreen::ModePtr mode = bestModeForOutput(output);
        if (!mode) {
            continue;
        }
        area = mode->size().width() * mode->size().height();
        if (area <= total) {
            continue;
        }

        total = area;
        biggest = output;
    }

    return biggest;
}

void Generator::disableAllDisconnectedOutputs(const KScreen::OutputList &outputs)
{
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected()) {
            qCDebug(KSCREEN_KDED) << output->name() << " Disabled";
            output->setEnabled(false);
        }
    }
}

KScreen::OutputPtr Generator::embeddedOutput(const KScreen::OutputList &outputs)
{
    for (const KScreen::OutputPtr &output : outputs) {
        if (output->type() == KScreen::Output::Panel) {
            return output;
        }
    }

    return KScreen::OutputPtr();
}

bool Generator::isLaptop() const
{
    if (m_forceLaptop) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLaptop();
}

bool Generator::isLidClosed() const
{
    if (m_forceLidClosed) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLidClosed();
}

bool Generator::isDocked() const
{
    return m_forceDocked;
}

void Generator::setForceLaptop(bool force)
{
    m_forceLaptop = force;
}

void Generator::setForceLidClosed(bool force)
{
    m_forceLidClosed = force;
}

void Generator::setForceDocked(bool force)
{
    m_forceDocked = force;
}

void Generator::setForceNotLaptop(bool force)
{
    m_forceNotLaptop = force;
}
