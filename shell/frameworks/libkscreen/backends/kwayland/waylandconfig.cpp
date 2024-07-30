/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandconfig.h"

#include "kscreen_kwayland_logging.h"

#include "waylandbackend.h"
#include "waylandoutputdevice.h"
#include "waylandoutputmanagement.h"
#include "waylandscreen.h"

#include "tabletmodemanager_interface.h"

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QThread>
#include <QTimer>
#include <configmonitor.h>
#include <mode.h>
#include <output.h>

#include <wayland-client-protocol.h>

#include <chrono>
#include <utility>

using namespace KScreen;
using namespace std::chrono_literals;

WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_outputManagement(nullptr)
    , m_registryInitialized(false)
    , m_blockSignals(true)
    , m_kscreenConfig(new Config)
    , m_kscreenPendingConfig(nullptr)
    , m_screen(new WaylandScreen(this))
    , m_tabletModeAvailable(false)
    , m_tabletModeEngaged(false)
{
    initKWinTabletMode();
    setupRegistry();
}

WaylandConfig::~WaylandConfig()
{
    if (m_registry) {
        wl_registry_destroy(m_registry);
    }
}

void WaylandConfig::initKWinTabletMode()
{
    auto *interface =
        new OrgKdeKWinTabletModeManagerInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/org/kde/KWin"), QDBusConnection::sessionBus(), this);
    if (!interface->isValid()) {
        m_tabletModeAvailable = false;
        m_tabletModeEngaged = false;
        return;
    }

    m_tabletModeAvailable = interface->tabletModeAvailable();
    m_tabletModeEngaged = interface->tabletMode();

    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeChanged, this, [this](bool tabletMode) {
        if (m_tabletModeEngaged == tabletMode) {
            return;
        }
        m_tabletModeEngaged = tabletMode;
        if (!m_blockSignals && m_initializingOutputs.empty()) {
            Q_EMIT configChanged();
        }
    });
    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeAvailableChanged, this, [this](bool available) {
        if (m_tabletModeAvailable == available) {
            return;
        }
        m_tabletModeAvailable = available;
        if (!m_blockSignals && m_initializingOutputs.empty()) {
            Q_EMIT configChanged();
        }
    });
}

void WaylandConfig::blockSignals()
{
    Q_ASSERT(m_blockSignals == false);
    m_blockSignals = true;
}

void WaylandConfig::unblockSignals()
{
    Q_ASSERT(m_blockSignals == true);
    m_blockSignals = false;
}

void WaylandConfig::setupRegistry()
{
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp) {
        return;
    }

    auto display = waylandApp->display();
    m_registry = wl_display_get_registry(display);

    auto globalAdded = [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
        auto self = static_cast<WaylandConfig *>(data);
        if (qstrcmp(interface, WaylandOutputDevice::interface()->name) == 0) {
            self->addOutput(name, std::min(9u, version));
        }
        if (qstrcmp(interface, WaylandOutputManagement::interface()->name) == 0) {
            self->m_outputManagement = new WaylandOutputManagement(registry, name, std::min(9u, version));
        }
        if (qstrcmp(interface, WaylandOutputOrder::interface()->name) == 0) {
            self->m_outputOrder = std::make_unique<WaylandOutputOrder>(registry, name, std::min(1u, version));
            connect(self->m_outputOrder.get(), &WaylandOutputOrder::outputOrderChanged, self, [self](const QList<QString> &names) {
                bool change = false;
                for (const auto &output : std::as_const(self->m_outputMap)) {
                    const uint32_t newIndex = names.indexOf(output->name()) + 1;
                    change = change || output->index() != newIndex;
                    output->setIndex(newIndex);
                }
                if (change && !self->m_blockSignals) {
                    Q_EMIT self->configChanged();
                }
            });
        }
    };

    auto globalRemoved = [](void *data, wl_registry *registry, uint32_t name) {
        Q_UNUSED(registry)
        auto self = static_cast<WaylandConfig *>(data);
        Q_EMIT self->globalRemoved(name);
    };

    static const wl_registry_listener registryListener{globalAdded, globalRemoved};
    wl_registry_add_listener(m_registry, &registryListener, this);

    static const wl_callback_listener callbackListener{[](void *data, wl_callback *callback, uint32_t callbackData) {
        Q_UNUSED(callback)
        Q_UNUSED(callbackData)
        auto self = static_cast<WaylandConfig *>(data);
        self->m_registryInitialized = true;
        self->unblockSignals();
        self->checkInitialized();
    }};
    auto callback = wl_display_sync(waylandApp->display());
    wl_callback_add_listener(callback, &callbackListener, this);
    QElapsedTimer timer;
    timer.start();
    while (!m_initialized) {
        if (timer.durationElapsed() >= 300ms) {
            qCWarning(KSCREEN_WAYLAND) << "Connection to Wayland server timed out.";
            break;
        }
        wl_display_roundtrip(display);
    }
}

int s_outputId = 0;

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qCDebug(KSCREEN_WAYLAND) << "adding output" << name;

    auto device = new WaylandOutputDevice(++s_outputId);
    m_initializingOutputs << device;

    connect(this, &WaylandConfig::globalRemoved, this, [name, device, this](const uint32_t &interfaceName) {
        if (name == interfaceName) {
            removeOutput(device);
        }
    });

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = connect(device, &WaylandOutputDevice::done, this, [this, connection, device]() {
        QObject::disconnect(*connection);
        delete connection;

        m_initializingOutputs.removeOne(device);
        m_outputMap.insert(device->id(), device);
        if (m_outputOrder) {
            device->setIndex(m_outputOrder->order().indexOf(device->name()) + 1);
        }
        checkInitialized();

        if (m_initializingOutputs.isEmpty()) {
            m_screen->setOutputs(m_outputMap.values());
        }
        if (!m_blockSignals && m_initializingOutputs.isEmpty()) {
            Q_EMIT configChanged();
        }

        connect(device, &WaylandOutputDevice::done, this, [this]() {
            // output got update must update current config
            if (!m_blockSignals) {
                Q_EMIT configChanged();
            }
        });
    });

    device->init(m_registry, name, version);
}

