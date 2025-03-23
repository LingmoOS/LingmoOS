// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONFIGWRAPPER_H
#define DCONFIGWRAPPER_H

#include <dtkdeclarative_global.h>
#include <QQmlParserStatus>
#include <QtQml/qqml.h>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class DConfigWrapperMetaObject;
class DConfigWrapper : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString subpath READ subpath WRITE setSubpath)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(Config)
#endif
public:
    explicit DConfigWrapper(QObject *parent = nullptr);
    ~DConfigWrapper() override;

    QString name() const;
    void setName(const QString &name);

    QString subpath() const;
    void setSubpath(const QString &subpath);

public Q_SLOTS:
    QVariant value(const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    void resetValue(const QString &key);
    QStringList keyList() const;
    bool isValid() const;

Q_SIGNALS:
    void valueChanged(const QString &key);

public:
    virtual void classBegin() override;
    virtual void componentComplete() override;

private:
    friend DConfigWrapperMetaObject;
    DTK_CORE_NAMESPACE::DConfig *impl;
    QString m_name;
    QString m_subpath;
    Q_DISABLE_COPY(DConfigWrapper)
};

#endif // DCONFIGWRAPPER_H
