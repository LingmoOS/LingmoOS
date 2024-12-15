// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATACONVERTOR_H
#define DATACONVERTOR_H

#include "abstractconvertor.h"

#include <QtCore>

#define DataConvIns GrandSearch::DataConvertor::instance()

namespace GrandSearch {

typedef AbstractConvertor* (*CreateConvertor)();

class DataConvertor
{
public:
    static DataConvertor *instance();
    void initConvetor();
    bool isSupported(const QString &ver);
    int convert(const QString &version, const QString &type, void *in, void *out);
    bool regist(const QString &ver, CreateConvertor creator);
    void unRegist(CreateConvertor creator);

protected:
    explicit DataConvertor();
protected:
    bool m_inited = false;
    QHash<QString, CreateConvertor> m_convertors;
    QReadWriteLock m_rwLock;
};

}

#endif // DATACONVERTOR_H
