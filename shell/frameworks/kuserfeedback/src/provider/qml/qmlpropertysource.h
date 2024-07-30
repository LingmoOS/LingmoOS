/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_QMLPROPERTYSOURCE_H
#define KUSERFEEDBACK_QMLPROPERTYSOURCE_H

#include "qmlabstractdatasource.h"
#include <QVariant>

class CustomPropertySource;

namespace KUserFeedback {

/*! Data source reporting information provided by a QML application
 */
class QmlPropertySource : public QmlAbstractDataSource
{
    Q_OBJECT
    Q_PROPERTY(QString sourceId READ sourceId WRITE setSourceId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QVariant data READ data WRITE setData NOTIFY dataChanged)
public:
    QmlPropertySource(QObject* parent = nullptr);

    QString sourceId() const;
    QString name() const;
    QString description() const;
    QVariant data() const;

    void setSourceId(const QString &id);
    void setName(const QString &name);
    void setDescription(const QString &description);
    void setData(const QVariant &data);

Q_SIGNALS:
    void idChanged(const QString &id);
    void nameChanged(const QString &name);
    void descriptionChanged(const QString &description);
    void dataChanged(const QVariant &data);

private:
    CustomPropertySource* customSource();
};
}

#endif // KUSERFEEDBACK_QTVERSIONSOURCE_H
