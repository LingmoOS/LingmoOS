// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlmapplication.h"
#include <QDebug>
DlmApplication::DlmApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{
}

void DlmApplication::handleQuitAction()
{
    emit applicatinQuit(false);
}
