/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPSCONNECTION_H
#define KCUPSCONNECTION_H

#include <QMetaMethod>
#include <QMutex>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>
#include <QWidget>

#include <kcupslib_export.h>

#include <cups/cups.h>

#define KCUPS_DEVICE_CLASS QLatin1String("device-class")
#define KCUPS_DEVICE_ID QLatin1String("device-id")
#define KCUPS_DEVICE_INFO QLatin1String("device-info")
#define KCUPS_DEVICE_MAKE_AND_MODEL QLatin1String("device-make-and-model")
#define KCUPS_DEVICE_LOCATION QLatin1String("device-location")
#define KCUPS_DEVICE_URI QLatin1String("device-uri")

#define KCUPS_PRINTER_NAME QLatin1String("printer-name")
#define KCUPS_PRINTER_LOCATION QLatin1String("printer-location")
#define KCUPS_PRINTER_INFO QLatin1String("printer-info")
#define KCUPS_PRINTER_URI QLatin1String("printer-uri")
#define KCUPS_PRINTER_MAKE_AND_MODEL QLatin1String("printer-make-and-model")
#define KCUPS_PRINTER_STATE QLatin1String("printer-state")
#define KCUPS_PRINTER_STATE_MESSAGE QLatin1String("printer-state-message")
#define KCUPS_PRINTER_IS_SHARED QLatin1String("printer-is-shared")
#define KCUPS_PRINTER_IS_ACCEPTING_JOBS QLatin1String("printer-is-accepting-jobs")
#define KCUPS_PRINTER_TYPE QLatin1String("printer-type")
#define KCUPS_PRINTER_TYPE_MASK QLatin1String("printer-type-mask")
#define KCUPS_PRINTER_COMMANDS QLatin1String("printer-commands")
#define KCUPS_PRINTER_URI_SUPPORTED QLatin1String("printer-uri-supported")
#define KCUPS_PRINTER_ERROR_POLICY QLatin1String("printer-error-policy")
#define KCUPS_PRINTER_ERROR_POLICY_SUPPORTED QLatin1String("printer-error-policy-supported")
#define KCUPS_PRINTER_OP_POLICY QLatin1String("printer-op-policy")
#define KCUPS_PRINTER_OP_POLICY_SUPPORTED QLatin1String("printer-op-policy-supported")

#define KCUPS_MEMBER_URIS QLatin1String("member-uris")
#define KCUPS_MEMBER_NAMES QLatin1String("member-names")

#define KCUPS_MARKER_CHANGE_TIME QLatin1String("marker-change-time")
#define KCUPS_MARKER_COLORS QLatin1String("marker-colors")
#define KCUPS_MARKER_LEVELS QLatin1String("marker-levels")
#define KCUPS_MARKER_HIGH_LEVELS "marker-high-levels"
#define KCUPS_MARKER_LOW_LEVELS "marker-low-levels"
#define KCUPS_MARKER_NAMES QLatin1String("marker-names")
#define KCUPS_MARKER_TYPES QLatin1String("marker-types")
#define KCUPS_MARKER_MESSAGE "marker-message"

#define KCUPS_JOB_ID QLatin1String("job-id")
#define KCUPS_JOB_NAME QLatin1String("job-name")
#define KCUPS_JOB_K_OCTETS QLatin1String("job-k-octets")
#define KCUPS_JOB_K_OCTETS_PROCESSED QLatin1String("job-k-octets-processed")
#define KCUPS_JOB_PRINTER_URI QLatin1String("job-printer-uri")
#define KCUPS_JOB_PRINTER_STATE_MESSAGE QLatin1String("job-printer-state-message")
#define KCUPS_JOB_ORIGINATING_USER_NAME QLatin1String("job-originating-user-name")
#define KCUPS_JOB_ORIGINATING_HOST_NAME QLatin1String("job-originating-host-name")
#define KCUPS_JOB_MEDIA_PROGRESS QLatin1String("job-media-progress")
#define KCUPS_JOB_MEDIA_SHEETS QLatin1String("job-media-sheets")
#define KCUPS_JOB_MEDIA_SHEETS_COMPLETED QLatin1String("job-media-sheets-completed")
#define KCUPS_JOB_PRESERVED QLatin1String("job-preserved")
#define KCUPS_JOB_STATE QLatin1String("job-state")
#define KCUPS_JOB_STATE_REASONS QLatin1String("job-state-reasons")
#define KCUPS_JOB_HOLD_UNTIL QLatin1String("job-hold-until")
#define KCUPS_JOB_SHEETS_DEFAULT QLatin1String("job-sheets-default")
#define KCUPS_JOB_SHEETS_SUPPORTED QLatin1String("job-sheets-supported")
#define KCUPS_JOB_SHEETS_DEFAULT QLatin1String("job-sheets-default")
#define KCUPS_JOB_SHEETS_SUPPORTED QLatin1String("job-sheets-supported")

