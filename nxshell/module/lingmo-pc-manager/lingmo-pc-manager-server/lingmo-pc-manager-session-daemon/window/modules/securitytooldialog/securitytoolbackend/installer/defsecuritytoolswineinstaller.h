// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSWINEINSTALLER_H
#define DEFSECURITYTOOLSWINEINSTALLER_H

#include "defsecuritytoolsbaseinstaller.h"

class DefSecurityToolsWineInstaller : public DefSecurityToolsBaseInstaller
{
    Q_OBJECT
public:
    explicit DefSecurityToolsWineInstaller(QObject *parent = nullptr);
    ~DefSecurityToolsWineInstaller() override;

public:
    virtual void showtool(const QStringList &strParams) override;
};

#endif // DEFSECURITYTOOLSWINEINSTALLER_H
