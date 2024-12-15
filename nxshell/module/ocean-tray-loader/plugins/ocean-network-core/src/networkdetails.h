// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKDETAILS_H
#define NETWORKDETAILS_H

#include <QObject>

class QJsonObject;

namespace ocean {

namespace network {

class NetworkDetailRealize;

class NetworkDetails : public QObject
{
    Q_OBJECT

    friend class ObjectManager;

public:
    QString name();
    QList<QPair<QString, QString>> items();

signals:
    void infoChanged();

protected:
    NetworkDetails(NetworkDetailRealize *realize, QObject *parent = Q_NULLPTR);
    ~NetworkDetails();

private:
    NetworkDetailRealize *m_realize;
};

}

}

#endif  // NETWORKDETAILS_H
