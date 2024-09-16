// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "cusapplication.h"

CusApplication::CusApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{
}

void CusApplication::handleQuitAction()
{
    emit handleQuitActionChanged();
    DApplication::handleQuitAction();
}
