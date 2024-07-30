// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PASSWORDPROVIDER_H_
#define PASSWORDPROVIDER_H_

#include "providerbase.h"

namespace LingmoPass
{

class PasswordsModel;
class PasswordProvider : public ProviderBase
{
    Q_OBJECT

    friend class PasswordsModel;

protected:
    using ProviderBase::ProviderBase;

    HandlingResult handleSecret(QStringView secret) override;
};

}
#endif
