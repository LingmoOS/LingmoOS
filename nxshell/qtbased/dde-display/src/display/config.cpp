// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>

#include <QGuiApplication>
#include <QRect>

using namespace KScreen;
using namespace dde::display;

ConfigHandler::ConfigHandler(QObject *parent)
    : QObject(parent)
{
}

void ConfigHandler::setConfig(KScreen::ConfigPtr config)
{
    m_config = config;
    m_initialConfig = m_config->clone();
    m_initialControl.reset(new ControlConfig(m_initialConfig));

    KScreen::ConfigMonitor::instance()->addConfig(m_config);
    m_control.reset(new ControlConfig(config));

    const auto outputs = config->outputs();
    qInfo() << "MY_TEST" << outputs;
    for (const KScreen::OutputPtr &output : outputs) {
        initOutput(output);
    }
    m_lastNormalizedScreenSize = screenSize();

    // TODO: put this into m_initialControl
    m_initialRetention = getRetention();
    Q_EMIT retentionChanged();

    connect(m_config.data(), &KScreen::Config::outputAdded, this, [this]() {
        Q_EMIT outputConnect(true);
    });
    connect(m_config.data(), &KScreen::Config::outputRemoved, this, [this]() {
        Q_EMIT outputConnect(false);
    });
    connect(m_config.data(), &KScreen::Config::primaryOutputChanged, this, &ConfigHandler::primaryOutputChanged);
}

void ConfigHandler::resetScale(const KScreen::OutputPtr &output)
{
    // Load scale control (either not set, same or windowing system does not transmit scale).
    const qreal scale = m_control->getScale(output);
    if (scale > 0) {
        output->setScale(scale);
        const auto outputs = m_initialConfig->outputs();
        for (auto initialOutput : outputs) {
            if (initialOutput->id() == output->id()) {
                initialOutput->setScale(scale);
                break;
            }
        }
    }
}

void ConfigHandler::initOutput(const KScreen::OutputPtr &output)
{
    // output->setExplicitLogicalSize(config()->logicalSizeForOutput(*output));

    if (output->isConnected()) {
        resetScale(output);
        Q_EMIT addMonitor(output);
    }
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, [this, output]() {
        Q_EMIT outputConnect(output->isConnected());
    });
}

void ConfigHandler::updateInitialData()
{
    m_previousConfig = m_initialConfig->clone();
    m_initialRetention = getRetention();
    connect(new GetConfigOperation(), &GetConfigOperation::finished, this, [this](ConfigOperation *op) {
        if (op->hasError()) {
            return;
        }
        m_initialConfig = qobject_cast<GetConfigOperation *>(op)->config();
        const auto outputs = m_config->outputs();
        for (const auto &output : outputs) {
            resetScale(output);
        }
        m_initialControl.reset(new ControlConfig(m_initialConfig));
        checkNeedsSave();
    });
}

bool ConfigHandler::shouldTestNewSettings()
{
    return checkSaveandTestCommon(false);
}

void ConfigHandler::checkNeedsSave()
{
    if (m_config->supportedFeatures() & KScreen::Config::Feature::PrimaryDisplay) {
        if (m_config->primaryOutput() && m_initialConfig->primaryOutput()) {
            if (m_config->primaryOutput()->hashMd5() != m_initialConfig->primaryOutput()->hashMd5()) {
                Q_EMIT needsSaveChecked(true);
                return;
            }
        } else if ((bool)m_config->primaryOutput() != (bool)m_initialConfig->primaryOutput()) {
            Q_EMIT needsSaveChecked(true);
            return;
        }
    }

    if (m_initialRetention != getRetention()) {
        Q_EMIT needsSaveChecked(true);
        return;
    }
    Q_EMIT needsSaveChecked(checkSaveandTestCommon(true));
}

bool ConfigHandler::checkSaveandTestCommon(bool isSaveCheck)
{
    const auto outputs = m_config->connectedOutputs();
    for (const auto &output : outputs) {
        const QString hash = output->hashMd5();
        const auto configs = m_initialConfig->outputs();
        for (const auto &config : configs) {
            if (hash != config->hashMd5()) {
                continue;
            }

            if (output->isEnabled() != config->isEnabled()) {
                return true;
            }

            // clang-format off
            if (output->isEnabled()) {
                bool scaleChanged = false;
                if (isSaveCheck || m_config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
                     scaleChanged = output->scale() != config->scale();
                }
                if ( output->currentModeId() != config->currentModeId()
                    || output->pos() != config->pos()
                    || scaleChanged
                    || output->rotation() != config->rotation()
                    || output->replicationSource() != config->replicationSource()
                    || autoRotate(output) != m_initialControl->getAutoRotate(output)
                    || autoRotateOnlyInTabletMode(output) != m_initialControl->getAutoRotateOnlyInTabletMode(output)
                    || output->overscan() != config->overscan()
                    || output->vrrPolicy() != config->vrrPolicy()
                    || output->rgbRange() != config->rgbRange()) {
                        return true;
                    }
            }
            // clang-format on
        }
    }
    return false;
}

