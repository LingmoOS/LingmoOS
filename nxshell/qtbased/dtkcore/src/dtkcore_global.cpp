// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkcore_global.h"
#include <QDebug>
#include <QFileInfo>

#if (!defined DTK_VERSION) || (!defined DTK_VERSION_STR)
#error "DTK_VERSION or DTK_VERSION_STR not defined!"
#endif

int dtkVersion()
{
    return DTK_VERSION;
}

const char *dtkVersionString()
{
#ifdef QT_DEBUG
    qWarning() << "Use DTK_VERSION_STR instead.";
#endif
    return "";  // DTK_VERSION_STR;
}
