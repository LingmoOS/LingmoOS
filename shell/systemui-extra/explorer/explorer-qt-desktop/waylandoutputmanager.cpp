/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "waylandoutputmanager.h"
#include "lingmo-output-client.h"

#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include <KWayland/Client/registry.h>
#include <KWayland/Client/event_queue.h>

#include <QSocketNotifier>

#include <QRect>
#include <QTimeLine>

#include <QDebug>

WaylandOutputManager::WaylandOutputManager(QObject *parent) : QObject(parent)
{
    m_timeLine = new QTimeLine(1000, this);
    connect(m_timeLine, &QTimeLine::finished, this, &WaylandOutputManager::setLINGMOOutputEnableInternal);

    auto _display = wl_display_connect(NULL);
    m_display = _display;
    int fd = wl_display_get_fd(_display);
    m_fd = fd;
    if (fd == -1) {
        return;
    }

    m_eventQueue = new KWayland::Client::EventQueue(this);
    m_eventQueue->setup(_display);

    m_registry = new KWayland::Client::Registry(this);
    m_registry->create(_display);
    m_registry->setEventQueue(m_eventQueue);
}

void WaylandOutputManager::run()
{
    if (!m_registry) {
        return;
    }

    m_socketNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, [=](){
        wl_display_flush(m_display);
        if (wl_display_dispatch(m_display) == -1) {
            auto error = wl_display_get_error(m_display);
            if (error != 0) {
                if (m_display) {
                    free(m_display);
                }
                return exit(error);
            }
        }
        m_eventQueue->dispatch();
    });

    connect(m_registry, &KWayland::Client::Registry::interfaceAnnounced, this, [=](QByteArray interface, quint32 name, quint32 version){
        QString interfaceName = interface;
        if (interfaceName == "lingmo_output") {
            m_lingmoOutput = static_cast<lingmo_output *>(wl_registry_bind(*m_registry, name, &lingmo_output_interface, version));
            qDebug()<<"lingmo output created"<<m_lingmoOutput<<name<<version;
            lingmo_output_set_outputs_paint_enabled(m_lingmoOutput, true);
        }
    });

    QObject::connect(m_registry, &KWayland::Client::Registry::xdgOutputAnnounced, [=](quint32 name, quint32 version){
        m_xdgOutputManager = m_registry->createXdgOutputManager(name, version);
        qDebug()<<"xdg output manager"<<m_xdgOutputManager<<"created"<<name<<version;
        for (auto output : m_outputs.keys()) {
            if (!m_outputs.value(output)) {
                addXdgOutput(output);
            }
        }
    });

    QObject::connect(m_registry, &KWayland::Client::Registry::outputAnnounced, [=](quint32 name, quint32 version){
        auto output = m_registry->createOutput(name, version);
        qDebug()<<"output added:"<<output;

        QObject::connect(output, &KWayland::Client::Output::changed, [=](){
            qDebug()<<output<<"changed:"<<output->model()<<output->geometry();
        });
        QObject::connect(output, &KWayland::Client::Output::removed, [=](){
            qDebug()<<output<<"removed:"<<output->model();
            m_outputs.remove(output);
            output->deleteLater();
        });

        Q_EMIT outputAdded(output);

        if (!m_xdgOutputManager)
            return;
        addXdgOutput(output);
    });

    m_registry->setup();
    m_eventQueue->dispatch();
}

void WaylandOutputManager::setLINGMOOutputEnable(bool enable)
{
    m_timeLine->setCurrentTime(0);
    m_timeLine->start();
}

void WaylandOutputManager::addXdgOutput(KWayland::Client::Output *output)
{
    //auto _zxdg_output = static_cast<zxdg_output_v1 *>(*xdgOutput);
    //eventQueue.addProxy(_zxdg_output);

    // set xdg output parent as output
    auto xdgOutput = m_xdgOutputManager->getXdgOutput(output, output);
    m_outputs.insert(output, xdgOutput);

    QObject::connect(xdgOutput, &KWayland::Client::XdgOutput::changed, [=](){
        qDebug()<<xdgOutput<<output->model()<<"changed:"<<xdgOutput->logicalPosition()<<xdgOutput->logicalSize();
    });
    QObject::connect(xdgOutput, &KWayland::Client::XdgOutput::destroyed, [=](){
        //qDebug()<<xdgOutput<<output->model()<<"removed";
        qDebug()<<xdgOutput<<"removed";
    });

    Q_EMIT xdgOutputsAdded(xdgOutput);
}

void WaylandOutputManager::setLINGMOOutputEnableInternal()
{
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    if (m_lingmoOutput) {
        //lingmo_output_set_outputs_paint_enabled(m_lingmoOutput, true);
        // flush message to wayland server
        m_eventQueue->dispatch();
    }
}

QList<KWayland::Client::Output *> WaylandOutputManager::outputs()
{
    return m_outputs.keys();
}

QList<KWayland::Client::XdgOutput *> WaylandOutputManager::xdgOutputs()
{
    return m_outputs.values();
}
