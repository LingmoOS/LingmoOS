/*
 * <one line to give the library's name and an idea of what it does.>
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QObject>

#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
class ConfigOperationPrivate;

class KSCREEN_EXPORT ConfigOperation : public QObject
{
    Q_OBJECT

public:
    enum Option {
        NoOptions,
        NoEDID,
    };
    Q_DECLARE_FLAGS(Options, Option)

    ~ConfigOperation() override;

    bool hasError() const;
    QString errorString() const;

    virtual KScreen::ConfigPtr config() const = 0;

    bool exec();

Q_SIGNALS:
    void finished(ConfigOperation *operation);

protected:
    explicit ConfigOperation(ConfigOperationPrivate *dd, QObject *parent = nullptr);

    void setError(const QString &error);
    void emitResult();

protected Q_SLOTS:
    virtual void start() = 0;

protected:
    ConfigOperationPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(ConfigOperation)
};
}