#define KCUPS_MY_JOBS QLatin1String("my-jobs")
#define KCUPS_WHICH_JOBS QLatin1String("which-jobs")

#define KCUPS_TIME_AT_COMPLETED QLatin1String("time-at-completed")
#define KCUPS_TIME_AT_CREATION QLatin1String("time-at-creation")
#define KCUPS_TIME_AT_PROCESSING QLatin1String("time-at-processing")

#define KCUPS_REQUESTED_ATTRIBUTES QLatin1String("requested-attributes")

#define KCUPS_REQUESTING_USER_NAME QLatin1String("requesting-user-name")
#define KCUPS_REQUESTING_USER_NAME_ALLOWED QLatin1String("requesting-user-name-allowed")
#define KCUPS_REQUESTING_USER_NAME_DENIED QLatin1String("requesting-user-name-denied")

#define KCUPS_PPD_MAKE_AND_MODEL QLatin1String("ppd-make-and-model")

#define KCUPS_NOTIFY_EVENTS QLatin1String("notify-events")
#define KCUPS_NOTIFY_PULL_METHOD QLatin1String("notify-pull-method")
#define KCUPS_NOTIFY_RECIPIENT_URI QLatin1String("notify-recipient-uri")
#define KCUPS_NOTIFY_LEASE_DURATION QLatin1String("notify-lease-duration")
#define KCUPS_NOTIFY_SUBSCRIPTION_ID QLatin1String("notify-subscription-id")

#define KCUPS_AUTH_INFO QLatin1String("auth-info")
#define KCUPS_AUTH_INFO_REQUIRED QLatin1String("auth-info-required")

typedef QList<QVariantMap> ReturnArguments;

class KIppRequest;
class KCupsPasswordDialog;
class KCUPSLIB_EXPORT KCupsConnection : public QThread
{
    Q_OBJECT
public:
    /**
     * This is the main Cups class @author Daniel Nicoletti <dantti12@gmail.com>
     *
     * By calling KCupsConnection::global() you have access to it.
     * Due to cups architecture, this class has to live on a
     * separate thread so we avoid blocking the user interface when
     * the cups call blocks.
     *
     * It is IMPORTANT that we do not create several thread
     * for each cups request, doing so is a valid but breaks our
     * authentication. We could tho store the user information an
     * set the user/password every time it was needed. But I am not
     * sure this is safe.
     *
     * Extending this means either adding methods to the KCupsRequest
     * class which will move to this thread and then run.
     */
    static KCupsConnection *global();

    /**
     * @brief KCupsConnection
     * @param parent
     *
     * This is the default constructor that connects to the default server
     * If you don't have any special reason for creating a connection
     * on your own consider calling global()
     */
    explicit KCupsConnection(QObject *parent = nullptr);
    explicit KCupsConnection(const QUrl &server, QObject *parent = nullptr);
    ~KCupsConnection() override;

    void setPasswordMainWindow(WId mainwindow);

Q_SIGNALS:
    /**
     * emitted when "server-started" is registered
     */
    void serverStarted(const QString &text);

    /**
     * emitted when "server-stopped" is registered
     */
    void serverStopped(const QString &text);

    /**
     * emitted when "server-restarted" is registered
     */
    void serverRestarted(const QString &text);

    /**
     * emitted when "server-audit" is registered
     */
    void serverAudit(const QString &text);

