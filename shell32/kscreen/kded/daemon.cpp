/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "daemon.h"

#include "../common/orientation_sensor.h"
#include "config.h"
#include "device.h"
#include "generator.h"
#include "kscreen_daemon_debug.h"
#include "kscreenadaptor.h"
#include "osdservice_interface.h"

#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/log.h>
#include <kscreen/mode.h>
#include <kscreen/output.h>
#include <kscreen/screen.h>
#include <kscreen/setconfigoperation.h>
#include <kscreendpms/dpms.h>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QAction>
#include <QGuiApplication>
#include <QOrientationReading>
#include <QScreen>
#include <QShortcut>
#include <QTimer>

#if HAVE_X11
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <private/qtx11extras_p.h>
#endif
#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#endif

K_PLUGIN_CLASS_WITH_JSON(KScreenDaemon, "kscreen.json")

#if HAVE_X11
struct DeviceListDeleter {
    void operator()(XDeviceInfo *p)
    {
        if (p) {
            XFreeDeviceList(p);
        }
    }
};

struct XDeleter {
    void operator()(void *p)
    {
        if (p) {
            XFree(p);
        }
    }
};
#endif

KScreenDaemon::KScreenDaemon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_monitoring(false)
    , m_changeCompressor(new QTimer(this))
    , m_saveTimer(nullptr)
    , m_lidClosedTimer(new QTimer(this))
    , m_orientationSensor(new OrientationSensor(this))
{
    connect(m_orientationSensor, &OrientationSensor::availableChanged, this, &KScreenDaemon::updateOrientation);
    connect(m_orientationSensor, &OrientationSensor::valueChanged, this, &KScreenDaemon::updateOrientation);

    KScreen::Log::instance();
    qMetaTypeId<KScreen::OsdAction>();
    QMetaObject::invokeMethod(this, "getInitialConfig", Qt::QueuedConnection);

    auto dpms = new KScreen::Dpms(this);
    connect(dpms, &KScreen::Dpms::modeChanged, this, [this](KScreen::Dpms::Mode mode, QScreen *screen) {
        if (m_monitoredConfig && m_monitoredConfig->data() && screen->geometry() == m_monitoredConfig->data()->primaryOutput()->geometry()) {
            if (mode == KScreen::Dpms::On) {
                m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());
            } else {
                m_orientationSensor->setEnabled(false);
            }
        }
    });
}

void KScreenDaemon::getInitialConfig()
{
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
        if (op->hasError()) {
            qCDebug(KSCREEN_KDED) << "Error getting initial configuration" << op->errorString();
            return;
        }

        m_monitoredConfig = std::unique_ptr<Config>(new Config(qobject_cast<KScreen::GetConfigOperation *>(op)->config()));
        m_monitoredConfig->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
        qCDebug(KSCREEN_KDED) << "Config" << m_monitoredConfig->data().data() << "is ready";
        KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig->data());

        init();
    });
}

KScreenDaemon::~KScreenDaemon()
{
    Generator::destroy();
    Device::destroy();
}

