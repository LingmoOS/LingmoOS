/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012-2015 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "xrandrconfig.h"

#include "xrandr.h"
#include "xrandrmode.h"
#include "xrandrscreen.h"

#include "../xcbwrapper.h"

#include "config.h"
#include "edid.h"
#include "mode.h"
#include "output.h"
#include "screen.h"

#include <QRect>
#include <QScopedPointer>
#include <QtGui/private/qtx11extras_p.h>

#include <optional>
#include <utility>

using namespace KScreen;

XRandRConfig::XRandRConfig()
    : QObject()
    , m_screen(nullptr)
{
    m_screen = new XRandRScreen(this);

    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> resources(XRandR::screenResources());

    xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(resources.data());
    const int crtcsCount = xcb_randr_get_screen_resources_crtcs_length(resources.data());
    for (int i = 0; i < crtcsCount; ++i) {
        addNewCrtc(crtcs[i]);
    }

    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(resources.data());
    const int outputsCount = xcb_randr_get_screen_resources_outputs_length(resources.data());
    for (int i = 0; i < outputsCount; ++i) {
        addNewOutput(outputs[i]);
    }
}

XRandRConfig::~XRandRConfig()
{
    qDeleteAll(m_outputs);
    qDeleteAll(m_crtcs);
    delete m_screen;
}

XRandROutput::Map XRandRConfig::outputs() const
{
    return m_outputs;
}

XRandROutput *XRandRConfig::output(xcb_randr_output_t output) const
{
    return m_outputs[output];
}

XRandRCrtc::Map XRandRConfig::crtcs() const
{
    return m_crtcs;
}

XRandRCrtc *XRandRConfig::crtc(xcb_randr_crtc_t crtc) const
{
    return m_crtcs[crtc];
}

XRandRScreen *XRandRConfig::screen() const
{
    return m_screen;
}

void XRandRConfig::addNewOutput(xcb_randr_output_t id)
{
    XRandROutput *xOutput = new XRandROutput(id, this);
    m_outputs.insert(id, xOutput);
}

void XRandRConfig::addNewCrtc(xcb_randr_crtc_t crtc)
{
    m_crtcs.insert(crtc, new XRandRCrtc(crtc, this));
}

void XRandRConfig::removeOutput(xcb_randr_output_t id)
{
    delete m_outputs.take(id);
}

KScreen::ConfigPtr XRandRConfig::toKScreenConfig() const
{
    KScreen::ConfigPtr config(new KScreen::Config);

    const Config::Features features = Config::Feature::Writable | Config::Feature::PrimaryDisplay | Config::Feature::OutputReplication;
    config->setSupportedFeatures(features);

    KScreen::OutputList kscreenOutputs;

    for (const XRandROutput *output : std::as_const(m_outputs)) {
        KScreen::OutputPtr kscreenOutput = output->toKScreenOutput();
        kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
    }

    config->setOutputs(kscreenOutputs);
    config->setScreen(m_screen->toKScreenScreen());

    return config;
}

