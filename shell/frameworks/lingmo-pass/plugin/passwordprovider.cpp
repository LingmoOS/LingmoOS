// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "passwordprovider.h"

using namespace LingmoPass;

ProviderBase::HandlingResult PasswordProvider::handleSecret(QStringView secret)
{
    setSecret(secret.toString());
    // We are only interested in the first line for passwords
    return HandlingResult::Stop;
}

#include "moc_passwordprovider.cpp"
