/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "qscreenconfig.h"
#include "qscreenbackend.h"
#include "qscreenoutput.h"
#include "qscreenscreen.h"

#include <config.h>
#include <mode.h>
#include <output.h>

#include <QGuiApplication>
#include <QRect>

#include <utility>

using namespace KScreen;

QScreenConfig::QScreenConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new QScreenScreen(this))
    , m_blockSignals(true)
{
    for (const QScreen *qscreen : QGuiApplication::screens()) {
        screenAdded(qscreen);
    }
    m_blockSignals = false;
    connect(qApp, &QGuiApplication::screenAdded, this, &QScreenConfig::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &QScreenConfig::screenRemoved);
}

QScreenConfig::~QScreenConfig()
{
    qDeleteAll(m_outputMap);
}

ConfigPtr QScreenConfig::toKScreenConfig() const
{
    ConfigPtr config(new Config);
    config->setScreen(m_screen->toKScreenScreen());
    updateKScreenConfig(config);
    return config;
}

int QScreenConfig::outputId(const QScreen *qscreen)
{
    for (auto output : std::as_const(m_outputMap)) {
        if (qscreen == output->qscreen()) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void QScreenConfig::screenAdded(const QScreen *qscreen)
{
    qCDebug(KSCREEN_QSCREEN) << "Screen added" << qscreen << qscreen->name();
    QScreenOutput *qscreenoutput = new QScreenOutput(qscreen, this);
    qscreenoutput->setId(outputId(qscreen));
    m_outputMap.insert(qscreenoutput->id(), qscreenoutput);

    if (!m_blockSignals) {
        Q_EMIT configChanged(toKScreenConfig());
    }
}

void QScreenConfig::screenRemoved(QScreen *qscreen)
{
    qCDebug(KSCREEN_QSCREEN) << "Screen removed" << qscreen << QGuiApplication::screens().count();
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    for (auto output : m_outputMap) {
        if (output->qscreen() == qscreen) {
            removedOutputId = output->id();
            m_outputMap.remove(removedOutputId);
            delete output;
        }
    }
    Q_EMIT configChanged(toKScreenConfig());
}

void QScreenConfig::updateKScreenConfig(ConfigPtr &config) const
{
    KScreen::ScreenPtr screen = config->screen();
    m_screen->updateKScreenScreen(screen);
    config->setScreen(screen);

    // Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!m_outputMap.contains(output->id())) {
            config->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    for (QScreenOutput *output : m_outputMap) {
        KScreen::OutputPtr kscreenOutput;
        if (config->outputs().contains(output->id())) {
            kscreenOutput = config->outputs()[output->id()];
            output->updateKScreenOutput(kscreenOutput);
        } else {
            kscreenOutput = output->toKScreenOutput();
            config->addOutput(kscreenOutput);
        }
        if (QGuiApplication::primaryScreen() == output->qscreen()) {
            config->setPrimaryOutput(kscreenOutput);
        }
    }
}

QMap<int, QScreenOutput *> QScreenConfig::outputMap() const
{
    return m_outputMap;
}

#include "moc_qscreenconfig.cpp"
