// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTCLASS_H
#define DPRINTCLASS_H
#include <vector>
#include <string>
#include "ddestination.h"

using namespace std;

class DPrintClass : public DDestination
{
public:
    DPrintClass();

private:
    vector<string> m_strPrinters;
};

#endif // DPRINTCLASS_H
