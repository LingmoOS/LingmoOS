// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DGLOBAL_H
#define DGLOBAL_H

#define DG_DFUNC(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(qGetPtrHelper(d)); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(qGetPtrHelper(d)); }

class Global {
public:
    static bool isOverride;
    static bool isTUIMode;

    static int bufferSize;
    static int compressionLevel;
    static int debugLevel;

    static bool disableMD5CheckForDimFile;
    static bool disableLoopDevice;

    static bool fixBoot;
};

#endif // DGLOBAL_H
