// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONVERTORV1_0_H
#define CONVERTORV1_0_H

#include "abstractconvertor.h"

namespace GrandSearch {

class ConvertorV1_0 : public AbstractConvertor
{
public:
    explicit ConvertorV1_0();
    ~ConvertorV1_0();
    static AbstractConvertor *create();
    QString version() const Q_DECL_OVERRIDE;
    QHash<QString, ConvertInterface> interfaces() Q_DECL_OVERRIDE;
    static int search(void *in, void *out);
    static int result(void *in, void *out);
    static int stop(void *in, void *out);
    static int action(void *in, void *out);
};

}

#endif // CONVERTORV1_0_H
