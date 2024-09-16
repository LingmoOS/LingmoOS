// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpmsclient.h"

#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QRect>
#include <QStandardPaths>
#include <QThread>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/registry.h>

using namespace KScreen;

using namespace KWayland::Client;

DpmsClient::DpmsClient(QObject *parent)
    : QObject(parent)
    , m_registry(new KWayland::Client::Registry)
{
}

DpmsClient::~DpmsClient()
{
    delete m_registry;
    delete m_connection;
}

void DpmsClient::connect()
{
    // setup connection
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);
    connected();
}

void DpmsClient::connected()
{
    qDebug() << "Connected!";
    m_registry->create(m_connection);
    QObject::connect(m_registry, &Registry::interfacesAnnounced, this, [this] {
        const bool hasDpms = m_registry->hasInterface(Registry::Interface::Dpms);
        // QLabel *hasDpmsLabel = new QLabel(&window);
        if (hasDpms) {
            qDebug() << QStringLiteral("Compositor provides a DpmsManager");
        } else {
            qDebug() << QStringLiteral("Compositor does not provid a DpmsManager");
        }

        if (hasDpms) {
            const auto dpmsData = m_registry->interface(Registry::Interface::Dpms);
            m_dpmsManager = m_registry->createDpmsManager(dpmsData.name, dpmsData.version);
        }

        Q_EMIT this->ready();
    });
    m_registry->setup();
}

void KScreen::DpmsClient::changeMode(KWayland::Client::Dpms::Mode mode)
{
    const auto outputs = m_registry->interfaces(Registry::Interface::Output);
    for (auto outputInterface : outputs) {
        KWayland::Client::Output *output = m_registry->createOutput(outputInterface.name, outputInterface.version, m_registry);

        Dpms *dpms = nullptr;
        if (m_dpmsManager) {
            dpms = m_dpmsManager->getDpms(output, output);
        }

        if (dpms) {
            QObject::connect(
                dpms,
                &Dpms::supportedChanged,
                this,
                [dpms, mode, output, this] {
                    if (m_excludedOutputNames.contains(output->model())) {
                        qDebug() << "Skipping" << output->model() << output->manufacturer();
                        return;
                    }

                    if (dpms->isSupported()) {
                        QObject::connect(dpms, &Dpms::modeChanged, this, &DpmsClient::modeChanged, Qt::QueuedConnection);
                        qDebug() << "Switching" << output->model() << output->manufacturer() << (mode == Dpms::Mode::On ? "on" : "off");
                        m_modeChanges++;
                        dpms->requestMode(mode);
                    }
                },
                Qt::QueuedConnection);
        }
    }
}

void DpmsClient::modeChanged()
{
    m_modeChanges = m_modeChanges - 1;
    if (m_modeChanges <= 0) {
        Q_EMIT finished();
        m_modeChanges = 0;
    }
}

void DpmsClient::on()
{
    changeMode(Dpms::Mode::On);
}

void KScreen::DpmsClient::off()
{
    changeMode(Dpms::Mode::Off);
}
