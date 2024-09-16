// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANBROWSERINFOFILTER_H
#define TRASHCLEANBROWSERINFOFILTER_H

#include "trashcleanappinfofilter.h"

class TrashCleanBrowserInfoFilter : public TrashCleanAppInfoFilter
{
    Q_OBJECT
public:
    explicit TrashCleanBrowserInfoFilter(QObject *parent = nullptr);
    virtual ~TrashCleanBrowserInfoFilter() override = default;
    // 针对浏览器cookies获取
    // 重写以下方法
    virtual TrashCleanAppList InitAppData(QString) override;
    virtual void MergeAppInfo() override;

private:
    virtual bool isValidConfig(const TrashCleanApp &) override;
};

#endif // TRASHCLEANBROWSERINFOFILTER_H
