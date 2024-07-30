/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_COMMAND_RESULT_H
#define PLASMAVAULT_KDED_ENGINE_COMMAND_RESULT_H

#include <QFuture>

#include "asynqt/basic/all.h"
#include "asynqt/utils/expected.h"

namespace PlasmaVault
{
class Error
{
public:
    enum Code {
        MountPointError,
        DeviceError,
        BackendError,
        CommandError,
        DeletionError,
        UnknownError,
        OperationCancelled,
    };

    Error(Code code = UnknownError, const QString &message = {}, const QString &out = {}, const QString &err = {});

    Code code() const;
    QString message() const;
    QString out() const;
    QString err() const;

private:
    Code m_code;
    QString m_message;
    QString m_out;
    QString m_err;
};

template<typename T = void>
using Result = AsynQt::Expected<T, Error>;

template<typename T = void>
using FutureResult = QFuture<Result<T>>;

inline FutureResult<> errorResult(Error::Code error, const QString &message, const QString &out = {}, const QString &err = {})
{
    qWarning() << message;
    return makeReadyFuture(Result<>::error(error, message, out, err));
}

} // namespace PlasmaVault

#endif // include guard
