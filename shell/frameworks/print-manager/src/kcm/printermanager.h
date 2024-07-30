/**
 * SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PRINTERMANAGER_H
#define PRINTERMANAGER_H

#include <KQuickConfigModule>
#include <QObject>

namespace PMTypes
{
Q_NAMESPACE
enum PPDType { Manual = 0, Auto, Custom };
Q_ENUM_NS(PPDType)
}

class KCupsRequest;

class PrinterManager : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QString osName READ osName CONSTANT FINAL)
    Q_PROPERTY(QString osBugReportUrl READ osBugReportUrl CONSTANT FINAL)

    Q_PROPERTY(QVariantList remotePrinters READ remotePrinters NOTIFY remotePrintersLoaded)

    Q_PROPERTY(QVariantList recommendedDrivers READ recommendedDrivers NOTIFY recommendedDriversLoaded)

    Q_PROPERTY(QVariantMap serverSettings READ serverSettings NOTIFY serverSettingsChanged)

    Q_PROPERTY(bool serverSettingsLoaded READ serverSettingsLoaded NOTIFY serverSettingsChanged)

    Q_PROPERTY(bool shareConnectedPrinters READ shareConnectedPrinters NOTIFY serverSettingsChanged)

    Q_PROPERTY(bool allowPrintingFromInternet READ allowPrintingFromInternet NOTIFY serverSettingsChanged)

    Q_PROPERTY(bool allowRemoteAdmin READ allowRemoteAdmin NOTIFY serverSettingsChanged)

    Q_PROPERTY(bool allowUserCancelAnyJobs READ allowUserCancelAnyJobs NOTIFY serverSettingsChanged)

public:
    PrinterManager(QObject *parent, const KPluginMetaData &metaData);

    Q_INVOKABLE void removePrinter(const QString &name);

    Q_INVOKABLE void makePrinterDefault(const QString &name);
    Q_INVOKABLE void makePrinterShared(const QString &name, bool shared, bool isClass);
    Q_INVOKABLE void makePrinterRejectJobs(const QString &name, bool reject);

    Q_INVOKABLE void printTestPage(const QString &name, bool isClass);
    Q_INVOKABLE void printSelfTestPage(const QString &name);
    Q_INVOKABLE void cleanPrintHeads(const QString &name);

    Q_INVOKABLE void pausePrinter(const QString &name);
    Q_INVOKABLE void resumePrinter(const QString &name);
    Q_INVOKABLE static bool isIPPCapable(const QString &uri);
    Q_INVOKABLE static bool isSCPAvailable();

    QString osName() const;
    QString osBugReportUrl() const;

    bool shareConnectedPrinters() const;
    bool allowPrintingFromInternet() const;
    bool allowRemoteAdmin() const;
    bool allowUserCancelAnyJobs() const;

public Q_SLOTS:
    void savePrinter(const QString &name, const QVariantMap &saveArgs, bool isClass);
    QVariantMap getPrinterPPD(const QString &name);
    void getServerSettings();
    void saveServerSettings(const QVariantMap &settings);

    void getRemotePrinters(const QString &uri, const QString &uriScheme);
    void getRecommendedDrivers(const QString &deviceId, const QString &makeAndModel, const QString &deviceUri);
    void clearRemotePrinters();
    void clearRecommendedDrivers();

Q_SIGNALS:
    void requestError(const QString &errorMessage);
    void removeDone();
    void saveDone();
    void serverSettingsChanged();
    void serverStopped();
    void serverStarted();
    void remotePrintersLoaded();
    void recommendedDriversLoaded();

private:
    KCupsRequest *setupRequest(std::function<void()> finished = []() {});
    QVariantList remotePrinters() const;
    QVariantList recommendedDrivers() const;
    QVariantMap serverSettings() const;
    bool serverSettingsLoaded() const;
    void initOSRelease();

    QVariantMap m_serverSettings;
    bool m_serverSettingsLoaded = false;
    QVariantList m_remotePrinters;
    QVariantList m_recommendedDrivers;
    QString m_osName;
    QString m_osBugReportUrl;
};

#endif
