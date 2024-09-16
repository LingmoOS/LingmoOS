// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYURL_H
#define QUERYURL_H

#include <QObject>

class QueryUrl : public QObject
{
    Q_OBJECT

public:
    explicit QueryUrl(QObject *parent = nullptr);
private:
};

#endif // QUERYURL_H
