// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSERVICEBASE_H
#define DSERVICEBASE_H

#include <QObject>
#include <QDBusContext>

/**
 * @brief The serviceBase class     服务基类
 */
class DServiceBase : public QObject
    , protected QDBusContext
{
    Q_OBJECT
public:
    explicit DServiceBase(const QString &path, const QString &interface, QObject *parent = nullptr);
    QString getPath() const;
    QString getInterface() const;

protected:
    QString getClientName();
    bool clientWhite(const int index);
    void notifyPropertyChanged(const QString &interface, const QString &propertyName);

private:
    QString m_path;
    QString m_interface;
};

#endif // DSERVICEBASE_H
