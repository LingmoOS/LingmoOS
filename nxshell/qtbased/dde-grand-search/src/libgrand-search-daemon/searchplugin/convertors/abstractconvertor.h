// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTCONVERTOR_H
#define ABSTRACTCONVERTOR_H

#include <QJsonObject>
#include <QHash>

//interface
#define PLUGININTERFACE_TYPE_SEARCH "search"
#define PLUGININTERFACE_TYPE_RESULT "result"
#define PLUGININTERFACE_TYPE_STOP "stop"
#define PLUGININTERFACE_TYPE_ACTION "action"

namespace GrandSearch {

typedef int (*ConvertInterface)(void *, void *);

class AbstractConvertor
{
public:
    explicit AbstractConvertor();
    virtual ~AbstractConvertor();
    virtual QString version() const;
    virtual QHash<QString, ConvertInterface> interfaces();
};

}

#endif // ABSTRACTCONVERTOR_H
