/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPS_REQUEST_H
#define KCUPS_REQUEST_H

#include <QEventLoop>
#include <QObject>

#include <kcupslib_export.h>

#include "KCupsConnection.h"
#include "KCupsJob.h"
#include "KCupsPrinter.h"
#include "KCupsServer.h"
#include "KIppRequest.h"

class KCUPSLIB_EXPORT KCupsRequest : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor, it takes no parent
     * because it will move to KCupsConnection thread
     *
     * Before calling any method connect to finished() signal or
     * use waitTillFinished().
     * You must delete the object manually after finished
     * using deleteLater().
     */
    explicit KCupsRequest(KCupsConnection *connection = nullptr);

    /**
     * This method creates an event loop
     * and quits after the request is finished
     */
    void waitTillFinished();

    /**
     * This method returns true if there was an error with the request
     */
    bool hasError() const;
    ipp_status_t error() const;
    http_status_t httpStatus() const;
    QString serverError() const;
    QString errorMsg() const;

    KCupsConnection *connection() const;

    /**
     * Non empty when getPrinters is called and finish is emitted
     */
    KCupsPrinters printers() const;

    /**
     * Non empty when getPPDs is called and finish is emitted
     */
    ReturnArguments ppds() const;

    /**
     * Non empty when getServerSettings() is called and finish is emitted
     */
    KCupsServer serverSettings() const;

    /**
     * Non empty when \sa getPrinterPPD() is called and finish is emitted
     * \warning You must unlik the given file name
     */
    QString printerPPD() const;

    /**
     * Non empty when getJobs is called and finish is emitted
     */
    KCupsJobs jobs() const;

    /**
     * Get all available PPDs from the givem make
     * @param make the maker of the printer
     */
    Q_INVOKABLE void getPPDS(const QString &make = QString());

    /**
     * Get all devices that could be added as a printer
     * This method emits device()
     */
    Q_INVOKABLE void getDevices(int timeout = CUPS_TIMEOUT_DEFAULT);

    /**
     * Get all devices that could be added as a printer
     * This method emits device()
     */
    Q_INVOKABLE void getDevices(int timeout, QStringList includeSchemes, QStringList excludeSchemes);

    /**
     * Get all available printers
     * @param mask filter the kind of printer that will be emitted (-1 to no filter)
     * @param requestedAttr the attributes to retrieve from cups
     * This method emits printer()
     *
     * THIS function can get the default server dest through the
     * "printer-is-default" attribute BUT it does not get user
     * defined default printer, see cupsGetDefault() on www.cups.org for details
     */
    Q_INVOKABLE void getPrinters(QStringList attributes, int mask = -1);

    /**
     * Get attributes from a given printer
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The attributes you are requesting
     *
     * @return The return will be stored in \sa printers()
     */
    Q_INVOKABLE void getPrinterAttributes(const QString &printerName, bool isClass, QStringList attributes);

    /**
     * Get all jobs
     * This method emits job()
     * TODO we need to see if we authenticate as root to do some taks
     *      the myJobs will return the user's jobs or the root's jobs
     * @param printer which printer you are requiring jobs for (empty = all printers)
     * @param myJobs true if you only want your jobs
     * @param whichJobs which kind jobs should be sent
     */
    Q_INVOKABLE void getJobs(const QString &printerName, bool myJobs, int whichJobs, QStringList attributes);

    /**
     * Get attributes from a given printer
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The attributes you are requesting
     *
     * @return The return will be stored in \sa printers()
     */
    Q_INVOKABLE void getJobAttributes(int jobId, const QString &printerUri, QStringList attributes);

    /**
     * Get the CUPS server settings
     * This method emits server()
     */
    Q_INVOKABLE void getServerSettings();

    /**
     * Get the PPD associated with @arg printerName
     * the result is stored at \sa printerPPD()
     */
    Q_INVOKABLE void getPrinterPPD(const QString &printerName);

    /**
     * Get the CUPS server settings
     * @param userValues the new server settings
     */
    Q_INVOKABLE void setServerSettings(const KCupsServer &server);

    // ---- Printer Methods
    /**
     * Add or Modify a Printer
     * @param printerName The printer to apply the change
     * @param attributes The new attributes of the printer
     * @param filename The file name in case of changing the PPD
     */
    void addOrModifyPrinter(const QString &printerName, const QVariantMap &attributes, const QString &filename = QString());

    /**
     * Add or Modify a Class
     * @param className The class to apply the change
     * @param attributes The new attributes of the printer
     */
    void addOrModifyClass(const QString &className, const QVariantMap &attributes);

    /**
     * Set if a given printer should be shared among other cups
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param shared True if it should be shared
     */
    void setShared(const QString &printerName, bool isClass, bool shared);

    /**
     * Set if a given printer should be the default one among others
     * @param printer The printer to apply the change
     */
    void setDefaultPrinter(const QString &printerName);

    /**
     * Pause the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void pausePrinter(const QString &printerName);

    /**
     * Resume the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void resumePrinter(const QString &printerName);

    /**
     * Allows the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void acceptJobs(const QString &printerName);

    /**
     * Prevents the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void rejectJobs(const QString &printerName);

    /**
     * Delete the given printer, if it's not local it's not
     * possible to delete it
     * @param printer The printer to apply the change
     */
    void deletePrinter(const QString &printerName);

    /**
     * Print a test page
     * @param printerName The printer where the test should be done
     * @param isClass True it is a printer class
     */
    void printTestPage(const QString &printerName, bool isClass);

    /**
     * Print a command test
     * @param printerName The printer where the test should be done
     * @param command The command to print
     * @param title The title of the command
     */
    Q_INVOKABLE void printCommand(const QString &printerName, const QString &command, const QString &title);

    // Jobs methods
    /**
     * Cancels tries to cancel a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void cancelJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void holdJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void releaseJob(const QString &printerName, int jobId);

    /**
     * Restart the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void restartJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param fromDestName the destination name which holds the job
     * @param jobId the job identification
     * @param toDestName the destination to hold the job
     */
    void moveJob(const QString &fromPrinterName, int jobId, const QString &toPrinterName);

    void authenticateJob(const QString &printerName, const QStringList authInfo, int jobId);

signals:
    void device(const QString &device_class,
                const QString &device_id,
                const QString &device_info,
                const QString &device_make_and_model,
                const QString &device_uri,
                const QString &device_location);

    void finished(KCupsRequest *);

private:
    void invokeMethod(const char *method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());
    Q_INVOKABLE void process(const KIppRequest &request);
    void setError(http_status_t httpStatus, ipp_status_t error, const QString &errorMsg);
    void setFinished(bool delayed = false);

    KCupsConnection *m_connection = nullptr;
    QEventLoop m_loop;
    bool m_finished = true;
    ipp_status_t m_error = IPP_OK;
    http_status_t m_httpStatus;
    QString m_errorMsg;
    ReturnArguments m_ppds;
    KCupsServer m_server;
    QString m_ppdFile;
    KCupsPrinters m_printers;
    KCupsJobs m_jobs;
};

#endif // KCUPS_REQUEST_H
