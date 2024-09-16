// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDE_DISPLAY_MANAGER_H
#define DDE_DISPLAY_MANAGER_H

#include "displaymanager.h"
#include "config.h"
#include "monitor.h"

#include <QObject>
#include <QTimer>

namespace dde {
namespace display {

class DisplayManager : public QObject
{
    Q_OBJECT

public:
    explicit DisplayManager(QObject *parent = nullptr);
    ~DisplayManager();

    inline QMap<QString, KScreen::OutputPtr> monitors() { return m_monitors; }

private:
    void initConnect();
    void requestBackend();
    void load();
    void handleMonitorAdd(const KScreen::OutputPtr &output);
    void handleMonitorRemove(const KScreen::OutputPtr &output);
    void handleMonitorChange(const KScreen::OutputPtr &output);

private:
    QTimer *m_loadCompressor;   //reload display settings delayed such that daemon can update output values.

    std::unique_ptr<ConfigHandler> m_configHandler;
    QMap<QString, KScreen::OutputPtr> m_monitors;
    bool m_firstLoad;
};

}
}

#endif // DDE_DISPLAY_MANAGER_H
