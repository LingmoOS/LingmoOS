// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LOADFOLDERWORKER_H
#define UT_LOADFOLDERWORKER_H

#include <QObject>
#include "gtest/gtest.h"
struct VNOTE_FOLDERS_MAP;

class UT_LoadFolderWorker : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    explicit UT_LoadFolderWorker(QObject *parent = nullptr);

signals:

public slots:
    void onFolderLoad(VNOTE_FOLDERS_MAP *folders);
};

#endif // UT_LOADFOLDERWORKER_H
