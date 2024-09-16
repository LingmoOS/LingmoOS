// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <dfileinfo.h>

class TestFile : public DFileInfo
{
    QIODevice *createIODevice() const override;
};
