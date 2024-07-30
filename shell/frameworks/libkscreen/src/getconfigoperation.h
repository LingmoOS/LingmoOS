/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include "configoperation.h"
#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
class GetConfigOperationPrivate;

class KSCREEN_EXPORT GetConfigOperation : public KScreen::ConfigOperation
{
    Q_OBJECT

public:
    explicit GetConfigOperation(Options options = NoOptions, QObject *parent = nullptr);
    ~GetConfigOperation() override;

    KScreen::ConfigPtr config() const override;

protected:
    void start() override;

private:
    Q_DECLARE_PRIVATE(GetConfigOperation)
};
}
