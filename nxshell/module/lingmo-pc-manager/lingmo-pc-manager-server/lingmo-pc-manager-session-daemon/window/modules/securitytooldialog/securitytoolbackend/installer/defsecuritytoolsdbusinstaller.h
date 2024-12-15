// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSDBUSINSTALLER_H
#define DEFSECURITYTOOLSDBUSINSTALLER_H

#include "defsecuritytoolsbaseinstaller.h"

#include <QObject>

class DefSecurityToolsDbusInstaller : public DefSecurityToolsBaseInstaller
{
    Q_OBJECT
public:
    explicit DefSecurityToolsDbusInstaller(QObject *parent = nullptr);
    ~DefSecurityToolsDbusInstaller() override;

public:
    virtual void showtool(const QStringList &strParams) override;
};

#endif // DEFSECURITYTOOLSDBUSINSTALLER_H
