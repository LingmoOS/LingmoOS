// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkdeclarative_global.h>

#include <QJsonValue>
#include <private/qqmlobjectmodel_p.h>

DQUICK_BEGIN_NAMESPACE

class ObjectModelProxyPrivate;
class ObjectModelProxy : public QQmlInstanceModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlObjectModel *sourceModel READ sourceModel WRITE setSourceModel)
    Q_PROPERTY(QJSValue filterAcceptsItem READ filterAcceptsItem WRITE setFilterAcceptsItem NOTIFY filterAcceptsItemChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(ObjectModelProxy)
#endif
public:
    explicit ObjectModelProxy(QObject *parent = nullptr);

public:
    bool isValid() const override;
    virtual QObject *object(int index, QQmlIncubator::IncubationMode incubationMode) override;
    virtual int count() const override;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    virtual QVariant variantValue(int index, const QString &name) override;
    virtual ReleaseFlags release(QObject *object, ReusableFlag reusableFlag = NotReusable) override;
#else
    virtual QString stringValue(int index, const QString &name) override;
    virtual ReleaseFlags release(QObject *object) override;
#endif
    virtual void setWatchedRoles(const QList<QByteArray> &roles) override;
    virtual QQmlIncubator::Status incubationStatus(int index) override;
    virtual int indexOf(QObject *object, QObject *objectContext) const override;

    QQmlObjectModel *sourceModel() const;
    void setSourceModel(QQmlObjectModel * sourceModel);
    QJSValue filterAcceptsItem() const;
    void setFilterAcceptsItem(QJSValue filterAcceptsItem);

public Q_SLOTS:
    void update();
    QObject *get(const int i) const;
    int mapToSource(const int i) const;
    int mapFromSource(const int i) const;

private Q_SLOTS:
    void syncDataFromItem();

Q_SIGNALS:
    void filterAcceptsItemChanged();

private:
    Q_DECLARE_PRIVATE(ObjectModelProxy)
};

DQUICK_END_NAMESPACE
