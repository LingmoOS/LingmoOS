// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "disomaster.h"
#include "xorriso.h"
#include <QRegularExpression>

#define PCHAR(s) (char *)(s)

#define XORRISO_OPT(opt, x, ...)                 \
    Xorriso_set_problem_status(x, PCHAR(""), 0); \
    r = Xorriso_option_##opt(x, __VA_ARGS__);    \
    r = Xorriso_eval_problem_status(x, r, 0);

#define JOBFAILED_IF(r, x)                              \
    if (r <= 0) {                                       \
        Xorriso_option_end(x, 1);                       \
        Q_EMIT jobStatusChanged(JobStatus::Failed, -1); \
        return false;                                   \
    }

int XorrisoResultHandler(void *handle, char *text);
int XorrisoInfoHandler(void *handle, char *text);

namespace DISOMasterNS {

class DISOMasterPrivate
{
private:
    DISOMasterPrivate(DISOMaster *q)
        : q_ptr(q) {}
    XorrisO *xorriso;
    QHash<QUrl, QUrl> files;
    QHash<QString, DeviceProperty> dev;
    QStringList xorrisomsg;
    QString curdev;
    QString curspeed;
    DISOMaster *q_ptr;
    Q_DECLARE_PUBLIC(DISOMaster)

    void getCurrentDeviceProperty();

public:
    void messageReceived(int type, char *text);
};

/*!
 * \class DISOMaster
 * \brief The DISOMaster class is a simple wrapper around libisoburn.
 *
 * DISOMaster provides basic optical drive operation and on-disc
 * filesystem manipulation.
 *
 * All method calls in this class are synchronous: they do not
 * return until the operation completes. Note the signal is emitted
 * from a separate thread (while the job is actually running).
 */
DISOMaster::DISOMaster(QObject *parent)
    : QObject(parent),
      d_ptr(new DISOMasterPrivate(this))
{
    Q_D(DISOMaster);
    int r = Xorriso_new(&d->xorriso, PCHAR("xorriso"), 0);
    if (r <= 0) {
        d->xorriso = nullptr;
        return;
    }

    r = Xorriso_startup_libraries(d->xorriso, 0);
    if (r <= 0) {
        Xorriso_destroy(&d->xorriso, 0);
        return;
    }

    Xorriso_sieve_big(d->xorriso, 0);
    Xorriso_start_msg_watcher(d->xorriso, XorrisoResultHandler, d, XorrisoInfoHandler, d, 0);
}

DISOMaster::~DISOMaster()
{
    Q_D(DISOMaster);

    if (d->xorriso) {
        Xorriso_stop_msg_watcher(d->xorriso, 0);
        Xorriso_destroy(&d->xorriso, 0);
    }
}

/*!
 * \brief Acquire an optical drive.
 * \param dev The device identifier of the drive to be
 * acquired (e.g. "/dev/sr0")
 *
 * Unless otherwise stated, all methods below requires
 * a drive acquired.
 * DISOMaster will take exclusive control of the device
 * until it is released by calling releaseDevice().
 *
 * \return true on success. If this function fails, it
 * is usually because the device is currently in use.
 */
bool DISOMaster::acquireDevice(QString dev)
{
    Q_D(DISOMaster);

    if (dev.length()) {
        d->files.clear();
        d->curdev = dev;

        int r;
        XORRISO_OPT(dev, d->xorriso, dev.toUtf8().data(), 3);
        if (r <= 0) {
            d->curdev = "";
            return false;
        }
        return true;
    }

    return false;
}

/*!
 * \brief Release the drive currently held.
 */
void DISOMaster::releaseDevice()
{
    Q_D(DISOMaster);
    d->curdev = "";
    d->files.clear();
    Xorriso_option_end(d->xorriso, 0);
}

/*!
 * \brief Get the drive currently held (if any).
 * \return the device identifier of the drive currently held.
 * If no drive is currently held, returns an empty string.
 */
QString DISOMaster::currentDevice() const
{
    Q_D(const DISOMaster);
    return d->curdev;
}

/*!
 * \brief Get the property of the currently acquired device.
 *
 * This function requires a disc spin-up and may take a while.
 * The result will be automatically cached.
 *
 * \return the property of the acquired device.
 * \sa getDevicePropertyCached(), nullifyDevicePropertyCache(QString dev)
 */
DeviceProperty DISOMaster::getDeviceProperty()
{
    Q_D(DISOMaster);
    d->getCurrentDeviceProperty();
    return d->dev[d->curdev];
}

/*!
 * \brief Get cached property of a device.
 *
 * Returns the property of a device when it was acquired
 * last time. Does not require a device acquired.
 * If the device has no property cached, this function will return an
 * invalid device property, which you can tell by testing whether the
 * devid field is empty.
 *
 * \return the cached property.
 * \sa getDeviceProperty(), nullifyDevicePropertyCache(QString dev)
 */
DeviceProperty DISOMaster::getDevicePropertyCached(QString dev) const
{
    Q_D(const DISOMaster);
    if (d->dev.find(dev) != d->dev.end()) {
        return d->dev[dev];
    }
    return DeviceProperty();
}

/*!
 * \brief Nullify cached device property for a drive.
 *
 * Call this after a disc swap (or whenever you are sure
 * the previously cached device property is no longer
 * up to date).
 */
void DISOMaster::nullifyDevicePropertyCache(QString dev)
{
    Q_D(DISOMaster);
    if (d->dev.find(dev) != d->dev.end()) {
        d->dev.erase(d->dev.find(dev));
    }
}

/*!
 * \brief Get a list of messages from xorriso.
 *
 * This will clear the internal message buffer.
 *
 * \return a list of messages from xorriso since the last command.
 */
QStringList DISOMaster::getInfoMessages()
{
    Q_D(DISOMaster);
    QStringList ret = d->xorrisomsg;
    d->xorrisomsg.clear();
    return ret;
}

/*!
 * \brief Get the current data transfer rate.
 *
 * The value is a multiplier of base speed of the current media.
 *
 * \return a string containing a multiplier (e.g. "10.0x").
 */
QString DISOMaster::getCurrentSpeed() const
{
    Q_D(const DISOMaster);
    return d->curspeed;
}

/*!
 * \brief Stage files for burning.
 * \param filelist A map from local files to on-disc files.
 */
void DISOMaster::stageFiles(const QHash<QUrl, QUrl> filelist)
{
    Q_D(DISOMaster);
    d->files.unite(filelist);
}

/*!
 * \brief Get all files currently staged for burning.
 * \return a map from local files to on-disc files.
 */
const QHash<QUrl, QUrl> &DISOMaster::stagingFiles() const
{
    Q_D(const DISOMaster);
    return d->files;
}

/*!
 * \brief Unstage files for burning.
 * \param filelist the local files to unstage.
 */
void DISOMaster::removeStagingFiles(const QList<QUrl> filelist)
{
    Q_D(DISOMaster);
    for (auto &i : filelist) {
        auto it = d->files.find(i);
        if (it != d->files.end()) {
            d->files.erase(it);
        }
    }
}

/*!
 * \brief DISOMaster::commit  Burn all staged files to the disc.
 * \param opts   burning options
 * \param speed  desired writing speed in kilobytes per second
 * \param volId  volume name of the disc
 * \return       true on success, false on failure
 */
bool DISOMaster::commit(const BurnOptions &opts, int speed /* = 0*/, QString volId /* = "ISOIMAGE"*/)
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0);
    d->xorrisomsg.clear();

    QString spd = QString::number(speed) + "k";
    if (speed == 0) {
        spd = "0";
    }

    int r;

    XORRISO_OPT(speed, d->xorriso, spd.toUtf8().data(), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(volid, d->xorriso, volId.toUtf8().data(), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(overwrite, d->xorriso, PCHAR("off"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(joliet, d->xorriso, PCHAR(opts.testFlag(JolietSupport) ? "on" : "off"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(rockridge, d->xorriso, PCHAR(opts.testFlag(RockRidgeSupport) ? "on" : "off"), 0);
    JOBFAILED_IF(r, d->xorriso);

    for (auto it = d->files.begin(); it != d->files.end(); ++it) {
        XORRISO_OPT(
                map, d->xorriso,
                it.key().toString().toUtf8().data(),
                it.value().toString().toUtf8().data(),
                0);
        JOBFAILED_IF(r, d->xorriso);
    }

    XORRISO_OPT(close, d->xorriso, PCHAR(opts.testFlag(KeepAppendable) ? "off" : "on"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(commit, d->xorriso, 0);
    JOBFAILED_IF(r, d->xorriso);

    return true;
}

/*!
 * \brief Burn all staged files to the disc.
 * \param speed desired writing speed in kilobytes per second
 * \param closeSession if true, closes the session after files are burned
 * \param volId volume name of the disc
 * \return true on success, false on failure
 *
 * closeSession will be ignored if the disc is reusable.
 * The staging file list will be cleared afterwards.
 */
bool DISOMaster::commit(int speed, bool closeSession, QString volId)
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0);
    d->xorrisomsg.clear();

    QString spd = QString::number(speed) + "k";
    if (speed == 0) {
        spd = "0";
    }

    int r;

    XORRISO_OPT(speed, d->xorriso, spd.toUtf8().data(), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(volid, d->xorriso, volId.toUtf8().data(), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(overwrite, d->xorriso, PCHAR("off"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(joliet, d->xorriso, PCHAR("on"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(rockridge, d->xorriso, PCHAR("on"), 0);
    JOBFAILED_IF(r, d->xorriso);

    for (auto it = d->files.begin(); it != d->files.end(); ++it) {
        XORRISO_OPT(
                map, d->xorriso,
                it.key().toString().toUtf8().data(),
                it.value().toString().toUtf8().data(),
                0);
        JOBFAILED_IF(r, d->xorriso);
    }

    XORRISO_OPT(close, d->xorriso, PCHAR(closeSession ? "on" : "off"), 0);
    JOBFAILED_IF(r, d->xorriso);

    XORRISO_OPT(commit, d->xorriso, 0);
    JOBFAILED_IF(r, d->xorriso);

    return true;
}

/*!
 * \brief Erase the disc (if it's not already blank).
 * \return true on success, false on failure
 */
bool DISOMaster::erase()
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Running, 0);
    d->xorrisomsg.clear();

    int r;
    XORRISO_OPT(abort_on, d->xorriso, PCHAR("ABORT"), 0);
    XORRISO_OPT(blank, d->xorriso, PCHAR("as_needed"), 0);
    JOBFAILED_IF(r, d->xorriso);

    return true;
}

/*!
 * \brief Perform a data integration check for the disc.
 * \param qgood if not null, will be set to the portion of sectors that can be read fast.
 * \param qslow if not null, will be set to the portion of sectors that can still be read, but slowly.
 * \param qbad if not null, will be set to the portion of sectors that are corrupt.
 * \return true on success, false on failure (if for some reason the disc could not be checked)
 *
 * The values returned should add up to 1 (or very close to 1).
 */
bool DISOMaster::checkmedia(double *qgood, double *qslow, double *qbad)
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Running, 0);
    d->xorrisomsg.clear();

    int r, ac, avail;
    int dummy = 0;
    char **av;

    getDeviceProperty();
    XORRISO_OPT(check_media, d->xorriso, 0, nullptr, &dummy, 0);
    JOBFAILED_IF(r, d->xorriso);

    quint64 ngood = 0;
    quint64 nslow = 0;
    quint64 nbad = 0;

    do {
        Xorriso_sieve_get_result(d->xorriso, PCHAR("Media region :"), &ac, &av, &avail, 0);
        if (ac == 3) {
            quint64 szblk = QString(av[1]).toLongLong();
            if (av[2][0] == '-') {
                nbad += szblk;
            } else if (av[2][0] == '0') {
                ngood += szblk;
            } else {
                if (QString(av[2]).contains("slow")) {
                    nslow += szblk;
                } else {
                    ngood += szblk;
                }
            }
        }
        Xorriso__dispose_words(&ac, &av);
    } while (avail > 0);

    if (qgood) {
        *qgood = 1. * ngood / d->dev[d->curdev].datablocks;
    }
    if (qslow) {
        *qslow = 1. * nslow / d->dev[d->curdev].datablocks;
    }
    if (qbad) {
        *qbad = 1. * nbad / d->dev[d->curdev].datablocks;
    }

    Xorriso_sieve_clear_results(d->xorriso, 0);

    Q_EMIT jobStatusChanged(DISOMaster::JobStatus::Finished, 0);

    return true;
}

/*!
 * \brief Dump the content of the disc to a file
 *  xorriso -outdev /dev/sr[?] -check_media use=outdev data_to=[path]
 * \param isopath the image file to be dumped
 * \return true on success, false on failure
 */
bool DISOMaster::dumpISO(const QUrl isopath)
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Running, 0);
    d->xorrisomsg.clear();

    Q_ASSERT(!isopath.isEmpty());
    Q_ASSERT(isopath.isValid());

    int r;
    char **av = new char *[2];
    int dummy = 0;
    av[0] = strdup((QString("use=outdev")).toUtf8().data());
    av[1] = strdup((QString("data_to=") + isopath.path()).toUtf8().data());
    XORRISO_OPT(check_media, d->xorriso, 2, av, &dummy, 0);

    for (int i = 0; i < 2; ++i)
        free(av[i]);
    delete[] av;

    JOBFAILED_IF(r, d->xorriso);

    return true;
}

/*!
 * \brief Burn an image to the disc.
 * \param isopath the image file to be burnt.
 * \param speed the desired write speed in kilobytes per second.
 * \return true on success, false on failure
 */
bool DISOMaster::writeISO(const QUrl isopath, int speed)
{
    Q_D(DISOMaster);
    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0);
    d->xorrisomsg.clear();
    QString spd = QString::number(speed) + "k";
    if (speed == 0) {
        spd = "0";
    }

    int r;

    char **av = new char *[6];
    int dummy = 0;
    av[0] = strdup("cdrecord");
    av[1] = strdup("-v");
    av[2] = strdup((QString("dev=") + d->curdev).toUtf8().data());
    av[3] = strdup("blank=as_needed");
    av[4] = strdup((QString("speed=") + spd).toUtf8().data());
    av[5] = strdup(isopath.path().toUtf8().data());
    XORRISO_OPT(as, d->xorriso, 6, av, &dummy, 1);
    JOBFAILED_IF(r, d->xorriso);

    //-as cdrecord releases the device automatically.
    //we don't want that.
    acquireDevice(d->curdev);

    for (int i = 0; i < 6; ++i) {
        free(av[i]);
    }
    delete[] av;

    return true;
}

void DISOMasterPrivate::getCurrentDeviceProperty()
{
    if (!curdev.length()) {
        return;
    }

    dev[curdev].devid = curdev;

    int r, ac, avail;
    char **av;
    Xorriso_sieve_get_result(xorriso, PCHAR("Media current:"), &ac, &av, &avail, 0);
    if (ac < 1) {
        Xorriso__dispose_words(&ac, &av);
        return;
    }
    QString mt = av[0];
    const static QHash<QString, MediaType> typemap = {
        { "CD-ROM", MediaType::CD_ROM },
        { "CD-R", MediaType::CD_R },
        { "CD-RW", MediaType::CD_RW },
        { "DVD-ROM", MediaType::DVD_ROM },
        { "DVD-R", MediaType::DVD_R },
        { "DVD-RW", MediaType::DVD_RW },
        { "DVD+R", MediaType::DVD_PLUS_R },
        { "DVD+R/DL", MediaType::DVD_PLUS_R_DL },
        { "DVD-RAM", MediaType::DVD_RAM },
        { "DVD+RW", MediaType::DVD_PLUS_RW },
        { "BD-ROM", MediaType::BD_ROM },
        { "BD-R", MediaType::BD_R },
        { "BD-RE", MediaType::BD_RE }
    };
    mt = mt.left(mt.indexOf(' '));
    if (typemap.find(mt) != typemap.end()) {
        dev[curdev].media = typemap[mt];
    } else {
        dev[curdev].media = MediaType::NoMedia;
    }
    Xorriso__dispose_words(&ac, &av);

    Xorriso_sieve_get_result(xorriso, PCHAR("Media summary:"), &ac, &av, &avail, 0);
    if (ac == 4) {
        const QString units = "kmg";
        dev[curdev].datablocks = atoll(av[1]);
        dev[curdev].data = atof(av[2]) * (1 << ((units.indexOf(*(QString(av[2]).rbegin())) + 1) * 10));
        dev[curdev].avail = atof(av[3]) * (1 << ((units.indexOf(*(QString(av[3]).rbegin())) + 1) * 10));
    }
    Xorriso__dispose_words(&ac, &av);

    Xorriso_sieve_get_result(xorriso, PCHAR("Media status :"), &ac, &av, &avail, 0);
    if (ac == 1) {
        dev[curdev].formatted = QString(av[0]).contains("is blank");
    }
    Xorriso__dispose_words(&ac, &av);

    Xorriso_sieve_get_result(xorriso, PCHAR("Volume id    :"), &ac, &av, &avail, 0);
    if (ac == 1) {
        dev[curdev].volid = QString(av[0]);
    }
    Xorriso__dispose_words(&ac, &av);

    XORRISO_OPT(list_speeds, xorriso, 0);
    if (r <= 0) {
        return;
    }

    dev[curdev].writespeed.clear();
    do {
        Xorriso_sieve_get_result(xorriso, PCHAR("Write speed  :"), &ac, &av, &avail, 0);
        if (ac == 2) {
            dev[curdev].writespeed.push_back(QString(av[0]) + '\t' + QString(av[1]));
        }
        Xorriso__dispose_words(&ac, &av);
    } while (avail > 0);

    Xorriso_sieve_clear_results(xorriso, 0);
}

void DISOMasterPrivate::messageReceived(int type, char *text)
{
    Q_Q(DISOMaster);
    Q_UNUSED(type);
    QString msg(text);
    msg = msg.trimmed();

    fprintf(stderr, "msg from xorriso (%s) : %s\n", type ? " info " : "result", msg.toStdString().c_str());
    xorrisomsg.push_back(msg);

    //closing session
    if (msg.contains("UPDATE : Closing track/session.")) {
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Stalled, 1);
        return;
    }

    //stalled
    if (msg.contains("UPDATE : Thank you for being patient.")) {
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Stalled, 0);
        return;
    }

    //cdrecord / blanking
    QRegularExpression r("([0-9.]*)%\\s*(fifo|done)");
    QRegularExpressionMatch m = r.match(msg);
    if (m.hasMatch()) {
        double percentage = m.captured(1).toDouble();
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Running, percentage);
    }

    //commit
    r = QRegularExpression("([0-9]*)\\s*of\\s*([0-9]*) MB written");
    m = r.match(msg);
    if (m.hasMatch()) {
        double percentage = 100. * m.captured(1).toDouble() / m.captured(2).toDouble();
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Running, percentage);
    }

    //check media
    r = QRegularExpression("([0-9]*) blocks read in ([0-9]*) seconds , ([0-9.]*)x");
    m = r.match(msg);
    if (m.hasMatch() && dev[curdev].datablocks != 0) {
        double percentage = 100. * m.captured(1).toDouble() / dev[curdev].datablocks;
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Running, percentage);
    }

    //current speed
    r = QRegularExpression("([0-9]*\\.[0-9]x)[bBcCdD.]");
    m = r.match(msg);
    if (m.hasMatch()) {
        curspeed = m.captured(1);
    } else {
        curspeed.clear();
    }

    //operation complete
    if (msg.contains("Blanking done") || msg.contains(QRegularExpression("Writing to .* completed successfully."))) {
        Q_EMIT q->jobStatusChanged(DISOMaster::JobStatus::Finished, 0);
    }
}

}

int XorrisoResultHandler(void *handle, char *text)
{
    ((DISOMasterNS::DISOMasterPrivate *)handle)->messageReceived(0, text);
    return 1;
}
int XorrisoInfoHandler(void *handle, char *text)
{
    //working around xorriso passing wrong handle to the callback
    if (strstr(text, "DEBUG : Concurrent message watcher")) {
        return 1;
    }
    ((DISOMasterNS::DISOMasterPrivate *)handle)->messageReceived(1, text);
    return 1;
}
/*
 * xorriso -devices
 * xorriso -dev * -toc
 * xorriso -dev * -list_speeds
 * xorriso -dev * -check_media
 * xorriso -dev * -blank as_needed
 * xorriso -dev * -speed x -map /home/tfosirhc/Pictures/ /Pictures [-close on|off] -commit
 * other todos:
 * -volid
 */