void KScreenDaemon::init()
{
    KActionCollection *coll = new KActionCollection(this);
    QAction *action = coll->addAction(QStringLiteral("display"));
    action->setText(i18n("Switch Display"));
    QList<QKeySequence> switchDisplayShortcuts({Qt::Key_Display, Qt::MetaModifier + Qt::Key_P});
    KGlobalAccel::self()->setGlobalShortcut(action, switchDisplayShortcuts);
    connect(action, &QAction::triggered, this, &KScreenDaemon::displayButton);

    new KScreenAdaptor(this);

    const QString osdService = QStringLiteral("org.kde.kscreen.osdService");
    const QString osdPath = QStringLiteral("/org/kde/kscreen/osdService");
    m_osdServiceInterface = new OrgKdeKscreenOsdServiceInterface(osdService, osdPath, QDBusConnection::sessionBus(), this);
    // Set a longer timeout to not assume timeout while the osd is still shown
    m_osdServiceInterface->setTimeout(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(60)).count());

    m_changeCompressor->setInterval(10);
    m_changeCompressor->setSingleShot(true);
    connect(m_changeCompressor, &QTimer::timeout, this, &KScreenDaemon::applyConfig);

    m_lidClosedTimer->setInterval(1000);
    m_lidClosedTimer->setSingleShot(true);
    connect(m_lidClosedTimer, &QTimer::timeout, this, &KScreenDaemon::disableLidOutput);

    connect(Device::self(), &Device::lidClosedChanged, this, &KScreenDaemon::lidClosedChanged);
    connect(Device::self(), &Device::resumingFromSuspend, this, [this]() {
        KScreen::Log::instance()->setContext(QStringLiteral("resuming"));
        m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());
        qCDebug(KSCREEN_KDED) << "Resumed from suspend, checking for screen changes";
        // We don't care about the result, we just want to force the backend
        // to query XRandR so that it will detect possible changes that happened
        // while the computer was suspended, and will emit the change events.
        new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID, this);
    });
    connect(Device::self(), &Device::aboutToSuspend, this, [this]() {
        qCDebug(KSCREEN_KDED) << "System is going to suspend, won't be changing config (waited for "
                              << (m_lidClosedTimer->interval() - m_lidClosedTimer->remainingTime()) << "ms)";
        m_lidClosedTimer->stop();
        m_orientationSensor->setEnabled(false);
    });

    connect(Generator::self(), &Generator::ready, this, [this] {
        applyConfig();

        if (Device::self()->isLaptop() && Device::self()->isLidClosed()) {
            disableLidOutput();
        }

        m_startingUp = false;
    });

    Generator::self()->setCurrentConfig(m_monitoredConfig->data());
    monitorConnectedChange();
}

void KScreenDaemon::updateOrientation()
{
    if (!m_monitoredConfig) {
        return;
    }
    const auto features = m_monitoredConfig->data()->supportedFeatures();
    if (!features.testFlag(KScreen::Config::Feature::AutoRotation) || !features.testFlag(KScreen::Config::Feature::TabletMode)) {
        return;
    }

    if (!m_orientationSensor->available() || !m_orientationSensor->enabled()) {
        return;
    }

    const auto orientation = m_orientationSensor->value();
    if (orientation == QOrientationReading::Undefined) {
        // Orientation sensor went off. Do not change current orientation.
        return;
    }
    if (orientation == QOrientationReading::FaceUp || orientation == QOrientationReading::FaceDown) {
        // We currently don't do anything with FaceUp/FaceDown, but in the future we could use them
        // to shut off and switch on again a display when display is facing downwards/upwards.
        return;
    }

    m_monitoredConfig->setDeviceOrientation(orientation);
    if (m_monitoring) {
        doApplyConfig(m_monitoredConfig->data());
    } else {
        m_configDirty = true;
    }
}

void KScreenDaemon::doApplyConfig(const KScreen::ConfigPtr &config)
{
    qCDebug(KSCREEN_KDED) << "Do set and apply specific config";
    auto configWrapper = std::unique_ptr<Config>(new Config(config));
    configWrapper->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);

    doApplyConfig(std::move(configWrapper));
}

void KScreenDaemon::doApplyConfig(std::unique_ptr<Config> config)
{
    m_monitoredConfig = std::move(config);

    m_monitoredConfig->activateControlWatching();
    m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());

    connect(m_monitoredConfig.get(), &Config::controlChanged, this, [this]() {
        m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());
        updateOrientation();
    });

    refreshConfig();
}

void KScreenDaemon::refreshConfig()
{
    setMonitorForChanges(false);
    m_configDirty = false;
    KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig->data());

    connect(new KScreen::SetConfigOperation(m_monitoredConfig->data()), &KScreen::SetConfigOperation::finished, this, [this]() {
        qCDebug(KSCREEN_KDED) << "Config applied";
        if (m_configDirty) {
            // Config changed in the meantime again, apply.
            doApplyConfig(m_monitoredConfig->data());
        } else {
            setMonitorForChanges(true);
        }
    });
}

