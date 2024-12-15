// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <sys/types.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_ewmh.h>

#include <QMap>
#include <QObject>
#include <QSharedPointer>

namespace dock {
const int MotifHintStatus               = 8;
const int MotifHintFunctions            = 1;
const int MotifHintInputMode            = 4;
const int MotifHintDecorations          = 2;

const int MotifFunctionNone             = 0;
const int MotifFunctionAll              = 1;
const int MotifFunctionMove             = 4;
const int MotifFunctionClose            = 32;
const int MotifFunctionResize           = 2;
const int MotifFunctionMinimize         = 8;
const int MotifFunctionMaximize         = 16;

typedef struct {
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    int32_t inputMode;
    uint32_t status;
} MotifWMHints;

class X11Utils
{
public:
    static X11Utils* instance();
    xcb_connection_t* getXcbConnection();

    xcb_window_t getRootWindow();
    xcb_window_t getActiveWindow();
    xcb_atom_t getAtomByName(const QString &name);
    QString getNameByAtom(const xcb_atom_t &atom);
    pid_t getWindowPid(const xcb_window_t &window);
    QString getWindowName(const xcb_window_t &window);
    QString getWindowIcon(const xcb_window_t &window);
    QString getWindowIconName(const xcb_window_t &window);
    QList<xcb_atom_t> getWindowState(const xcb_window_t &window);
    QList<xcb_atom_t> getWindowTypes(const xcb_window_t &window);
    QList<xcb_window_t> getWindowClientList(const xcb_window_t &window);
    QList<xcb_atom_t> getWindowAllowedActions(const xcb_window_t &window);
    MotifWMHints getWindowMotifWMHints(const xcb_window_t &window);
    QStringList getWindowWMClass(const xcb_window_t &window);
    QRect getWindowGeometry(const xcb_window_t &window);
    xcb_window_t getDecorativeWindow(const xcb_window_t &window);

    void minimizeWindow(const xcb_window_t &window);
    void maxmizeWindow(const xcb_window_t &window);
    void closeWindow(const xcb_window_t &window);
    void killClient(const xcb_window_t &window);
    void setActiveWindow(const xcb_window_t &window);
    void restackWindow(const xcb_window_t &window);
    void setWindowIconGemeotry(const xcb_window_t &window, const QRect &geometry);

private:
    X11Utils();
    ~X11Utils();

private:
    xcb_window_t m_rootWindow;
    xcb_ewmh_connection_t m_ewmh;
    QMap<QString, xcb_atom_t> m_atoms;
    xcb_connection_t* m_connection;
};
}