QSize ConfigHandler::screenSize() const
{
    int width = 0, height = 0;
    QSize size;

    const auto connectedOutputs = m_config->connectedOutputs();
    for (const auto &output : connectedOutputs) {
        if (!output->isPositionable()) {
            continue;
        }
        const int outputRight = output->geometry().right();
        const int outputBottom = output->geometry().bottom();

        if (outputRight > width) {
            width = outputRight;
        }
        if (outputBottom > height) {
            height = outputBottom;
        }
    }
    if (width > 0 && height > 0) {
        size = QSize(width, height);
    } else {
        size = QSize();
    }
    return size;
}

QSize ConfigHandler::normalizeScreen()
{
    if (!m_config) {
        return QSize();
    }

    const auto currentScreenSize = screenSize();
    m_lastNormalizedScreenSize = currentScreenSize;

    Q_EMIT screenNormalizationUpdate(true);
    return currentScreenSize;
}

void ConfigHandler::checkScreenNormalization()
{
    const bool normalized = !m_config || (m_lastNormalizedScreenSize == screenSize());

    Q_EMIT screenNormalizationUpdate(normalized);
}

void ConfigHandler::primaryOutputSelected(int index)
{
    Q_UNUSED(index)
    // TODO
}

void ConfigHandler::primaryOutputChanged(const KScreen::OutputPtr &output)
{
    Q_UNUSED(output)
}

Control::OutputRetention ConfigHandler::getRetention() const
{
    using Retention = Control::OutputRetention;

    auto ret = Retention::Undefined;
    if (!m_control) {
        return ret;
    }
    const auto outputs = m_config->connectedOutputs();
    if (outputs.isEmpty()) {
        return ret;
    }
    ret = m_control->getOutputRetention(outputs.first());

    for (const auto &output : outputs) {
        const auto outputRet = m_control->getOutputRetention(output);
        if (ret != outputRet) {
            // Control file with different retention values per output.
            return Retention::Undefined;
        }
    }

    if (ret == Retention::Undefined) {
        // If all outputs have undefined retention,
        // this should be displayed as global retention.
        return Retention::Global;
    }
    return ret;
}

int ConfigHandler::retention() const
{
    return static_cast<int>(getRetention());
}

void ConfigHandler::setRetention(int retention)
{
    using Retention = Control::OutputRetention;

    if (!m_control) {
        return;
    }
    if (retention != static_cast<int>(Retention::Global) && retention != static_cast<int>(Retention::Individual)) {
        // We only allow setting to global or individual retention.
        return;
    }
    if (retention == ConfigHandler::retention()) {
        return;
    }
    auto ret = static_cast<Retention>(retention);
    const auto connectedOutputs = m_config->connectedOutputs();
    for (const auto &output : connectedOutputs) {
        m_control->setOutputRetention(output, ret);
    }
    checkNeedsSave();
    Q_EMIT retentionChanged();
    Q_EMIT changed();
}

qreal ConfigHandler::scale(const KScreen::OutputPtr &output) const
{
    return m_control->getScale(output);
}

void ConfigHandler::setScale(KScreen::OutputPtr &output, qreal scale)
{
    m_control->setScale(output, scale);
}

KScreen::OutputPtr ConfigHandler::replicationSource(const KScreen::OutputPtr &output) const
{
    return m_control->getReplicationSource(output);
}

void ConfigHandler::setReplicationSource(KScreen::OutputPtr &output, const KScreen::OutputPtr &source)
{
    m_control->setReplicationSource(output, source);
}

bool ConfigHandler::autoRotate(const KScreen::OutputPtr &output) const
{
    return m_control->getAutoRotate(output);
}

void ConfigHandler::setAutoRotate(KScreen::OutputPtr &output, bool autoRotate)
{
    m_control->setAutoRotate(output, autoRotate);
}

bool ConfigHandler::autoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const
{
    return m_control->getAutoRotateOnlyInTabletMode(output);
}

void ConfigHandler::setAutoRotateOnlyInTabletMode(KScreen::OutputPtr &output, bool value)
{
    m_control->setAutoRotateOnlyInTabletMode(output, value);
}

uint32_t ConfigHandler::overscan(const KScreen::OutputPtr &output) const
{
    return m_control->getOverscan(output);
}

void ConfigHandler::setOverscan(const KScreen::OutputPtr &output, uint32_t value)
{
    m_control->setOverscan(output, value);
}

KScreen::Output::VrrPolicy ConfigHandler::vrrPolicy(const KScreen::OutputPtr &output) const
{
    return m_control->getVrrPolicy(output);
}

void ConfigHandler::setVrrPolicy(const KScreen::OutputPtr &output, KScreen::Output::VrrPolicy value)
{
    m_control->setVrrPolicy(output, value);
}

KScreen::Output::RgbRange ConfigHandler::rgbRange(const KScreen::OutputPtr &output) const
{
    return m_control->getRgbRange(output);
}

void ConfigHandler::setRgbRange(const KScreen::OutputPtr &output, KScreen::Output::RgbRange value)
{
    m_control->setRgbRange(output, value);
}

void ConfigHandler::writeControl()
{
    if (!m_control) {
        return;
    }
    m_control->writeFile();
}

