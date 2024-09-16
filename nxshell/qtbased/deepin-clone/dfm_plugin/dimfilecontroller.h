// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DIMFILECONTROLLER_H
#define DIMFILECONTROLLER_H

#include <dabstractfilecontroller.h>

class DIMFileController : public DAbstractFileController
{
public:
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
};

#endif // DIMFILECONTROLLER_H
