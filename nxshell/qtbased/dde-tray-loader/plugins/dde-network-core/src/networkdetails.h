// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKDETAILS_H
#define NETWORKDETAILS_H

#include <QObject>

class QJsonObject;

namespace dde {

namespace network {

class NetworkDetailRealize;

class NetworkDetails : QObject
{
    Q_OBJECT

    friend class ObjectManager;

public:
    QString name();
    QList<QPair<QString, QString>> items();

protected:
    NetworkDetails(NetworkDetailRealize *realize, QObject *parent = Q_NULLPTR);
    ~NetworkDetails();

private:
    NetworkDetailRealize *m_realize;
};

}

}

#endif  // NETWORKDETAILS_H
