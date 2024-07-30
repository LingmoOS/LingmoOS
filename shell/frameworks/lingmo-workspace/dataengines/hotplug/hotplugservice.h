/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <Lingmo5Support/Service>

class HotplugEngine;

class HotplugService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    HotplugService(HotplugEngine *parent, const QString &source);

protected:
    Lingmo5Support::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    HotplugEngine *m_engine;
    QString m_dest;
};
