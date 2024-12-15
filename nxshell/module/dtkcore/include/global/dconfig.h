// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONFIG_H
#define DCONFIG_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>
#include <QVariant>

DCORE_BEGIN_NAMESPACE
class DConfigBackend {
public:
    virtual ~DConfigBackend();
    virtual bool isValid() const = 0;
    virtual bool load(const QString &/*appId*/) = 0;
    virtual QStringList keyList() const = 0;
    virtual QVariant value(const QString &/*key*/, const QVariant &/*fallback*/) const = 0;
    virtual void setValue(const QString &/*key*/, const QVariant &/*value*/) = 0;
    virtual void reset(const QString &key) { setValue(key, QVariant());}
    virtual QString name() const {return QString("");}
    virtual bool isDefaultValue(const QString &/*key*/) const { return true; }
};

class DConfigPrivate;
class LIBDTKCORESHARED_EXPORT DConfig : public QObject, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DConfig)

    Q_PROPERTY(QStringList keyList READ keyList FINAL)

public:
    explicit DConfig(const QString &name, const QString &subpath = QString(),
                     QObject *parent = nullptr);

    explicit DConfig(DConfigBackend *backend, const QString &name, const QString &subpath = QString(),
                     QObject *parent = nullptr);

    static DConfig *create(const QString &appId, const QString &name, const QString &subpath = QString(),
                           QObject *parent = nullptr);
    static DConfig *create(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath = QString(),
                                  QObject *parent = nullptr);
    static DConfig *createGeneric(const QString &name, const QString &subpath = QString(),
                                  QObject *parent = nullptr);
    static DConfig *createGeneric(DConfigBackend *backend, const QString &name, const QString &subpath = QString(),
                                  QObject *parent = nullptr);

    static void setAppId(const QString &appId);

    QString backendName() const;

    QStringList keyList() const;

    bool isValid() const;
    bool isDefaultValue(const QString &key) const;
    QVariant value(const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    void reset(const QString &key);

    QString name() const;
    QString subpath() const;

Q_SIGNALS:
    void valueChanged(const QString &key);

private:
    explicit DConfig(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath,
                     QObject *parent = nullptr);
};

DCORE_END_NAMESPACE

#endif // DCONFIG_H
