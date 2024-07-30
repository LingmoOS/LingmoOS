/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DUMMY_INHIBITION_H
#define SOLID_DUMMY_INHIBITION_H

#include "backends/abstractinhibition.h"
#include "inhibition.h"
#include "solid/power.h"

#include <qglobal.h>

namespace Solid
{
class DummyInhibitionJob;
class DummyInhibition : public AbstractInhibition
{
    Q_OBJECT
public:
    explicit DummyInhibition(QObject *parent = nullptr);
    virtual ~DummyInhibition();

    void start() override;
    void stop() override;
    Inhibition::State state() const override;

    Power::InhibitionTypes inhibitions;
    QString description;
    Inhibition::State m_state;
};
}

#endif // SOLID_DUMMY_INHIBITION_H
