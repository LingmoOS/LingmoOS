/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_QMLABSTRACTDATASOURCE_H
#define KUSERFEEDBACK_QMLABSTRACTDATASOURCE_H

#include <provider.h>

namespace KUserFeedback {

class AbstractDataSource;

class QmlAbstractDataSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KUserFeedback::Provider::TelemetryMode mode READ telemetryMode
               WRITE setTelemetryMode NOTIFY telemetryModeChanged)
public:
    explicit QmlAbstractDataSource(AbstractDataSource *source, QObject *parent);
    ~QmlAbstractDataSource() override;

    Provider::TelemetryMode telemetryMode() const;
    void setTelemetryMode(Provider::TelemetryMode mode);

    AbstractDataSource* source() const;

Q_SIGNALS:
    void telemetryModeChanged();

private:
    AbstractDataSource *m_source;
};

}

#endif // KUSERFEEDBACK_QMLABSTRACTDATASOURCE_H
