// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHSERVICE_P_H
#define GRANDSEARCHSERVICE_P_H

#include "grandsearchservice.h"

namespace GrandSearch {

class GrandSearchServicePrivate
{
public:
    explicit GrandSearchServicePrivate(MainWindow *mainWindow, GrandSearchService *parent = nullptr);

    GrandSearchService *q_p = nullptr;
    MainWindow *m_mainWindow = nullptr;
};

}

#endif // GRANDSEARCHSERVICE_P_H