void KScreenDaemon::applyConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying config";
    if (m_monitoredConfig->fileExists()) {
        applyKnownConfig();
        return;
    }
    applyIdealConfig();
}

void KScreenDaemon::applyKnownConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying known config";

    std::unique_ptr<Config> readInConfig = m_monitoredConfig->readFile();
    if (readInConfig) {
        doApplyConfig(std::move(readInConfig));
    } else {
        qCDebug(KSCREEN_KDED) << "Loading failed, falling back to the ideal config" << m_monitoredConfig->id();
        applyIdealConfig();
    }
}

void KScreenDaemon::applyLayoutPreset(const QString &presetName)
{
    const QMetaEnum actionEnum = QMetaEnum::fromType<KScreen::OsdAction::Action>();
    Q_ASSERT(actionEnum.isValid());

    bool ok;
    auto action = static_cast<KScreen::OsdAction::Action>(actionEnum.keyToValue(qPrintable(presetName), &ok));
    if (!ok) {
        qCWarning(KSCREEN_KDED) << "Cannot apply unknown screen layout preset named" << presetName;
        return;
    }
    applyOsdAction(action);
}

bool KScreenDaemon::getAutoRotate()
{
    return m_monitoredConfig->getAutoRotate();
}

void KScreenDaemon::setAutoRotate(bool value)
{
    if (!m_monitoredConfig) {
        return;
    }
    m_monitoredConfig->setAutoRotate(value);
    m_orientationSensor->setEnabled(value);
}

bool KScreenDaemon::isAutoRotateAvailable()
{
    return m_orientationSensor->available();
}

void KScreenDaemon::showOSD()
{
    auto call = m_osdServiceInterface->showActionSelector();
    auto watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        watcher->deleteLater();
        QDBusReply<int> reply = *watcher;
        if (!reply.isValid()) {
            return;
        }
        applyOsdAction(static_cast<KScreen::OsdAction::Action>(reply.value()));
    });
}

void KScreenDaemon::applyOsdAction(KScreen::OsdAction::Action action)
{
    switch (action) {
    case KScreen::OsdAction::NoAction:
        qCDebug(KSCREEN_KDED) << "OSD: no action";
        return;
    case KScreen::OsdAction::SwitchToInternal:
        qCDebug(KSCREEN_KDED) << "OSD: switch to internal";
        doApplyConfig(Generator::self()->displaySwitch(Generator::TurnOffExternal));
        return;
    case KScreen::OsdAction::SwitchToExternal:
        qCDebug(KSCREEN_KDED) << "OSD: switch to external";
        doApplyConfig(Generator::self()->displaySwitch(Generator::TurnOffEmbedded));
        return;
    case KScreen::OsdAction::ExtendLeft:
        qCDebug(KSCREEN_KDED) << "OSD: extend left";
        doApplyConfig(Generator::self()->displaySwitch(Generator::ExtendToLeft));
        return;
    case KScreen::OsdAction::ExtendRight:
        qCDebug(KSCREEN_KDED) << "OSD: extend right";
        doApplyConfig(Generator::self()->displaySwitch(Generator::ExtendToRight));
        return;
    case KScreen::OsdAction::Clone:
        qCDebug(KSCREEN_KDED) << "OSD: clone";
        doApplyConfig(Generator::self()->displaySwitch(Generator::Clone));
        return;
    }
    Q_UNREACHABLE();
}

void KScreenDaemon::applyIdealConfig()
{
    const bool showOsd = m_monitoredConfig->data()->connectedOutputs().count() > 1 && !m_startingUp;

    doApplyConfig(Generator::self()->idealConfig(m_monitoredConfig->data()));

    if (showOsd) {
        qCDebug(KSCREEN_KDED) << "Getting ideal config from user via OSD...";
        showOSD();
    } else {
        m_osdServiceInterface->hideOsd();
    }
}

