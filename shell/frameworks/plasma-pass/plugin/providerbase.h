// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PROVIDERBASE_H_
#define PROVIDERBASE_H_

#include <QObject>
#include <QTimer>

#include "klipperutils.h"

#include <memory>

class QDBusPendingCallWatcher;
class KJob;
class QMimeData;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace Plasma
#else
namespace Plasma5Support
#endif
{
class DataEngineConsumer;
}

namespace PlasmaPass
{
class PasswordsModel;

class ProviderBase : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(int timeout READ timeout NOTIFY timeoutChanged)
    Q_PROPERTY(int defaultTimeout READ defaultTimeout CONSTANT)
    Q_PROPERTY(QString secret READ secret NOTIFY secretChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    friend class PasswordsModel;
public:
    ~ProviderBase() override;

    QString secret() const;
    bool isValid() const;
    int timeout() const;
    int defaultTimeout() const; // in milliseconds
    bool hasError() const;
    QString error() const;

public Q_SLOTS:
    void reset();

Q_SIGNALS:
    void secretChanged();
    void validChanged();
    void timeoutChanged();
    void errorChanged();

protected:
    explicit ProviderBase(const QString &path, QObject *parent = nullptr);

    void setSecret(const QString &secret);
    void setSecretTimeout(std::chrono::seconds timeout);
    void setError(const QString &error);

    enum class HandlingResult {
        Continue,
        Stop
    };
    virtual HandlingResult handleSecret(QStringView secret) = 0;

private Q_SLOTS:
    void start();
    void onPlasmaServiceRemovePasswordResult(KJob *job);

private:
    void expireSecret();

    void removePasswordFromClipboard(const QString &password);
    static void clearClipboard();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    std::unique_ptr<Plasma::DataEngineConsumer> mEngineConsumer;
#else
    std::unique_ptr<Plasma5Support::DataEngineConsumer> mEngineConsumer;
#endif
    QString mPath;
    QString mError;
    QString mSecret;
    QTimer mTimer;
    int mTimeout = 0;
    std::chrono::seconds mSecretTimeout;

    static KlipperUtils::State sKlipperState;
};

}
#endif
