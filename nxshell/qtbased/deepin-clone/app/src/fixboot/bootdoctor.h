// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BOOTDOCTOR_H
#define BOOTDOCTOR_H

#include <QString>

class BootDoctor
{
public:
    static bool fix(const QString &partDevice);
    static QString errorString();

private:
    static QString m_lastErrorString;
};

#endif // BOOTDOCTOR_H
