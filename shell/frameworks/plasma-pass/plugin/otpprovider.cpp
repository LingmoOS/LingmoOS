// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "otpprovider.h"

#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QDateTime>

#include <KLocalizedString>

#include <chrono>

#include <liboath/oath.h>

using namespace PlasmaPass;
using namespace std::chrono_literals;

namespace {

static const QString otpAuthSchema = QStringLiteral("otpauth://");
static const QString secretQueryItem = QStringLiteral("secret");

QString parseOtpType(const QUrl &url)
{
    return url.host();
}

} // namespace

OTPProvider::OTPProvider(const QString &path, QObject *parent)
    : ProviderBase(path, parent)
{
    setSecretTimeout(30s);
}

ProviderBase::HandlingResult OTPProvider::handleSecret(QStringView secret)
{
    if (!secret.startsWith(otpAuthSchema)) {
        return HandlingResult::Continue;
    }

    QUrl url(secret.toString());
    const auto otpType = parseOtpType(url);
    if (otpType == QLatin1String("totp")) {
        handleTOTP(url);
    } else {
        setError(i18n("Unsupported OTP type %1", otpType));
        return HandlingResult::Stop;
    }

    return HandlingResult::Stop;
}


void OTPProvider::handleTOTP(const QUrl &url)
{
    const QUrlQuery query(url.query());
    const auto secret = query.queryItemValue(secretQueryItem).toUtf8();

    char *decodedSecret = {};
    size_t decodedSecretLen = 0;
    oath_base32_decode(secret.data(), secret.size(), &decodedSecret, &decodedSecretLen);

    char output_otp[6] = {};
    oath_totp_generate(decodedSecret, decodedSecretLen,
            QDateTime::currentDateTime().toSecsSinceEpoch(),
            OATH_TOTP_DEFAULT_TIME_STEP_SIZE,
            OATH_TOTP_DEFAULT_START_TIME,
            6,
            output_otp);

    setSecret(QString::fromLatin1(output_otp, sizeof(output_otp)));
}

#include "moc_otpprovider.cpp"
