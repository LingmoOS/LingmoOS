// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYGROUP_H
#define PROPERTYGROUP_H

#include <QObject>
#include <QPointer>
#include <QMap>

QT_BEGIN_NAMESPACE
class QSignalMapper;
QT_END_NAMESPACE
class PropertyGroup : public QObject
{
    Q_OBJECT
public:
    explicit PropertyGroup(QObject *parent = nullptr);

    int addObject(QObject *obj);
    void removeObject(QObject *obj);
    void addProperty(const QByteArray &propertyName);
    void removeProperty(const QByteArray &propertyName);

private slots:
    void onObjectPropertyChanged();

private:
    QList<QObject*> m_objList;
    QByteArrayList m_propertyList;
};

#endif // PROPERTYGROUP_H
