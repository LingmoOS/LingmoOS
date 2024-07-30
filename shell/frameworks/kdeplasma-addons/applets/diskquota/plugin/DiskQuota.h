/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef PLASMA_DISK_QUOTA_H
#define PLASMA_DISK_QUOTA_H

#include "QuotaListModel.h"
#include <QObject>
#include <QProcess>
class QTimer;

/**
 * Class monitoring the file system quota.
 * The monitoring is performed through a timer, running the 'quota'
 * command line tool.
 */
class DiskQuota : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool quotaInstalled READ quotaInstalled NOTIFY quotaInstalledChanged)
    Q_PROPERTY(bool cleanUpToolInstalled READ cleanUpToolInstalled NOTIFY cleanUpToolInstalledChanged)

    Q_PROPERTY(TrayStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip NOTIFY subToolTipChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)

    Q_PROPERTY(QuotaListModel *model READ model CONSTANT)

public:
    explicit DiskQuota(QObject *parent = nullptr);

public:
    /**
     * System tray icon states.
     */
    enum TrayStatus {
        ActiveStatus = 0,
        PassiveStatus,
        NeedsAttentionStatus,
    };
    Q_ENUM(TrayStatus)

public:
    bool quotaInstalled() const;
    void setQuotaInstalled(bool installed);

    bool cleanUpToolInstalled() const;
    void setCleanUpToolInstalled(bool installed);

    TrayStatus status() const;
    void setStatus(TrayStatus status);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString subToolTip() const;
    void setSubToolTip(const QString &subToolTip);

    QString iconName() const;
    void setIconName(const QString &name);

    /**
     * Getter function for the model that is used in QML.
     */
    QuotaListModel *model() const;

public Q_SLOTS:
    /**
     * Called every timer timeout to update the data model.
     * Launches an asynchronous 'quota' process to obtain data,
     * and finally calls quotaProcessFinished().
     */
    void updateQuota();

    /**
     * Processes the quota data from the 'quota' process.
     */
    void quotaProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * Opens the cleanup tool (filelight) at the folder @p mountPoint.
     */
    void openCleanUpTool(const QString &mountPoint);

Q_SIGNALS:
    void quotaInstalledChanged();
    void cleanUpToolInstalledChanged();
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();

private:
    QTimer *m_timer = nullptr;
    QProcess *m_quotaProcess = nullptr;
    bool m_quotaInstalled = true;
    bool m_cleanUpToolInstalled = true;
    TrayStatus m_status = PassiveStatus;
    QString m_iconName = QStringLiteral("disk-quota");
    QString m_toolTip;
    QString m_subToolTip;
    QuotaListModel *m_model = nullptr;
};

#endif // PLASMA_DISK_QUOTA_H