    /**
     * emitted when "printer-added" is registered
     */
    void printerAdded(const QString &text,
                      const QString &printerUri,
                      const QString &printerName,
                      uint printerState,
                      const QString &printerStateReasons,
                      bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-modified" is registered
     */
    void printerModified(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-deleted" is registered
     */
    void printerDeleted(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-state-changed" is registered
     */
    void printerStateChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-stopped" is registered
     */
    void printerStopped(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-restarted" is registered
     */
    void printerRestarted(const QString &text,
                          const QString &printerUri,
                          const QString &printerName,
                          uint printerState,
                          const QString &printerStateReasons,
                          bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-shutdown" is registered
     */
    void printerShutdown(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-media-changed" is registered
     */
    void printerMediaChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-finishings-changed" is registered
     */
    void printerFinishingsChanged(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs);

    /**
     * emitted when "job-state-changed" is registered
     */
    void jobState(const QString &text,
                  const QString &printerUri,
                  const QString &printerName,
                  uint printerState,
                  const QString &printerStateReasons,
                  bool printerIsAcceptingJobs,
                  uint jobId,
                  uint jobState,
                  const QString &jobStateReasons,
                  const QString &jobName,
                  uint jobImpressionsCompleted);

    /**
     * emitted when "job-created" is registered
     */
    void jobCreated(const QString &text,
                    const QString &printerUri,
                    const QString &printerName,
                    uint printerState,
                    const QString &printerStateReasons,
                    bool printerIsAcceptingJobs,
                    uint jobId,
                    uint jobState,
                    const QString &jobStateReasons,
                    const QString &jobName,
                    uint jobImpressionsCompleted);

    /**
     * emitted when "job-stopped" is registered
     */
    void jobStopped(const QString &text,
                    const QString &printerUri,
                    const QString &printerName,
                    uint printerState,
                    const QString &printerStateReasons,
                    bool printerIsAcceptingJobs,
                    uint jobId,
                    uint jobState,
                    const QString &jobStateReasons,
                    const QString &jobName,
                    uint jobImpressionsCompleted);

    /**
     * emitted when "job-config-changed" is registered
     */
    void jobConfigChanged(const QString &text,
                          const QString &printerUri,
                          const QString &printerName,
                          uint printerState,
                          const QString &printerStateReasons,
                          bool printerIsAcceptingJobs,
                          uint jobId,
                          uint jobState,
                          const QString &jobStateReasons,
                          const QString &jobName,
                          uint jobImpressionsCompleted);

    /**
     * emitted when "job-progress" is registered
     */
    void jobProgress(const QString &text,
                     const QString &printerUri,
                     const QString &printerName,
                     uint printerState,
                     const QString &printerStateReasons,
                     bool printerIsAcceptingJobs,
                     uint jobId,
                     uint jobState,
                     const QString &jobStateReasons,
                     const QString &jobName,
                     uint jobImpressionsCompleted);

    /**
     * emitted when "job-completed" is registered
     */
    void jobCompleted(const QString &text,
                      const QString &printerUri,
                      const QString &printerName,
                      uint printerState,
                      const QString &printerStateReasons,
                      bool printerIsAcceptingJobs,
                      uint jobId,
                      uint jobState,
                      const QString &jobStateReasons,
                      const QString &jobName,
                      uint jobImpressionsCompleted);

    void rhPrinterAdded(const QString &queueName);
    void rhPrinterRemoved(const QString &queueName);
    void rhQueueChanged(const QString &queueName);
    void rhJobQueuedLocal(const QString &queueName, uint jobId, const QString &jobOwner);
    void rhJobStartedLocal(const QString &queueName, uint jobId, const QString &jobOwner);

protected:
    friend class KCupsRequest;

    virtual void run() override;
    bool readyToStart();
    bool retry(const char *resource, int operation) const;
    ReturnArguments request(const KIppRequest &request, ipp_tag_t groupTag = IPP_TAG_ZERO) const;

private slots:
    void updateSubscription();
    void renewDBusSubscription();
    void cancelDBusSubscription();

protected:
    virtual void connectNotify(const QMetaMethod &signal) override;
    virtual void disconnectNotify(const QMetaMethod &signal) override;
    QString eventForSignal(const QMetaMethod &signal) const;

private:
    void init();

    int renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events = QStringList());

    void notifierConnect(const QString &signal, QObject *receiver, const char *slot);

    static ReturnArguments parseIPPVars(ipp_t *response, ipp_tag_t group_tag);
    static QVariant ippAttrToVariant(ipp_attribute_t *attr);

    static KCupsConnection *m_instance;

    bool m_inited = false;
    KCupsPasswordDialog *m_passwordDialog;
    QUrl m_serverUrl;

    QTimer *m_subscriptionTimer;
    QTimer *m_renewTimer;
    QStringList m_connectedEvents; // note this updated in another thread. Always guard with m_mutex
    QStringList m_requestedDBusEvents;
    int m_subscriptionId = -1;
    QMutex m_mutex;
};

#endif // KCUPSCONNECTION_H
