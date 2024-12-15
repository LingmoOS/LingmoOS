// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROPERTIESFORWARDER_H
#define PROPERTIESFORWARDER_H

#include <QObject>

class PropertiesForwarder : public QObject
{
    Q_OBJECT
public:
    explicit PropertiesForwarder(QString path, QObject *parent);
public Q_SLOTS:
    void PropertyChanged();

private:
    QString m_path;
};

#endif
