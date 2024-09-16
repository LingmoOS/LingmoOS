// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPINFO_H
#define DESKTOPINFO_H

#include <QString>

class DesktopInfo
{
public:
    DesktopInfo();
    bool waylandDectected();

private:
    QString XDG_SESSION_TYPE;
    QString WAYLAND_DISPLAY;

};

#endif // DESKTOPINFO_H
