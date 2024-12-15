// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CANCELJSONDATA_H
#define CANCELJSONDATA_H
#include "jsondata.h"

class cancelJsonData : public JsonData
{
public:
    cancelJsonData();
    ~cancelJsonData() override;
};

#endif // CANCELJSONDATA_H
