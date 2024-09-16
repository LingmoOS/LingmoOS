// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUBLISHERDEF_H
#define PUBLISHERDEF_H

#define DeleteObject_Null(obj) \
    if (obj) \
        delete obj; \
    obj = nullptr;

#define for_int(count) for (int i = 0; i < count; ++i)

#define Debug 0

#define Dynamic_Cast(Type, obj, base) \
    Type *obj = dynamic_cast<Type *>(base)

#define Dynamic_Cast_CheckNull(Type, obj, base) \
    Type *obj = dynamic_cast<Type *>(base); \
    if (!obj) \
        return;

#endif // PUBLISHERDEF_H
