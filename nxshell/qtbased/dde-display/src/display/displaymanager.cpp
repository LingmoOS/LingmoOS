// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displaymanager.h"
#include "monitoradaptor.h"

#include <kscreen/getconfigoperation.h>
#include <kscreen/configmonitor.h>
#include <kscreen/output.h>

#include <QProcess>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusConnection>

using namespace KScreen;
using namespace dde::display;

DisplayManager::DisplayManager(QObject *parent)
    : QObject(parent)
    ,m_loadCompressor(new QTimer(this))
    ,m_firstLoad(true)
{
    load();
}

DisplayManager::~DisplayManager()
{

}

void DisplayManager::initConnect()
{

}

void DisplayManager::load()
{
    qDebug() << "ready to read in config.";

    if (m_firstLoad) {
        requestBackend();
        m_firstLoad = false;
    }

   auto *oldConfig = m_configHandler.release();
    if (oldConfig) {
        delete oldConfig;
    }

    m_configHandler.reset(new ConfigHandler(this));

    connect(new GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, [this](KScreen::ConfigOperation *op) {
              if (op->hasError()) {
                m_configHandler.reset();
                return;
              }

              KScreen::ConfigPtr config =
                  qobject_cast<GetConfigOperation *>(op)->config();
              m_configHandler->setConfig(config);
            });

    connect(m_configHandler.get(), &ConfigHandler::addMonitor, this, &DisplayManager::handleMonitorAdd);
    connect(m_configHandler.get(), &ConfigHandler::removeMonitor, this, &DisplayManager::handleMonitorRemove);
    connect(m_configHandler.get(), &ConfigHandler::monitorChanged, this, &DisplayManager::handleMonitorChange);
}

void DisplayManager::requestBackend()
{
    QDBusInterface inter("org.kde.KScreen",
                             "/",
                             "org.kde.KScreen",
                             QDBusConnection::sessionBus());
    if (!inter.isValid()) {
        QProcess process;
        process.start("uname", QStringList() << "-m");
        process.waitForFinished();
        QString output = process.readAll();
        output = output.simplified();

        QString command = "/usr/lib/" + output + "-linux-gnu"
                          +"/libexec/kf5/kscreen_backend_launcher";
        QProcess::startDetached(command, QStringList{});
    }
}

void DisplayManager::handleMonitorAdd(const KScreen::OutputPtr &output)
{   
    if (!output) {
        return;
        qWarning() << "invalid output";
    }

    if (!m_monitors.isEmpty()) {
        m_monitors.clear();
    }

    Monitor *monitor = new Monitor(output, this);

    new MonitorAdaptor(monitor);

    QString path = QString("/org/deepin/dde/Display1") + QString("/Monitor_") + QString::number(output->id());
    QDBusConnection::sessionBus().registerObject(path, "org.deepin.dde.Display1.Monitor", monitor);

    m_monitors[path] = output;
}

void DisplayManager::handleMonitorRemove(const KScreen::OutputPtr &output)
{

}

void DisplayManager::handleMonitorChange(const KScreen::OutputPtr &output)
{

}
