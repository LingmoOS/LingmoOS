// Copyright (C) 2011 ~ 2017 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resolutionlist.h"

void registerResolutionListMetaType()
{
    registerResolutionMetaType();

    qRegisterMetaType<ResolutionList>("ResolutionList");
    qDBusRegisterMetaType<ResolutionList>();
}
