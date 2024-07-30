/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_QMLPROPERTYRATIOSOURCE_H
#define KUSERFEEDBACK_QMLPROPERTYRATIOSOURCE_H

#include "qmlabstractdatasource.h"

namespace KUserFeedback {

class PropertyRatioSource;

class QmlPropertyRatioSource : public QmlAbstractDataSource
{
    Q_OBJECT
    Q_PROPERTY(QString sourceId READ sourceId WRITE setSourceId NOTIFY changed)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY changed)
    Q_PROPERTY(QObject* object READ object WRITE setObject NOTIFY changed)
    Q_PROPERTY(QString propertyName READ propertyName WRITE setPropertyName NOTIFY changed)
public:
    explicit QmlPropertyRatioSource(QObject *parent = nullptr);
    ~QmlPropertyRatioSource() override;

    QString sourceId() const;
    void setSourceId(const QString &id);

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &desc);

    QObject* object() const;
    void setObject(QObject *object);

    QString propertyName() const;
    void setPropertyName(const QString &name);

    Q_INVOKABLE void addValueMapping(const QVariant &value, const QString &name);

Q_SIGNALS:
    void changed();

private:
    PropertyRatioSource* prSrc() const;
};

}

#endif // KUSERFEEDBACK_QMLPROPERTYRATIOSOURCE_H
