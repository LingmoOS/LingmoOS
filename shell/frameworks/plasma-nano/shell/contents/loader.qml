/*   vim:set foldmethod=marker:
 *
 *   SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2

Item {
    id: main

    property string shell  : "org.kde.plasma.mini"
    property bool willing  : true
    property string currentSession
    property int priority : 0 //currentSession == "/usr/share/xsessions/plasma-minishell" ? 0 : 10

    // This is not needed, but allows the
    // handler to know whether its shell is loaded
    property bool loaded   : false
}

