/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef LINGMOSEARCHSERVICE_H
#define LINGMOSEARCHSERVICE_H

#include <QObject>
#include <QCommandLineParser>
#include <QGSettings>
#include <QQuickView>
#include "qtsingleapplication.h"
#include "common.h"
#include "index-scheduler.h"
#include "monitor.h"
#include "index-monitor.h"

namespace LingmoUISearch {

class LingmoUISearchService : public QtSingleApplication
{
    Q_OBJECT
public:
    LingmoUISearchService(int &argc, char *argv[], const QString &applicationName = "lingmo-search-service");
    ~LingmoUISearchService();

protected Q_SLOTS:
    void parseCmd(const QString& msg, bool isPrimary);
private:
    void loadMonitorWindow();
    void showMonitorState();
    static QUrl nodeUrl();

    IndexScheduler *m_indexScheduler = nullptr;
    Monitor *m_monitor = nullptr;
    QRemoteObjectHost m_qroHost;
    QQuickView *m_quickView = nullptr;
    QUrl m_qmlPath = QString("qrc:/qml/IndexMonitor.qml");

};
}
#endif // LINGMOSEARCHSERVICE_H