void WaylandConfig::removeOutput(WaylandOutputDevice *output)
{
    qCDebug(KSCREEN_WAYLAND) << "removing output" << output->name();

    if (m_initializingOutputs.removeOne(output)) {
        // output was not yet fully initialized, just remove here and return
        delete output;
        return;
    }

    // remove the output from output mapping
    const auto removedOutput = m_outputMap.take(output->id());
    Q_ASSERT(removedOutput == output);
    Q_UNUSED(removedOutput);
    m_screen->setOutputs(m_outputMap.values());
    delete output;

    if (!m_blockSignals) {
        Q_EMIT configChanged();
    }
}

bool WaylandConfig::isReady() const
{
    // clang-format off
    return !m_blockSignals
            && m_registryInitialized
            && m_initializingOutputs.isEmpty()
            && m_outputMap.count() > 0
            && m_outputManagement != nullptr;
    // clang-format on
}

void WaylandConfig::checkInitialized()
{
    if (!m_initialized && isReady()) {
        m_initialized = true;
        m_screen->setOutputs(m_outputMap.values());
        Q_EMIT initialized();
    }
}

KScreen::ConfigPtr WaylandConfig::currentConfig()
{
    m_kscreenConfig->setScreen(m_screen->toKScreenScreen(m_kscreenConfig));

    const auto features = Config::Feature::Writable | Config::Feature::PerOutputScaling | Config::Feature::AutoRotation | Config::Feature::TabletMode
        | Config::Feature::PrimaryDisplay | Config::Feature::XwaylandScales | Config::Feature::SynchronousOutputChanges | Config::Feature::OutputReplication;
    m_kscreenConfig->setSupportedFeatures(features);
    m_kscreenConfig->setValid(qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>());

    KScreen::ScreenPtr screen = m_kscreenConfig->screen();
    m_screen->updateKScreenScreen(screen);

    // Removing removed outputs
    const KScreen::OutputList outputs = m_kscreenConfig->outputs();
    for (const auto &output : outputs) {
        if (!m_outputMap.contains(output->id())) {
            m_kscreenConfig->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet
    KScreen::OutputList kscreenOutputs = m_kscreenConfig->outputs();
    QMap<OutputPtr, uint32_t> priorities;
    for (const auto &output : m_outputMap) {
        KScreen::OutputPtr kscreenOutput;
        if (m_kscreenConfig->outputs().contains(output->id())) {
            kscreenOutput = m_kscreenConfig->outputs()[output->id()];
            output->updateKScreenOutput(kscreenOutput);
        } else {
            kscreenOutput = output->toKScreenOutput();
            m_kscreenConfig->addOutput(kscreenOutput);
        }
        priorities[kscreenOutput] = output->index();
    }
    m_kscreenConfig->setOutputPriorities(priorities);

    m_kscreenConfig->setTabletModeAvailable(m_tabletModeAvailable);
    m_kscreenConfig->setTabletModeEngaged(m_tabletModeEngaged);

    return m_kscreenConfig;
}

QMap<int, WaylandOutputDevice *> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

void WaylandConfig::tryPendingConfig()
{
    if (!m_kscreenPendingConfig) {
        return;
    }
    applyConfig(m_kscreenPendingConfig);
    m_kscreenPendingConfig = nullptr;
}

WaylandOutputDevice *WaylandConfig::findOutputDevice(struct ::kde_output_device_v2 *outputdevice) const
{
    for (WaylandOutputDevice *device : m_outputMap) {
        if (device->object() == outputdevice) {
            return device;
        }
    }
    return nullptr;
}

void WaylandConfig::applyConfig(const KScreen::ConfigPtr &newConfig)
{
    newConfig->adjustPriorities(); // never trust input

    // Create a new configuration object
    auto wlConfig = m_outputManagement->createConfiguration();
    bool changed = false;

    if (m_blockSignals) {
        // Last apply still pending, remember new changes and apply afterwards
        m_kscreenPendingConfig = newConfig;
        return;
    }

    for (const auto &output : newConfig->outputs()) {
        changed |= m_outputMap[output->id()]->setWlConfig(wlConfig, output);
    }

    if (!changed) {
        return;
    }

    // We now block changes in order to compress events while the compositor is doing its thing
    // once it's done or failed, we'll trigger configChanged() only once, and not per individual
    // property change.
    connect(wlConfig, &WaylandOutputConfiguration::applied, this, [this, wlConfig] {
        wlConfig->deleteLater();
        unblockSignals();
        Q_EMIT configChanged();
        tryPendingConfig();
    });
    connect(wlConfig, &WaylandOutputConfiguration::failed, this, [this, wlConfig] {
        wlConfig->deleteLater();
        unblockSignals();
        Q_EMIT configChanged();
        tryPendingConfig();
    });

    // Now block signals and ask the compositor to apply the changes.
    blockSignals();
    wlConfig->apply();
}

#include "moc_waylandconfig.cpp"
