/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <QStandardItemModel>
#include <QTimer>
#include <qqmlregistration.h>

#include <KCupsPrinter.h>
#include <kcupslib_export.h>

class KCupsRequest;
class KCUPSLIB_EXPORT PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool serverUnavailable READ serverUnavailable NOTIFY serverUnavailableChanged)
    /**
     * Whether or not to actually display the location of the printer
     *
     * Only show the location if there is more than one printer
     * and at least two distinct locations exist.  If there is only one
     * printer or 2 or more printers have the same location, this will be false
     */
    Q_PROPERTY(bool displayLocationHint READ displayLocationHint NOTIFY displayLocationHintChanged)
    /**
     * true if model only contains printers (not classes)
     */
    Q_PROPERTY(bool printersOnly READ printersOnly NOTIFY countChanged)

public:
    enum Role {
        DestStatus = Qt::UserRole,
        DestState,
        DestName,
        DestIsDefault,
        DestIsShared,
        DestIsAcceptingJobs,
        DestIsPaused,
        DestIsClass,
        DestLocation,
        DestDescription,
        DestKind,
        DestType,
        DestCommands,
        DestMarkerChangeTime,
        DestMarkers,
        DestIconName,
        DestRemote,
        DestUri,
        DestUriSupported,
        DestMemberNames
    };
    Q_ENUM(Role)

    enum JobAction { Cancel, Hold, Release, Move };
    Q_ENUM(JobAction)

    explicit PrinterModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int count() const;
    bool serverUnavailable() const;

    virtual QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void rejectJobs(const QString &printerName);
    Q_INVOKABLE void acceptJobs(const QString &printerName);

    bool displayLocationHint() const;

public slots:
    void update();
    void getDestsFinished(KCupsRequest *request);
    void slotCountChanged();

signals:
    void countChanged(int count);
    void serverUnavailableChanged(bool unavailable);
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void displayLocationHintChanged();

private slots:
    void insertUpdatePrinterName(const QString &printerName);
    void insertUpdatePrinter(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void insertUpdatePrinterFinished(KCupsRequest *request);
    void printerRemovedName(const QString &printerName);
    void printerRemoved(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
    void printerStateChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void printerStopped(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
    void printerRestarted(const QString &text,
                          const QString &printerUri,
                          const QString &printerName,
                          uint printerState,
                          const QString &printerStateReasons,
                          bool printerIsAcceptingJobs);
    void printerShutdown(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);
    void printerModified(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);
    void serverChanged(const QString &text);

private:
    WId m_parentId;
    QHash<int, QByteArray> m_roles;
    bool m_unavailable = true;
    bool m_displayLocationHint = true;

    void setDisplayLocationHint();
    int destRow(const QString &destName);
    void insertDest(int pos, const KCupsPrinter &printer);
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;
    bool printersOnly() const;
    QStringList m_attrs;
};

#endif // PRINTER_MODEL_H
