// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LOADNOTEITEMSWORKER_H
#define UT_LOADNOTEITEMSWORKER_H

#include <QObject>
#include "gtest/gtest.h"
struct VNOTE_ALL_NOTES_MAP;
class UT_LoadNoteItemsWorker : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    explicit UT_LoadNoteItemsWorker(QObject *parent = nullptr);

signals:

public slots:
    void onNoteLoad(VNOTE_ALL_NOTES_MAP *foldesMap);
};

#endif // UT_LOADNOTEITEMSWORKER_H
