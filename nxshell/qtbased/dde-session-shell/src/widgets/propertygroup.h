// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
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
    // todo
    // Q_PROPERTY(bool enable ...)
public:
    explicit PropertyGroup(QObject *parent = nullptr);

    int addObject(QObject *obj);
    void removeObject(QObject *obj);
    // todo
    // removeObject(int index)
    // takeObject(int index);
    // QList<QObject*> objectList();
    void addProperty(const QByteArray &propertyName);
    void removeProperty(const QByteArray &propertyName);
    // todo
    // QByteArrayList propertyList() const;

private:
    void onObjectPropertyChanged(QObject *obj);

    QList<QObject*> m_objList;
    QByteArrayList m_propertyList;
    QMap<QByteArray, QSignalMapper*> m_signalMapperMap;
};

#endif // PROPERTYGROUP_H
