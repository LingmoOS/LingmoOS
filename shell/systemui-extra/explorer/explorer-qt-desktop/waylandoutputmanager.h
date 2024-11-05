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

#ifndef WAYLANDOUTPUTMANAGER_H
#define WAYLANDOUTPUTMANAGER_H

#include <QObject>

#include <KWayland/Client/xdgoutput.h>
#include <KWayland/Client/output.h>
#include <QMap>

class QSocketNotifier;
class QTimeLine;

namespace KWayland {
namespace Client {
class Registry;
}
}

class WaylandOutputManager : public QObject
{
    Q_OBJECT
public:
    explicit WaylandOutputManager(QObject *parent = nullptr);

    QList<KWayland::Client::Output *> outputs();
    QList<KWayland::Client::XdgOutput *> xdgOutputs();

Q_SIGNALS:
    void outputAdded(KWayland::Client::Output *output);
    void xdgOutputsAdded(KWayland::Client::XdgOutput *xdgOutput);

public Q_SLOTS:
    void run();
    void setLINGMOOutputEnable(bool enable);

protected:
    void addXdgOutput(KWayland::Client::Output *output);

private Q_SLOTS:
    void setLINGMOOutputEnableInternal();

private:
    struct wl_display *m_display = nullptr;
    int m_fd = -1;
    KWayland::Client::Registry *m_registry = nullptr;
    KWayland::Client::EventQueue *m_eventQueue = nullptr;

    KWayland::Client::XdgOutputManager *m_xdgOutputManager = nullptr;

    QMap<KWayland::Client::Output *, KWayland::Client::XdgOutput *> m_outputs;

    QSocketNotifier *m_socketNotifier = nullptr;

    struct lingmo_output *m_lingmoOutput = nullptr;
    QTimeLine *m_timeLine = nullptr;
};

#endif // WAYLANDOUTPUTMANAGER_H
