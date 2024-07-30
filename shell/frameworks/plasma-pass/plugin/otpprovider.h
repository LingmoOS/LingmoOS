// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef OTPPROVIDER_H_
#define OTPPROVIDER_H_

#include "providerbase.h"

namespace PlasmaPass
{
class PasswordsModel;

class OTPProvider : public ProviderBase
{
    Q_OBJECT

    friend class PasswordsModel;
protected:
    explicit OTPProvider(const QString &path, QObject *parent = nullptr);

    HandlingResult handleSecret(QStringView secret) override;

private:
    void handleTOTP(const QUrl &url);
};

}

#endif // OTPPROVIDER_H_
