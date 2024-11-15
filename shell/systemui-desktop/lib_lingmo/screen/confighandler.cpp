/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "confighandler.h"

#include "outputmodel.h"

#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>

#include <QRect>
#include <QDebug>

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

    m_outputs = new OutputModel(this);
    connect(m_outputs, &OutputModel::positionChanged, this, &ConfigHandler::checkScreenNormalization);
    connect(m_outputs, &OutputModel::sizeChanged, this, &ConfigHandler::checkScreenNormalization);

    for (const KScreen::OutputPtr &output : config->outputs()) {
        initOutput(output);
    }
    m_lastNormalizedScreenSize = screenSize();

    // TODO: put this into m_initialControl
    m_initialRetention = getRetention();
    Q_EMIT retentionChanged();

    connect(m_outputs, &OutputModel::changed, this, [this]() {
        checkNeedsSave();
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

void ConfigHandler::resetScale(const KScreen::OutputPtr &output)
{
    // Load scale control (either not set, same or windowing system does not transmit scale).
    const qreal scale = m_control->getScale(output);
    if (scale > 0) {
        output->setScale(scale);
        for (auto initialOutput : m_initialConfig->outputs()) {
            if (initialOutput->id() == output->id()) {
                initialOutput->setScale(scale);
                break;
            }
        }
    }
}

void ConfigHandler::initOutput(const KScreen::OutputPtr &output)
{
    if (output->isConnected()) {
        resetScale(output);
        m_outputs->add(output);
    }
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, [this, output]() {
        Q_EMIT outputConnect(output->isConnected());
    });
}

void ConfigHandler::updateInitialData()
{
    m_initialRetention = getRetention();
    connect(new GetConfigOperation(), &GetConfigOperation::finished, this, [this](ConfigOperation *op) {
        if (op->hasError()) {
            return;
        }
        m_initialConfig = qobject_cast<GetConfigOperation *>(op)->config();
        for (auto output : m_config->outputs()) {
            resetScale(output);
        }
        m_initialControl.reset(new ControlConfig(m_initialConfig));
        checkNeedsSave();
    });
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

    for (const auto &output : m_config->connectedOutputs()) {
        const QString hash = output->hashMd5();
        for (const auto &initialOutput : m_initialConfig->outputs()) {
            if (hash != initialOutput->hashMd5()) {
                continue;
            }
            bool needsSave = false;
            if (output->isEnabled() != initialOutput->isEnabled()) {
                needsSave = true;
            }
            // clang-format off
            if (output->isEnabled()) {
                needsSave |= output->currentModeId() !=
                                initialOutput->currentModeId()
                             || output->pos() != initialOutput->pos()
                             || output->scale() != initialOutput->scale()
                             || output->rotation() != initialOutput->rotation()
                             || output->replicationSource() != initialOutput->replicationSource()
                             || autoRotate(output) != m_initialControl->getAutoRotate(output)
                             || autoRotateOnlyInTabletMode(output)
                                    != m_initialControl->getAutoRotateOnlyInTabletMode(output);
            }
            // clang-format on
            if (needsSave) {
                Q_EMIT needsSaveChecked(true);
                return;
            }
            break;
        }
    }
    Q_EMIT needsSaveChecked(false);
}

QSize ConfigHandler::screenSize() const
{
    int width = 0, height = 0;
    QSize size;

    for (const auto &output : m_config->connectedOutputs()) {
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
    bool changed = m_outputs->normalizePositions();

    const auto currentScreenSize = screenSize();
    changed |= m_lastNormalizedScreenSize != currentScreenSize;
    m_lastNormalizedScreenSize = currentScreenSize;

    Q_EMIT screenNormalizationUpdate(true);
    return currentScreenSize;
}

void ConfigHandler::checkScreenNormalization()
{
    const bool normalized = !m_config || (m_lastNormalizedScreenSize == screenSize() && m_outputs->positionsNormalized());

    Q_EMIT screenNormalizationUpdate(normalized);
}

void ConfigHandler::primaryOutputSelected(int index)
{
    Q_UNUSED(index)
    // TODO
}

void ConfigHandler::outputPrioritiesChanged()
{
    checkNeedsSave();
    Q_EMIT changed();
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
    for (const auto &output : m_config->connectedOutputs()) {
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

void ConfigHandler::writeControl()
{
    if (!m_control) {
        return;
    }
    m_control->writeFile();
}
