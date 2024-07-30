/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "qmlpropertysource.h"
#include <abstractdatasource.h>

using namespace KUserFeedback;

class CustomPropertySource : public AbstractDataSource
{
public:
    CustomPropertySource() : AbstractDataSource({}) {}

    QVariant data() override { return m_data; }
    QString name() const override { return m_name; }
    QString description() const override { return m_description; }

    void setSourceId(const QString &id) { setId(id); }

    QVariant m_data;
    QString m_name;
    QString m_description;
};

QmlPropertySource::QmlPropertySource(QObject* parent)
    : QmlAbstractDataSource(new CustomPropertySource(), parent)
{
}

QVariant QmlPropertySource::data() const
{
    return source()->data();
}

QString QmlPropertySource::name() const
{
    return source()->name();
}

QString QmlPropertySource::description() const
{
    return source()->description();
}

QString QmlPropertySource::sourceId() const
{
    return source()->id();
}

void QmlPropertySource::setData(const QVariant& data)
{
    if (customSource()->m_data != data) {
        customSource()->m_data = data;
        Q_EMIT dataChanged(data);
    }
}

void QmlPropertySource::setName(const QString &name)
{
    if (customSource()->m_name != name) {
        customSource()->m_name = name;
        Q_EMIT nameChanged(name);
    }
}

void QmlPropertySource::setDescription(const QString& description)
{
    if (customSource()->m_description != description) {
        customSource()->m_description = description;
        Q_EMIT descriptionChanged(description);
    }
}

void QmlPropertySource::setSourceId(const QString& id)
{
    if (id != source()->id()) {
        customSource()->setSourceId(id);
        Q_EMIT idChanged(id);
    }
}

CustomPropertySource * KUserFeedback::QmlPropertySource::customSource()
{
    return dynamic_cast<CustomPropertySource*>(source());
}

#include "moc_qmlpropertysource.cpp"
