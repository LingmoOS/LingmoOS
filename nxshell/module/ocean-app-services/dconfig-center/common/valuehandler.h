// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VALUEHANDLER_H
#define VALUEHANDLER_H

#include <QString>
#include <QVariant>

class ConfigGetter {
public:
    virtual ~ConfigGetter();

    virtual QStringList keyList() const = 0;
    virtual void setValue(const QString &key, const QVariant &value) = 0;
    virtual QVariant value(const QString &key) const = 0;
    virtual void reset(const QString &key) = 0;
    virtual QString permissions(const QString &key) const = 0;
    virtual QString visibility(const QString &key) const = 0;
    virtual QString displayName(const QString &key, const QString &locale) = 0;
    virtual QString description(const QString &key, const QString &locale) = 0;
    virtual QString version() const = 0;
    virtual int flags(const QString &key) const = 0;

    virtual void release() = 0;
    virtual bool isDefaultValue(const QString &key) = 0;
};

class ValueHandler : public QObject
{
    Q_OBJECT
public:
    explicit ValueHandler(const QString &appid, const QString &fileName, const QString &subpath);
    explicit ValueHandler(int uid, const QString &appid, const QString &fileName, const QString &subpath);
    ~ValueHandler();

    ConfigGetter *createManager();
    int getUid() const;

Q_SIGNALS:
    void valueChanged(const QString &key);

public:
    int uid = -1;
    const QString appid;
    const QString fileName;
    const QString subpath;
};

#endif // VALUEHANDLER_H
