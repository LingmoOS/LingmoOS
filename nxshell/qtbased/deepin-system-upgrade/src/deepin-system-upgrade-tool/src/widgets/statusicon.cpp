// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "statusicon.h"

QIcon getIcon(CheckResultType type)
{
    switch (type)
    {
    case CheckResultType::PASSED:
        return QIcon::fromTheme("dialog-ok");
    case CheckResultType::WARNING:
        return QIcon::fromTheme("dialog-warning");
    case CheckResultType::FAILED:
        return QIcon::fromTheme("dialog-error");
    }
}