void KScreenDaemon::configChanged()
{
    qCDebug(KSCREEN_KDED) << "Change detected";
    m_monitoredConfig->log();

    // Modes may have changed, fix-up current mode id
    bool changed = false;
    const auto outputs = m_monitoredConfig->data()->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (output->isConnected() && output->isEnabled()
            && (output->currentMode().isNull() || (output->followPreferredMode() && output->currentModeId() != output->preferredModeId()))) {
            qCDebug(KSCREEN_KDED) << "Current mode was" << output->currentModeId() << ", setting preferred mode" << output->preferredModeId();
            output->setCurrentModeId(output->preferredModeId());
            changed = true;
        }
    }
    if (changed) {
        refreshConfig();
    }

    // Reset timer, delay the writeback
    if (!m_saveTimer) {
        m_saveTimer = new QTimer(this);
        m_saveTimer->setInterval(300);
        m_saveTimer->setSingleShot(true);
        connect(m_saveTimer, &QTimer::timeout, this, &KScreenDaemon::saveCurrentConfig);
    }
    m_saveTimer->start();
#if HAVE_X11
    alignX11TouchScreen();
#endif
}

#if HAVE_X11
void KScreenDaemon::alignX11TouchScreen()
{
    if (qGuiApp->platformName() != QStringLiteral("xcb")) {
        return;
    }
    auto *display = QX11Info::display();
    if (!display) {
        return;
    }
    auto *connection = QX11Info::connection();
    if (!connection) {
        return;
    }

    const QRect totalRect(QPoint(0, 0), m_monitoredConfig->data()->screen()->currentSize());
    QRect internalOutputRect;
    int touchScreenRotationAngle = 0;

    for (const auto &output : m_monitoredConfig->data()->connectedOutputs()) {
        if (output->isEnabled() && output->type() == KScreen::Output::Panel) {
            internalOutputRect = output->geometry();

            switch (output->rotation()) {
            case KScreen::Output::Left:
                touchScreenRotationAngle = 90;
                break;
            case KScreen::Output::Right:
                touchScreenRotationAngle = 270;
                break;
            case KScreen::Output::Inverted:
                touchScreenRotationAngle = 180;
                break;
            default:
                touchScreenRotationAngle = 0;
            }
        }
    }

    // Compute the transformation matrix for the
    QTransform transform;
    transform = transform.translate(float(internalOutputRect.x()) / float(totalRect.width()), float(internalOutputRect.y()) / float(totalRect.height()));
    transform = transform.scale(float(internalOutputRect.width()) / float(totalRect.width()), float(internalOutputRect.height()) / float(totalRect.height()));
    transform = transform.rotate(touchScreenRotationAngle);

    // After rotation we need to make the matrix origin aligned with the workspace again
    // ____                                                      ___
    // |__|  -> 90° clockwise -> ___  -> needs to be moved up -> | |
    //                           | |                             |_|
    //                           |_|
    switch (touchScreenRotationAngle) {
    case 90:
        transform = transform.translate(0, -1);
        break;
    case 270:
        transform = transform.translate(-1, 0);
        break;
    case 180:
        transform = transform.translate(-1, -1);
        break;
    default:
        break;
    }

    auto getAtom = [](xcb_connection_t *connection, const char *name) {
        auto cookie = xcb_intern_atom(connection, true, strlen(name), name);
        auto reply = xcb_intern_atom_reply(connection, cookie, nullptr);
        if (reply) {
            return reply->atom;
        } else {
            return xcb_atom_t(0);
        }
    };

    int nDevices = 0;
    std::unique_ptr<XDeviceInfo, DeviceListDeleter> deviceInfo(XListInputDevices(display, &nDevices));
    auto touchScreenAtom = getAtom(connection, XI_TOUCHSCREEN);
    if (touchScreenAtom == 0) {
        return;
    }
    auto matrixAtom = getAtom(connection, "Coordinate Transformation Matrix");
    if (matrixAtom == 0) {
        return;
    }
    auto calibrationMatrixAtom = getAtom(connection, "libinput Calibration Matrix");
    auto floatAtom = getAtom(connection, "FLOAT");
    if (floatAtom == 0) {
        return;
    }

    auto setMatrixAtom = [display, floatAtom](XDeviceInfo *info, Atom atom, const QTransform &transform) {
        Atom type;
        int format = 0;
        unsigned long nItems, bytesAfter;
        unsigned char *dataPtr = nullptr;

        std::unique_ptr<unsigned char, XDeleter> data(dataPtr);
        XIGetProperty(display, info->id, atom, 0, 1000, False, AnyPropertyType, &type, &format, &nItems, &bytesAfter, &dataPtr);

        if (nItems != 9) {
            return;
        }
        if (format != sizeof(float) * CHAR_BIT || type != floatAtom) {
            return;
        }

        float *fData = reinterpret_cast<float *>(dataPtr);

        fData[0] = transform.m11();
        fData[1] = transform.m21();
        fData[2] = transform.m31();

        fData[3] = transform.m12();
        fData[4] = transform.m22();
        fData[5] = transform.m32();

        fData[6] = transform.m13();
        fData[7] = transform.m23();
        fData[8] = transform.m33();

        XIChangeProperty(display, info->id, atom, type, format, PropModeReplace, dataPtr, nItems);
    };

    for (XDeviceInfo *info = deviceInfo.get(); info < deviceInfo.get() + nDevices; info++) {
        // Make sure device is touchscreen
        if (info->type != touchScreenAtom) {
            continue;
        }

        int nProperties = 0;
        std::unique_ptr<Atom, XDeleter> properties(XIListProperties(display, info->id, &nProperties));

        bool matrixAtomFound = false;
        bool libInputCalibrationAtomFound = false;

        Atom *atom = properties.get();
        Atom *atomEnd = properties.get() + nProperties;
        for (; atom != atomEnd; atom++) {
            if (!internalOutputRect.isEmpty() && *atom == matrixAtom) {
                matrixAtomFound = true;
            } else if (!internalOutputRect.isEmpty() && *atom == calibrationMatrixAtom) {
                libInputCalibrationAtomFound = true;
            }
        }

        if (libInputCalibrationAtomFound) {
            setMatrixAtom(info, calibrationMatrixAtom, transform);
        }
        if (matrixAtomFound) {
            setMatrixAtom(info, matrixAtom, libInputCalibrationAtomFound ? QTransform() : transform);
        }

        // For now we assume there is only one touchscreen
        XFlush(display);
        break;
    }
}
#endif

