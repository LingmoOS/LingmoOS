// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WM_WINDOW_LIST_H
#define WM_WINDOW_LIST_H

#include "wm_connection.h"
#include "wm_atom.h"
#include "wm_info.h"

namespace core {
namespace process {
class Process;
}
}

namespace core {
namespace wm {

union size_u {
    struct size_t {
        uint w;
        uint h;
    } s;
    uint64_t k;
};

/**
 * @brief The WMWindowList class
 */
class WMWindowList : public QObject
{
    Q_OBJECT

public:
    explicit WMWindowList(QObject *parent = nullptr);
    ~WMWindowList() override = default;

    int getAppCount();

    QImage getWindowIcon(pid_t pid) const;
    QString getWindowTitle(pid_t pid) const;

    bool isTrayApp(pid_t pid) const;
    bool isGuiApp(pid_t pid) const;
    bool isDesktopEntryApp(pid_t pid) const;
    void addDesktopEntryApp(core::process::Process *proc);

    void removeDesktopEntryApp(pid_t pid);
    void updateWindowListCache();

private:
    QList<WMWId> getTrayWindows() const;
    WMWindow getWindowInfo(WMWId winId);
    pid_t getWindowPid(WMWId window) const;
    QStringList getWindowType(WMWId winId) const;
    QByteArray getAtomName(xcb_connection_t *conn, xcb_atom_t atom) const;

private:
    std::map<pid_t, WMWindow> m_guiAppcache;
    std::map<pid_t, WMWindow> m_trayAppcache;

    QList<pid_t> m_desktopEntryCache;
    WMConnection m_conn;
};

} // namespace wm
} // namespace core

#endif // WM_WINDOW_LIST_H
