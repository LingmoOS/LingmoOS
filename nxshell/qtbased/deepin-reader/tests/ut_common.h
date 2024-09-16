// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_COMMON_H
#define UT_COMMON_H

#include "stub.h"

extern bool g_QWidget_isVisible_result;                 // QWidget isVisible返回值

class UTCommon
{
public:
    UTCommon();
    ~UTCommon();

    /**
     * @brief stub_DMenu_exec     针对DMenu的exec打桩
     * @param stub
     * @return
     */
    static void stub_DMenu_exec(Stub &stub);

    /**
     * @brief stub_QWidget_isVisible     针对QWidget的isVisible打桩
     * @param stub
     * @param isVisible
     */
    static void stub_QWidget_isVisible(Stub &stub, bool isVisible);

    /**
     * @brief stub_QProcess_startDetached     针对QProcess的startDetached打桩
     * @param stub
     */
    static void stub_QProcess_startDetached(Stub &stub);
};


#endif // UT_COMMON_H