void KScreenDaemon::saveCurrentConfig()
{
    qCDebug(KSCREEN_KDED) << "Saving current config to file";

    // We assume the config is valid, since it's what we got, but we are interested
    // in the "at least one enabled screen" check

    if (m_monitoredConfig->canBeApplied()) {
        m_monitoredConfig->writeFile();
        m_monitoredConfig->log();
    } else {
        qCWarning(KSCREEN_KDED) << "Config does not have at least one screen enabled, WILL NOT save this config, this is not what user wants.";
        m_monitoredConfig->log();
    }
}

void KScreenDaemon::displayButton()
{
    qCDebug(KSCREEN_KDED) << "displayBtn triggered";
    showOSD();
}

void KScreenDaemon::lidClosedChanged(bool lidIsClosed)
{
    // Ignore this when we don't have any external monitors, we can't turn off our
    // only screen
    if (m_monitoredConfig->data()->connectedOutputs().count() == 1) {
        return;
    }

    if (lidIsClosed) {
        // Lid is closed, now we wait for couple seconds to find out whether it
        // will trigger a suspend (see Device::aboutToSuspend), or whether we should
        // turn off the screen
        qCDebug(KSCREEN_KDED) << "Lid closed, waiting to see if the computer goes to sleep...";
        m_lidClosedTimer->start();
        return;
    } else {
        qCDebug(KSCREEN_KDED) << "Lid opened!";
        // We should have a config with "_lidOpened" suffix lying around. If not,
        // then the configuration has changed while the lid was closed and we just
        // use applyConfig() and see what we can do ...
        if (auto openCfg = m_monitoredConfig->readOpenLidFile()) {
            doApplyConfig(std::move(openCfg));
        }
    }
}

