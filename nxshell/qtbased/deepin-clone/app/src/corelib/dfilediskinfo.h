// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DFILEDISKINFO_H
#define DFILEDISKINFO_H

#include "../dglobal.h"
#include "ddiskinfo.h"

class DFileDiskInfoPrivate;
class DFileDiskInfo : public DDiskInfo
{
public:
    explicit DFileDiskInfo();
    explicit DFileDiskInfo(const QString &fileName);

private:
    DG_DFUNC(DFileDiskInfo)
};

#endif // DFILEDISKINFO_H
