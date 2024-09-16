// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISOMASTER_H
#define DISOMASTER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QUrl>

namespace DISOMasterNS {

enum MediaType
{
    NoMedia = 0,
    CD_ROM,
    CD_R,
    CD_RW,
    DVD_ROM,
    DVD_R,
    DVD_RW,
    DVD_PLUS_R,
    DVD_PLUS_R_DL,
    DVD_RAM,
    DVD_PLUS_RW,
    BD_ROM,
    BD_R,
    BD_RE
};

enum BurnOption
{
    KeepAppendable = 1,
    VerifyDatas = 1 << 1,
    EjectDisc = 1 << 2,             // not used yet.
    ISO9660Only = 1 << 3,           // default
    JolietSupport = 1 << 4,         // add joliet extension
    RockRidgeSupport = 1 << 5,      // add rockridge extension
    JolietAndRockRidge = 1 << 6,    // add both of them, not used yet
};
Q_DECLARE_FLAGS(BurnOptions, BurnOption)

struct DeviceProperty
{
    /** \brief True when the media in device is blank or false otherwise.*/
    bool formatted;
    /** \brief Type of media currently in the device.*/
    MediaType media;
    /** \brief Size of on-disc data in bytes.*/
    quint64 data;
    /** \brief Available size in bytes.*/
    quint64 avail;
    /** \brief Size of on-disc data in number of blocks.*/
    quint64 datablocks;
    /** \brief List of write speeds supported, e.g. "10.0x".*/
    QList<QString> writespeed;
    /** \brief Device identifier. Empty if the device property is invalid.*/
    QString devid;
    /** \brief Volume name of the disc.*/
    QString volid;
};

class DISOMasterPrivate;
class DISOMaster : public QObject
{
    Q_OBJECT
public:
    enum JobStatus
    {
        Failed = -1,
        Idle,
        Running,
        Stalled,
        Finished
    };
    Q_ENUM(JobStatus)

    explicit DISOMaster(QObject *parent = nullptr);
    ~DISOMaster();

    bool acquireDevice(QString dev);
    void releaseDevice();
    QString currentDevice() const;

    DeviceProperty getDeviceProperty();
    DeviceProperty getDevicePropertyCached(QString dev) const;
    void nullifyDevicePropertyCache(QString dev);

    QStringList getInfoMessages();
    QString getCurrentSpeed() const;

    void stageFiles(const QHash<QUrl, QUrl> filelist);
    const QHash<QUrl, QUrl> &stagingFiles() const;
    void removeStagingFiles(const QList<QUrl> filelist);
    bool commit(const BurnOptions &opts, int speed = 0, QString volId = "ISOIMAGE");
    Q_DECL_DEPRECATED_X("Suggest use commit with BurnOptions instead") bool commit(int speed = 0, bool closeSession = false, QString volId = "ISOIMAGE");
    bool erase();
    bool checkmedia(double *qgood, double *qslow, double *qbad);
    bool dumpISO(const QUrl isopath);
    bool writeISO(const QUrl isopath, int speed = 0);

Q_SIGNALS:
    /**
     * \brief Indicates a change of current job status.
     *
     * \param status Current job status.
     * \param progress Job progress in percentage if status is running.
     *        Type of stalled work if status is stalled (1 = closing session, 0 = others).
     */
    void jobStatusChanged(DISOMasterNS::DISOMaster::JobStatus status, int progress);

private:
    QScopedPointer<DISOMasterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DISOMaster)
};

}

#endif
// vim: set tabstop=4 shiftwidth=4 softtabstop expandtab
