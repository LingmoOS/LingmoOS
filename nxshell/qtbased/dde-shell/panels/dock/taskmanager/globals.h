// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QString>
#include <QRegularExpression>

namespace dock {

static inline const QString DOCK_ACTION_ALLWINDOW = "dock-action-allWindow";
static inline const QString DOCK_ACTION_FORCEQUIT = "dock-action-forceQuit";
static inline const QString DOCK_ACTION_CLOSEALL = "dock-action-closeAll";
static inline const QString DOCK_ACTIN_LAUNCH = "dock-action-launch";
static inline const QString DOCK_ACTION_DOCK = "dock-action-dock";

static inline const QString TASKMANAGER_ALLOWFOCEQUIT_KEY = "Allow_Force_Quit";
static inline const QString TASKMANAGER_WINDOWSPLIT_KEY = "Window_Split";
static inline const QString TASKMANAGER_DOCKEDITEMS_KEY = "Docked_Items";

// copy from application-manager

inline static QString escapeToObjectPath(const QString &str)
{
    if (str.isEmpty()) {
        return "_";
    }

    auto ret = str;
    QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(_%1)"}.arg(hexStr));
        }
    }
    return ret;
}

inline static QString unescapeFromObjectPath(const QString &str)
{
    auto ret = str;
    for (qsizetype i = 0; i < str.size(); ++i) {
        if (str[i] == '_' and i + 2 < str.size()) {
            auto hexStr = str.sliced(i + 1, 2);
            ret.replace(QString{"_%1"}.arg(hexStr), QChar::fromLatin1(hexStr.toUInt(nullptr, 16)));
            i += 2;
        }
    }
    return ret;
}
}