void XRandRConfig::applyKScreenConfig(const KScreen::ConfigPtr &config)
{
    config->adjustPriorities(); // never trust input

    const KScreen::OutputList kscreenOutputs = config->outputs();

    const QSize newScreenSize = screenSize(config);
    const QSize currentScreenSize = m_screen->currentSize();

    // When the current screen configuration is bigger than the new size (like
    // when rotating an output), the XSetScreenSize can fail or apply the smaller
    // size only partially, because we apply the size (we have to) before the
    // output changes. To prevent all kinds of weird screen sizes from happening,
    // we initially set such screen size, that it can take the current as well
    // as the new configuration, then we apply the output changes, and finally then
    // (if necessary) we reduce the screen size to fix the new configuration precisely.
    const QSize intermediateScreenSize =
        QSize(qMax(newScreenSize.width(), currentScreenSize.width()), qMax(newScreenSize.height(), currentScreenSize.height()));

    int neededCrtcs = 0;

    // pairs of before/after
    QMap<xcb_randr_output_t, std::pair<std::optional<uint32_t>, std::optional<uint32_t>>> prioritiesChange;
    for (const XRandROutput *xrandrOutput : std::as_const(m_outputs)) {
        prioritiesChange[xrandrOutput->id()].first = std::optional(xrandrOutput->priority());
    }
    for (const KScreen::OutputPtr &kscreenOutput : kscreenOutputs) {
        prioritiesChange[kscreenOutput->id()].second = std::optional(kscreenOutput->priority());
    }
    const bool prioritiesDiffer = std::any_of(prioritiesChange.cbegin(), prioritiesChange.cend(), [](const auto &pair) {
        const auto &[before, after] = pair;
        return !before.has_value() || !after.has_value() || before.value() != after.value();
    });

    KScreen::OutputList toDisable, toEnable, toChange;

    for (const KScreen::OutputPtr &kscreenOutput : kscreenOutputs) {
        xcb_randr_output_t outputId = kscreenOutput->id();
        XRandROutput *currentOutput = output(outputId);

        const bool currentEnabled = currentOutput->isEnabled();

        if (!kscreenOutput->isEnabled() && currentEnabled) {
            toDisable.insert(outputId, kscreenOutput);
            continue;
        } else if (kscreenOutput->isEnabled() && !currentEnabled) {
            toEnable.insert(outputId, kscreenOutput);
            ++neededCrtcs;
            continue;
        } else if (!kscreenOutput->isEnabled() && !currentEnabled) {
            continue;
        }

        ++neededCrtcs;

        if (kscreenOutput->currentModeId() != currentOutput->currentModeId()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        if (kscreenOutput->pos() != currentOutput->position()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        if (kscreenOutput->rotation() != currentOutput->rotation()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        if (kscreenOutput->explicitLogicalSize() != currentOutput->logicalSize()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        XRandRMode *currentMode = currentOutput->modes().value(kscreenOutput->currentModeId().toInt());
        // For some reason, in some environments currentMode is null
        // which doesn't make sense because it is the *current* mode...
        // Since we haven't been able to figure out the reason why
        // this happens, we are adding this debug code to try to
        // figure out how this happened.
        if (!currentMode) {
            qWarning() << "Current mode is null:"
                       << "ModeId:" << currentOutput->currentModeId() << "Mode: " << currentOutput->currentMode() << "Output: " << currentOutput->id();
            printConfig(config);
            printInternalCond();
            continue;
        }

        // When the output would not fit into new screen size, we need to disable and reposition it.
        const QRect geom = kscreenOutput->geometry();
        if (geom.right() > newScreenSize.width() || geom.bottom() > newScreenSize.height()) {
            if (!toDisable.contains(outputId)) {
                qCDebug(KSCREEN_XRANDR) << "The new output would not fit into screen - new geometry: " << geom << ", new screen size:" << newScreenSize;
                toDisable.insert(outputId, kscreenOutput);
            }
        }
    }

    const KScreen::ScreenPtr kscreenScreen = config->screen();
    if (newScreenSize.width() > kscreenScreen->maxSize().width() || newScreenSize.height() > kscreenScreen->maxSize().height()) {
        qCDebug(KSCREEN_XRANDR) << "The new screen size is too big - requested: " << newScreenSize << ", maximum: " << kscreenScreen->maxSize();
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "Needed CRTCs: " << neededCrtcs;

    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> screenResources(XRandR::screenResources());

    if (neededCrtcs > screenResources->num_crtcs) {
        qCDebug(KSCREEN_XRANDR) << "We need more CRTCs than we have available - requested: " << neededCrtcs << ", available: " << screenResources->num_crtcs;
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "Actions to perform:\n"
                            << "\tPriorities:" << prioritiesDiffer;
    for (auto it = prioritiesChange.constBegin(); it != prioritiesChange.constEnd(); it++) {
        const auto &[before, after] = it.value();
        if (before != after) {
            qCDebug(KSCREEN_XRANDR) << "\tOutput" << it.key() << "\n"
                                    << "\t\tOld:" << (before.has_value() ? QString::number(before.value()) : QStringLiteral("none")) << "\n"
                                    << "\t\tNew:" << (after.has_value() ? QString::number(after.value()) : QStringLiteral("none"));
        }
    }

    qCDebug(KSCREEN_XRANDR) << "\tChange Screen Size:" << (newScreenSize != currentScreenSize);
    if (newScreenSize != currentScreenSize) {
        qCDebug(KSCREEN_XRANDR) << "\t\tOld:" << currentScreenSize << "\n"
                                << "\t\tIntermediate:" << intermediateScreenSize << "\n"
                                << "\t\tNew:" << newScreenSize;
    }

    qCDebug(KSCREEN_XRANDR) << "\tDisable outputs:" << !toDisable.isEmpty();
    if (!toDisable.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toDisable.keys();
    }

    qCDebug(KSCREEN_XRANDR) << "\tChange outputs:" << !toChange.isEmpty();
    if (!toChange.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toChange.keys();
    }

    qCDebug(KSCREEN_XRANDR) << "\tEnable outputs:" << !toEnable.isEmpty();
    if (!toEnable.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toEnable.keys();
    }

    // Grab the server so that no-one else can do changes to XRandR and to block
    // change notifications until we are done
    XCB::GrabServer grabber;

    // If there is nothing to do, not even bother
    if (!prioritiesDiffer && toDisable.isEmpty() && toEnable.isEmpty() && toChange.isEmpty()) {
        if (newScreenSize != currentScreenSize) {
            setScreenSize(newScreenSize);
        }
        return;
    }

    for (const KScreen::OutputPtr &output : toDisable) {
        disableOutput(output);
    }

    if (intermediateScreenSize != currentScreenSize) {
        setScreenSize(intermediateScreenSize);
    }

    bool forceScreenSizeUpdate = false;

    for (const KScreen::OutputPtr &output : toChange) {
        if (!changeOutput(output)) {
            /* If we disabled the output before changing it and XRandR failed
             * to re-enable it, then update screen size too */
            if (toDisable.contains(output->id())) {
                output->setEnabled(false);
                qCDebug(KSCREEN_XRANDR) << "Output failed to change: " << output->name();
                forceScreenSizeUpdate = true;
            }
        }
    }

    for (const KScreen::OutputPtr &output : toEnable) {
        if (!enableOutput(output)) {
            qCDebug(KSCREEN_XRANDR) << "Output failed to be Enabled: " << output->name();
            forceScreenSizeUpdate = true;
        }
    }

    for (auto it = prioritiesChange.constBegin(); it != prioritiesChange.constEnd(); it++) {
        const xcb_randr_output_t outputId = it.key();
        const auto &[before, after] = it.value();
        const uint32_t priority = after.value_or(0);
        setOutputPriority(outputId, priority);
    }

    if (forceScreenSizeUpdate || intermediateScreenSize != newScreenSize) {
        QSize newSize = newScreenSize;
        if (forceScreenSizeUpdate) {
            newSize = screenSize(config);
            qCDebug(KSCREEN_XRANDR) << "Forced to change screen size: " << newSize;
        }
        setScreenSize(newSize);
    }
}

void XRandRConfig::printConfig(const ConfigPtr &config) const
{
    qCDebug(KSCREEN_XRANDR) << "KScreen version:" /*<< LIBKSCREEN_VERSION*/;

    if (!config) {
        qCDebug(KSCREEN_XRANDR) << "Config is invalid";
        return;
    }
    if (!config->screen()) {
        qCDebug(KSCREEN_XRANDR) << "No screen in the configuration, broken backend";
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "Screen:"
                            << "\n"
                            << "\tmaxSize:" << config->screen()->maxSize() << "\n"
                            << "\tminSize:" << config->screen()->minSize() << "\n"
                            << "\tcurrentSize:" << config->screen()->currentSize();

    const OutputList outputs = config->outputs();
    for (const OutputPtr &output : outputs) {
        qCDebug(KSCREEN_XRANDR) << "\n-----------------------------------------------------\n"
                                << "\n"
                                << "Id: " << output->id() << "\n"
                                << "Name: " << output->name() << "\n"
                                << "Type: " << output->type() << "\n"
                                << "Connected: " << output->isConnected();

        if (!output->isConnected()) {
            continue;
        }

        qCDebug(KSCREEN_XRANDR) << "Enabled: " << output->isEnabled() << "\n"
                                << "Priority: " << output->priority() << "\n"
                                << "Rotation: " << output->rotation() << "\n"
                                << "Pos: " << output->pos() << "\n"
                                << "MMSize: " << output->sizeMm();
        if (output->currentMode()) {
            qCDebug(KSCREEN_XRANDR) << "Size: " << output->currentMode()->size();
        }

        qCDebug(KSCREEN_XRANDR) << "Clones: " << (output->clones().isEmpty() ? QStringLiteral("None") : QString::number(output->clones().count())) << "\n"
                                << "Mode: " << output->currentModeId() << "\n"
                                << "Preferred Mode: " << output->preferredModeId() << "\n"
                                << "Preferred modes: " << output->preferredModes() << "\n"
                                << "Modes: ";

        ModeList modes = output->modes();
        for (const ModePtr &mode : modes) {
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid *edid = output->edid();
        qCDebug(KSCREEN_XRANDR) << "EDID Info: ";
        if (edid && edid->isValid()) {
            qCDebug(KSCREEN_XRANDR) << "\tDevice ID: " << edid->deviceId() << "\n"
                                    << "\tName: " << edid->name() << "\n"
                                    << "\tVendor: " << edid->vendor() << "\n"
                                    << "\tSerial: " << edid->serial() << "\n"
                                    << "\tEISA ID: " << edid->eisaId() << "\n"
                                    << "\tHash: " << edid->hash() << "\n"
                                    << "\tWidth: " << edid->width() << "\n"
                                    << "\tHeight: " << edid->height() << "\n"
                                    << "\tGamma: " << edid->gamma() << "\n"
                                    << "\tRed: " << edid->red() << "\n"
                                    << "\tGreen: " << edid->green() << "\n"
                                    << "\tBlue: " << edid->blue() << "\n"
                                    << "\tWhite: " << edid->white();
        } else {
            qCDebug(KSCREEN_XRANDR) << "\tUnavailable";
        }
    }
}

void XRandRConfig::printInternalCond() const
{
    qCDebug(KSCREEN_XRANDR) << "Internal config in xrandr";
    for (const XRandROutput *output : m_outputs) {
        qCDebug(KSCREEN_XRANDR) << "Id: " << output->id() << "\n"
                                << "Current Mode: " << output->currentMode() << "\n"
                                << "Current mode id: " << output->currentModeId() << "\n"
                                << "Connected: " << output->isConnected() << "\n"
                                << "Enabled: " << output->isEnabled() << "\n"
                                << "Priority: " << output->priority();
        if (!output->isEnabled()) {
            continue;
        }

        XRandRMode::Map modes = output->modes();
        for (const XRandRMode *mode : modes) {
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->id() << "\n"
                                    << "\t" << mode->name() << "\n"
                                    << "\t" << mode->size() << mode->refreshRate();
        }
    }
}

QSize XRandRConfig::screenSize(const KScreen::ConfigPtr &config) const
{
    QRect rect;
    for (const KScreen::OutputPtr &output : config->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }

        const ModePtr currentMode = output->currentMode();
        if (!currentMode) {
            qCDebug(KSCREEN_XRANDR) << "Output: " << output->name() << " has no current Mode!";
            continue;
        }

        const QRect outputGeom = output->geometry();
        rect = rect.united(outputGeom);
    }

    const QSize size = QSize(rect.width(), rect.height());
    qCDebug(KSCREEN_XRANDR) << "Requested screen size is" << size;
    return size;
}

bool XRandRConfig::setScreenSize(const QSize &size) const
{
    const double dpi = 25.4 * XRandR::screen()->height_in_pixels / XRandR::screen()->height_in_millimeters;
    const int widthMM = (25.4 * size.width()) / dpi;
    const int heightMM = (25.4 * size.height()) / dpi;

    qCDebug(KSCREEN_XRANDR) << "RRSetScreenSize"
                            << "\n"
                            << "\tDPI:" << dpi << "\n"
                            << "\tSize:" << size << "\n"
                            << "\tSizeMM:" << QSize(widthMM, heightMM);

    xcb_randr_set_screen_size(XCB::connection(), XRandR::rootWindow(), size.width(), size.height(), widthMM, heightMM);
    m_screen->update(size);
    return true;
}

void XRandRConfig::setOutputPriority(xcb_randr_output_t outputId, uint32_t priority) const
{
    qCDebug(KSCREEN_XRANDR) << "RRSetOutputPrimary"
                            << "\n"
                            << "\tNew priority:" << priority;

    if (m_outputs.contains(outputId)) {
        m_outputs[outputId]->setPriority(priority);
    }
}

bool XRandRConfig::disableOutput(const OutputPtr &kscreenOutput) const
{
    XRandROutput *xOutput = output(kscreenOutput->id());
    Q_ASSERT(xOutput);
    Q_ASSERT(xOutput->crtc());

    if (!xOutput->crtc()) {
        qCWarning(KSCREEN_XRANDR) << "Attempting to disable output without CRTC, wth?";
        return false;
    }

    const xcb_randr_crtc_t crtc = xOutput->crtc()->crtc();

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (disable output)"
                            << "\n"
                            << "\tCRTC:" << crtc;

    xOutput->setPriority(0);

    auto cookie = xcb_randr_set_crtc_config(XCB::connection(), //
                                            crtc,
                                            XCB_CURRENT_TIME,
                                            XCB_CURRENT_TIME,
                                            0,
                                            0,
                                            XCB_NONE,
                                            XCB_RANDR_ROTATION_ROTATE_0,
                                            0,
                                            nullptr);

    XCB::ScopedPointer<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(XCB::connection(), cookie, nullptr));

    if (!reply) {
        qCDebug(KSCREEN_XRANDR) << "\tResult: unknown (error)";
        return false;
    }
    qCDebug(KSCREEN_XRANDR) << "\tResult:" << reply->status;

    // Update the cached output now, otherwise we get RRNotify_CrtcChange notification
    // for an outdated output, which can lead to a crash.
    if (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS) {
        xOutput->update(XCB_NONE, XCB_NONE, xOutput->isConnected() ? XCB_RANDR_CONNECTION_CONNECTED : XCB_RANDR_CONNECTION_DISCONNECTED);
        if (xOutput->crtc())
            xOutput->crtc()->updateTimestamp(reply->timestamp);
    }
    return (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
}

bool XRandRConfig::enableOutput(const OutputPtr &kscreenOutput) const
{
    XRandRCrtc *freeCrtc = nullptr;
    qCDebug(KSCREEN_XRANDR) << m_crtcs;

    for (XRandRCrtc *crtc : m_crtcs) {
        crtc->update();
        qCDebug(KSCREEN_XRANDR) << "Testing CRTC" << crtc->crtc() << "\n"
                                << "\tFree:" << crtc->isFree() << "\n"
                                << "\tMode:" << crtc->mode() << "\n"
                                << "\tPossible outputs:" << crtc->possibleOutputs() << "\n"
                                << "\tConnected outputs:" << crtc->outputs() << "\n"
                                << "\tGeometry:" << crtc->geometry();

        if (crtc->isFree() && crtc->possibleOutputs().contains(kscreenOutput->id())) {
            freeCrtc = crtc;
            break;
        }
    }

    if (!freeCrtc) {
        qCWarning(KSCREEN_XRANDR) << "Failed to get free CRTC for output" << kscreenOutput->id();
        return false;
    }

    XRandROutput *xOutput = output(kscreenOutput->id());
    const int modeId = kscreenOutput->currentMode() ? kscreenOutput->currentModeId().toInt() : kscreenOutput->preferredModeId().toInt();
    xOutput->updateLogicalSize(kscreenOutput, freeCrtc);

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (enable output)"
                            << "\n"
                            << "\tOutput:" << kscreenOutput->id() << "(" << kscreenOutput->name() << ")"
                            << "\n"
                            << "\tNew CRTC:" << freeCrtc->crtc() << "\n"
                            << "\tPos:" << kscreenOutput->pos() << "\n"
                            << "\tMode:" << kscreenOutput->currentMode() << "Preferred:" << kscreenOutput->preferredModeId() << "\n"
                            << "\tRotation:" << kscreenOutput->rotation();

    if (!sendConfig(kscreenOutput, freeCrtc)) {
        return false;
    }

    xOutput->update(freeCrtc->crtc(), modeId, XCB_RANDR_CONNECTION_CONNECTED);
    xOutput->setPriority(kscreenOutput->priority());
    return true;
}

bool XRandRConfig::changeOutput(const KScreen::OutputPtr &kscreenOutput) const
{
    XRandROutput *xOutput = output(kscreenOutput->id());
    Q_ASSERT(xOutput);

    if (!xOutput->crtc()) {
        qCDebug(KSCREEN_XRANDR) << "Output" << kscreenOutput->id() << "has no CRTC, falling back to enableOutput()";
        return enableOutput(kscreenOutput);
    }

    int modeId = kscreenOutput->currentMode() ? kscreenOutput->currentModeId().toInt() : kscreenOutput->preferredModeId().toInt();
    xOutput->updateLogicalSize(kscreenOutput);

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (change output)"
                            << "\n"
                            << "\tOutput:" << kscreenOutput->id() << "(" << kscreenOutput->name() << ")"
                            << "\n"
                            << "\tCRTC:" << xOutput->crtc()->crtc() << "\n"
                            << "\tPos:" << kscreenOutput->pos() << "\n"
                            << "\tMode:" << kscreenOutput->currentMode() << "Preferred:" << kscreenOutput->preferredModeId() << "\n"
                            << "\tRotation:" << kscreenOutput->rotation();

    if (!sendConfig(kscreenOutput, xOutput->crtc())) {
        return false;
    }

    xOutput->update(xOutput->crtc()->crtc(), modeId, XCB_RANDR_CONNECTION_CONNECTED);
    xOutput->setPriority(kscreenOutput->priority());
    return true;
}

bool XRandRConfig::sendConfig(const KScreen::OutputPtr &kscreenOutput, XRandRCrtc *crtc) const
{
    xcb_randr_output_t outputs[1]{static_cast<xcb_randr_output_t>(kscreenOutput->id())};
    const int modeId = kscreenOutput->currentMode() ? kscreenOutput->currentModeId().toInt() : kscreenOutput->preferredModeId().toInt();

    auto cookie = xcb_randr_set_crtc_config(XCB::connection(),
                                            crtc->crtc(),
                                            XCB_CURRENT_TIME,
                                            XCB_CURRENT_TIME,
                                            kscreenOutput->pos().rx(),
                                            kscreenOutput->pos().ry(),
                                            modeId,
                                            kscreenOutput->rotation(),
                                            1,
                                            outputs);

    XCB::ScopedPointer<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(XCB::connection(), cookie, nullptr));
    if (!reply) {
        qCDebug(KSCREEN_XRANDR) << "\tResult: unknown (error)";
        return false;
    }

    crtc->updateTimestamp(reply->timestamp);

    qCDebug(KSCREEN_XRANDR) << "\tResult: " << reply->status << " timestamp: " << reply->timestamp;
    return (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
}

#include "moc_xrandrconfig.cpp"
