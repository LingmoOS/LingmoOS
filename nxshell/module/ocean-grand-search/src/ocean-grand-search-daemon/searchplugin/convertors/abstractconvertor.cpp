// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractconvertor.h"

using namespace GrandSearch;

AbstractConvertor::AbstractConvertor()
{

}

AbstractConvertor::~AbstractConvertor()
{

}

QString AbstractConvertor::version() const
{
    return "";
}

QHash<QString, ConvertInterface> AbstractConvertor::interfaces()
{
    return {};
}
