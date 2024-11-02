/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include <QDebug>
#include <QQmlContext>
#include <cstdio>
#include <QVariant>
#include "lingmo-search-service.h"
#include "file-utils.h"

using namespace LingmoUISearch;
LingmoUISearchService::LingmoUISearchService(int &argc, char *argv[], const QString &applicationName)
        : QtSingleApplication (applicationName, argc, argv)
{
    qDebug()<<"lingmo search service constructor start";
    setApplicationVersion(QString("v%1").arg(VERSION));
    setQuitOnLastWindowClosed(false);

    if (!this->isRunning()) {
        connect(this, &QtSingleApplication::messageReceived, [=](const QString& msg) {
            this->parseCmd(msg, true);
        });
        qRegisterMetaType<IndexType>("IndexType");
        m_indexScheduler = new IndexScheduler(this);
        DirWatcher::getDirWatcher()->initDbusService();
        m_monitor = new Monitor(m_indexScheduler, this);
        m_qroHost.setHostUrl(nodeUrl());
        qDebug() << "Init remote status object:" << m_qroHost.enableRemoting<MonitorSourceAPI>(m_monitor);
    }

    //parse cmd
    qDebug()<<"parse cmd";
    auto message = arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());

    qDebug()<<"lingmo search service constructor end";
}

LingmoUISearchService::~LingmoUISearchService()
{
    if(m_quickView) {
        delete m_quickView;
        m_quickView = nullptr;
    }
}

void LingmoUISearchService::parseCmd(const QString& msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Stop service"));
    parser.addOption(quitOption);

    QCommandLineOption monitorWindow(QStringList()<<"m"<<"monitor", tr("Show index monitor window"));
    parser.addOption(monitorWindow);

    QCommandLineOption statusOption(QStringList()<<"s"<<"status", tr("show status of file index service"));
    parser.addOption(statusOption);

    QCommandLineOption forceUpdateOption(QStringList()<<"u"<<"update",
                                         tr("stop current index job and perform incremental updates for<index type>"),
                                         "indexType");
    parser.addOption(forceUpdateOption);

    if (isPrimary) {
        const QStringList args = QString(msg).split(' ');
        parser.process(args);
        if (parser.isSet(monitorWindow)) {
            loadMonitorWindow();
            m_quickView->show();
            return;
        }
        if (parser.isSet(quitOption)) {
            qApp->quit();
            return;
        }
        if (parser.isSet(forceUpdateOption)) {
            BatchIndexer::Targets targets = BatchIndexer::None;
            QString target = parser.value(forceUpdateOption);
            if(target == "all") {
                targets = BatchIndexer::All;
            } else if (target == "basic") {
                targets = BatchIndexer::Basic;
            } else if (target == "content") {
                targets = BatchIndexer::Content;
            } else if (target == "ocr") {
                targets = BatchIndexer::Ocr;
            } else if (target == "ai") {
                target = BatchIndexer::Ai;
            }
            m_indexScheduler->forceUpdate(targets);
        }
    } else {
        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());
        if(parser.isSet(statusOption)) {
            showMonitorState();
            return;
        } else {
            sendMessage(msg);
            ::exit(0);
        }
    }
}

void LingmoUISearchService::loadMonitorWindow()
{
    if(!m_monitor) {
        m_monitor = new Monitor(m_indexScheduler, this);
    }
    if(!m_quickView) {
        m_quickView = new QQuickView();
        m_quickView->setResizeMode(QQuickView::SizeRootObjectToView);
        m_quickView->rootContext()->setContextProperty("monitor", m_monitor);
        m_quickView->setSource(m_qmlPath);
    }
}

void LingmoUISearchService::showMonitorState()
{
    auto indexMonitor = IndexMonitor::self();

    connect(indexMonitor, &IndexMonitor::serviceReady, [&, indexMonitor](){
        qDebug() << "QRemoteObjectReplica initialized";
        QString state = indexMonitor->indexState();
        QString message = "Current index path: " + indexMonitor->currentIndexPaths().join(",") + "\n"
                          + "Current index scheduler state: " + indexMonitor->indexState() + "\n";
        if(state == "Idle" || state == "Stop") {
            message += "Basic index size: " + QString::number(indexMonitor->basicIndexDocNum()) + " \n"
                       + "Content index size: " + QString::number(indexMonitor->contentIndexDocNum()) + " \n"
                       + "Ocr index size: " + QString::number(indexMonitor->ocrContentIndexDocNum()) + " \n";

        } else {
            message += "Basic index progress: " + QString::number(indexMonitor->basicIndexProgress()) + " of " + QString::number(indexMonitor->basicIndexSize())+ " finished\n"
                       + "Content index progress: " + QString::number(indexMonitor->contentIndexProgress()) + " of " + QString::number(indexMonitor->contentIndexSize())+ " finished\n"
                       + "Ocr content index progress: " + QString::number(indexMonitor->ocrContentIndexProgress()) + " of " + QString::number(indexMonitor->ocrContentIndexSize())+ " finished\n";
        }
        IndexMonitor::stopMonitor();
        fputs(qPrintable(message), stdout);
        qApp->quit();
    });
}

QUrl LingmoUISearchService::nodeUrl()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    return QUrl(QStringLiteral("local:lingmo-search-service-monitor-") + QString(qgetenv("USER")) + display);
}
