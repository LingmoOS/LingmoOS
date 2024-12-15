// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONHEADERFILE_H
#define COMMONHEADERFILE_H

#define SAFE_DELETE_ELE( ptr ) \
    if (ptr != nullptr)      \
    {                     \
        delete ptr;       \
        ptr = nullptr;       \
    }

#define SAFE_DELETE_ARRAY( ptr ) \
    if (ptr != nullptr)            \
    {                           \
        delete[] ptr;           \
        ptr = nullptr;             \
    }

#endif // COMMONHEADERFILE_H
