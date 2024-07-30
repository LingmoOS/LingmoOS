/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "qmlabstractdatasource.h"

#include <KUserFeedback/AbstractDataSource>

using namespace KUserFeedback;

QmlAbstractDataSource::QmlAbstractDataSource(AbstractDataSource *source, QObject* parent)
    : QObject(parent)
    , m_source(source)
{
}

QmlAbstractDataSource::~QmlAbstractDataSource()
{
}

Provider::TelemetryMode QmlAbstractDataSource::telemetryMode() const
{
    return m_source->telemetryMode();
}

void QmlAbstractDataSource::setTelemetryMode(Provider::TelemetryMode mode)
{
    if (m_source->telemetryMode() == mode)
        return;
    m_source->setTelemetryMode(mode);
    Q_EMIT telemetryModeChanged();
}

AbstractDataSource* QmlAbstractDataSource::source() const
{
    return m_source;
}

#include "moc_qmlabstractdatasource.cpp"
