/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "config_handler.h"

#include "kcm_screen_debug.h"
#include "output_model.h"

#include <algorithm>
#include <cstdint>
#include <utility>

#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>

#include <QGuiApplication>
#include <QRect>

using namespace KScreen;

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

    m_outputModel = new OutputModel(this);
    connect(m_outputModel, &OutputModel::positionChanged, this, &ConfigHandler::checkScreenNormalization);
    connect(m_outputModel, &OutputModel::sizeChanged, this, &ConfigHandler::checkScreenNormalization);

    const auto outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        initOutput(output);
    }
    m_lastNormalizedScreenSize = screenSize();

    connect(m_outputModel, &OutputModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles) {
        Q_UNUSED(bottomRight)
        // Do not run checks during interactive reaarange
        if (!m_outputModel->isMoving()) {
            checkNeedsSave();
        }
        Q_EMIT changed();
    });
    connect(m_config.data(), &KScreen::Config::outputAdded, this, [this]() {
        Q_EMIT outputConnect(true);
    });
    connect(m_config.data(), &KScreen::Config::outputRemoved, this, [this]() {
        Q_EMIT outputConnect(false);
    });
    connect(m_config.data(), &KScreen::Config::prioritiesChanged, this, &ConfigHandler::outputPrioritiesChanged);

    Q_EMIT outputModelChanged();
}

void ConfigHandler::initOutput(const KScreen::OutputPtr &output)
{
    output->setExplicitLogicalSize(config()->logicalSizeForOutput(*output));

    if (output->isConnected()) {
        m_outputModel->add(output);
    }
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, [this, output]() {
        Q_EMIT outputConnect(output->isConnected());
    });
}

void ConfigHandler::updateInitialData()
{
    m_previousConfig = m_initialConfig->clone();
    connect(new GetConfigOperation(), &GetConfigOperation::finished, this, [this](ConfigOperation *op) {
        if (op->hasError()) {
            return;
        }
        m_initialConfig = qobject_cast<GetConfigOperation *>(op)->config();
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
    if (checkPrioritiesNeedSave()) {
        Q_EMIT needsSaveChecked(true);
        return;
    }
    Q_EMIT needsSaveChecked(checkSaveandTestCommon(true));
}

bool ConfigHandler::checkPrioritiesNeedSave()
{
    if (!(m_config->supportedFeatures() & KScreen::Config::Feature::PrimaryDisplay)) {
        return false;
    }
    // first item of pair is initial config, second is current
    QMap<QString, std::pair<std::optional<uint32_t>, std::optional<uint32_t>>> map;

    // exploiting the fact that operator[] on a map is what's called get_or_insert_default in other languages
    const auto &initialList = m_initialConfig->outputs();
    for (const OutputPtr &output : initialList) {
        map[output->hashMd5()].first = std::optional(output->priority());
    }
    const auto &currentList = m_config->outputs();
    for (const OutputPtr &output : currentList) {
        map[output->hashMd5()].second = std::optional(output->priority());
    }
    // so if we end up with items that are not both initialized to the same priority
    for (const auto &[left, right] : std::as_const(map)) {
        if (!(left.has_value() && right.has_value() && left.value() == right.value())) {
            // then configs must be different after all
            return true;
        }
    }
    return false;
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
                    || output->overscan() != config->overscan()
                    || output->vrrPolicy() != config->vrrPolicy()
                    || output->rgbRange() != config->rgbRange()
                    || output->autoRotatePolicy() != config->autoRotatePolicy()
                    || output->iccProfilePath() != config->iccProfilePath()
                    || output->isHdrEnabled() != config->isHdrEnabled()
                    || output->isWcgEnabled() != config->isWcgEnabled()
                    || output->sdrBrightness() != config->sdrBrightness()
                    || output->sdrGamutWideness() != config->sdrGamutWideness()
                    || output->colorProfileSource() != config->colorProfileSource()) {
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

    m_outputModel->normalizePositions();
    const auto currentScreenSize = screenSize();
    m_lastNormalizedScreenSize = currentScreenSize;

    Q_EMIT screenNormalizationUpdate(true);
    return currentScreenSize;
}

void ConfigHandler::checkScreenNormalization()
{
    const bool normalized = !m_config || (m_lastNormalizedScreenSize == screenSize() && m_outputModel->positionsNormalized());

    Q_EMIT screenNormalizationUpdate(normalized);
}

void ConfigHandler::outputPrioritiesChanged()
{
    checkNeedsSave();
    Q_EMIT changed();
}

KScreen::OutputPtr ConfigHandler::replicationSource(const KScreen::OutputPtr &output) const
{
    return m_control->getReplicationSource(output);
}

void ConfigHandler::setReplicationSource(KScreen::OutputPtr &output, const KScreen::OutputPtr &source)
{
    m_control->setReplicationSource(output, source);
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

#include "moc_config_handler.cpp"
