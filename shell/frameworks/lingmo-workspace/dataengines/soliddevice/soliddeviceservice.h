/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <Lingmo5Support/Service>

class SolidDeviceEngine;

class SolidDeviceService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    SolidDeviceService(SolidDeviceEngine *parent, const QString &source);

protected:
    Lingmo5Support::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    SolidDeviceEngine *m_engine;
    QString m_dest;
};
