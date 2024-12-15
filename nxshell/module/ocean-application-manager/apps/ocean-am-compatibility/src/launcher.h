// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QString>
#include <DExpected>

class Launcher {
public:
    enum LaunchedType{
        Unknown,
        ByUser,
    };
    void setPath(const QString &path);
    void setAction(const QString &action);
    void setLaunchedType(LaunchedType type);
    Dtk::Core::DExpected<void> run();

private:
    Dtk::Core::DExpected<void> launch();

    QString m_path;
    QString m_action;
    LaunchedType m_launchedType = Unknown;
};