void KScreenDaemon::disableLidOutput()
{
    // Make sure nothing has changed in the past second... :-)
    if (!Device::self()->isLidClosed()) {
        return;
    }

    // If we are here, it means that closing the lid did not result in suspend
    // action.
    // FIXME: This could be because the suspend took longer than m_lidClosedTimer
    // timeout. Ideally we need to be able to look into PowerDevil config to see
    // what's the configured action for lid events, but there's no API to do that
    // and I'm not parsing PowerDevil's configs...

    qCDebug(KSCREEN_KDED) << "Lid closed, finding lid to disable";
    for (KScreen::OutputPtr &output : m_monitoredConfig->data()->outputs()) {
        if (output->type() == KScreen::Output::Panel) {
            if (output->isConnected() && output->isEnabled()) {
                // Save the current config with opened lid, just so that we know
                // how to restore it later
                m_monitoredConfig->writeOpenLidFile();
                disableOutput(output);
                refreshConfig();
                return;
            }
        }
    }
}

void KScreenDaemon::outputConnectedChanged()
{
    if (!m_changeCompressor->isActive()) {
        m_changeCompressor->start();
    }

    KScreen::Output *output = qobject_cast<KScreen::Output *>(sender());
    qCDebug(KSCREEN_KDED) << "outputConnectedChanged():" << output->name();

    if (output->isConnected()) {
        Q_EMIT outputConnected(output->name());

        if (!m_monitoredConfig->fileExists()) {
            Q_EMIT unknownOutputConnected(output->name());
        }
    }
}

void KScreenDaemon::monitorConnectedChange()
{
    const KScreen::OutputList outputs = m_monitoredConfig->data()->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        connect(output.data(), &KScreen::Output::isConnectedChanged, this, &KScreenDaemon::outputConnectedChanged, Qt::UniqueConnection);
    }
    connect(
        m_monitoredConfig->data().data(),
        &KScreen::Config::outputAdded,
        this,
        [this](const KScreen::OutputPtr &output) {
            if (output->isConnected()) {
                m_changeCompressor->start();
            }
            connect(output.data(), &KScreen::Output::isConnectedChanged, this, &KScreenDaemon::outputConnectedChanged, Qt::UniqueConnection);
        },
        Qt::UniqueConnection);
    connect(m_monitoredConfig->data().data(),
            &KScreen::Config::outputRemoved,
            this,
            &KScreenDaemon::applyConfig,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

void KScreenDaemon::setMonitorForChanges(bool enabled)
{
    if (m_monitoring == enabled) {
        return;
    }

    qCDebug(KSCREEN_KDED) << "Monitor for changes: " << enabled;
    m_monitoring = enabled;
    if (m_monitoring) {
        connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &KScreenDaemon::configChanged, Qt::UniqueConnection);
    } else {
        disconnect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &KScreenDaemon::configChanged);
    }
}

void KScreenDaemon::disableOutput(const KScreen::OutputPtr &output)
{
    const QRect geom = output->geometry();
    qCDebug(KSCREEN_KDED) << "Laptop geometry:" << geom << output->pos() << (output->currentMode() ? output->currentMode()->size() : QSize());

    // Move all outputs right from the @p output to left
    for (KScreen::OutputPtr &otherOutput : m_monitoredConfig->data()->outputs()) {
        if (otherOutput == output || !otherOutput->isConnected() || !otherOutput->isEnabled()) {
            continue;
        }

        QPoint otherPos = otherOutput->pos();
        if (otherPos.x() >= geom.right() && otherPos.y() >= geom.top() && otherPos.y() <= geom.bottom()) {
            otherPos.setX(otherPos.x() - geom.width());
        }
        qCDebug(KSCREEN_KDED) << "Moving" << otherOutput->name() << "from" << otherOutput->pos() << "to" << otherPos;
        otherOutput->setPos(otherPos);
    }

    // Disable the output
    output->setEnabled(false);
}

#include "daemon.moc"
