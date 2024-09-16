// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <dexpected.h>
#include <qobject.h>
#include <qscopedpointer.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DKbdBacklightPrivate;
enum class KbdSource;

class DKbdBacklight : public QObject
{
    Q_OBJECT
public:
    virtual ~DKbdBacklight();

signals:
    void brightnessChanged(const qint32 value);
    void brightnessChangedWithSource(const qint32 value, const KbdSource &source);

public slots:
    DExpected<qint32> brightness() const;
    DExpected<qint32> maxBrightness() const;
    DExpected<void> setBrightness(const qint32 value);

private:
    QScopedPointer<DKbdBacklightPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DKbdBacklight)

    explicit DKbdBacklight(QObject *parent = nullptr);
    Q_DISABLE_COPY(DKbdBacklight)

    friend class DPowerManager;
};

DPOWER_END_NAMESPACE
