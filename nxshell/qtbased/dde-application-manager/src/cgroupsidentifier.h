// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CGROUPSIDENTIFIER_H
#define CGROUPSIDENTIFIER_H

#include "identifier.h"
#include <QFile>

class CGroupsIdentifier : public Identifier
{
public:
    IdentifyRet Identify(pid_t pid) override;

private:
    [[nodiscard]] static QString parseCGroupsPath(QFile &file) noexcept;
};

#endif
