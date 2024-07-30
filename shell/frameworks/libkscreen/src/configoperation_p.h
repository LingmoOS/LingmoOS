/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QObject>

#include "abstractbackend.h"
#include "backendinterface.h"
#include "configoperation.h"

namespace KScreen
{
class ConfigOperationPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ConfigOperationPrivate(ConfigOperation *qq);
    ~ConfigOperationPrivate() override;

    // For out-of-process
    void requestBackend();
    virtual void backendReady(org::kde::kscreen::Backend *backend);

    // For in-process
    KScreen::AbstractBackend *loadBackend();

public Q_SLOTS:
    void doEmitResult();

private:
    QString error;
    bool isExec;

protected:
    ConfigOperation *const q_ptr;
    Q_DECLARE_PUBLIC(ConfigOperation)
};

}
