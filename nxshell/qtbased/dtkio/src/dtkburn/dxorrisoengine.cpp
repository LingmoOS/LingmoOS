// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dxorrisoengine.h"

#include <functional>

#include <QHash>
#include <QRegularExpression>

DBURN_BEGIN_NAMESPACE

namespace ISOEnginePrivate {
static inline char *PCHAR(const char *c)
{
    return const_cast<char *>(c);
}

static inline int XORRISO_OPT(struct XorrisO *x, std::function<int()> opt)
{
    Xorriso_set_problem_status(x, PCHAR(""), 0);
    int r = opt();
    return Xorriso_eval_problem_status(x, r, 0);
}

static inline bool JOBFAILED_IF(DXorrisoEngine *engine, int r, struct XorrisO *x)
{
    if (r <= 0) {
        Xorriso_option_end(x, 1);
        Q_EMIT engine->jobStatusChanged(JobStatus::Failed, -1, "");
        return true;
    }
    return false;
}

static int xorrisoResultHandler(void *handle, char *text)
{
    (static_cast<DXorrisoEngine *>(handle))->messageReceived(0, text);
    return 1;
}
static int xorrisoInfoHandler(void *handle, char *text)
{
    // working around xorriso passing wrong handle to the callback
    if (strstr(text, "DEBUG : Concurrent message watcher")) {
        return 1;
    }
    (static_cast<DXorrisoEngine *>(handle))->messageReceived(1, text);
    return 1;
}

}   // namespace ISOEnginePrivate

DXorrisoEngine::DXorrisoEngine(QObject *parent)
    : QObject(parent)
{
    initialize();
}

DXorrisoEngine::~DXorrisoEngine()
{
    if (m_xorriso) {
        Xorriso_stop_msg_watcher(m_xorriso, 0);
        Xorriso_destroy(&m_xorriso, 0);
    }
}

/*!
 * @~english \brief Acquire an optical drive.
 * @~english \param dev The device identifier of the drive to be
 * acquired (e.g. "/dev/sr0")
 *
 * @~english Unless otherwise stated, all methods below requires
 * a drive acquired.
 * @~english XorrisoEngine will take exclusive control of the device
 * until it is released by calling releaseDevice().
 *
 * @~english \return true on success. If this function fails, it
 * is usually because the device is currently in use.
 */
bool DXorrisoEngine::acquireDevice(const QString &dev)
{
    if (!dev.isEmpty()) {
        m_curDev = dev;

        int result = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, dev]() {
            return Xorriso_option_dev(m_xorriso, dev.toUtf8().data(), 3);
        });
        if (result <= 0) {
            m_curDev = "";
            return false;
        }
        return true;
    }
    return false;
}

/*!
 * @~english \brief Release the drive currently held.
 */
void DXorrisoEngine::releaseDevice()
{
    m_curDev = "";
    Xorriso_option_end(m_xorriso, 0);
}

void DXorrisoEngine::clearResult()
{
    Xorriso_sieve_clear_results(m_xorriso, 0);
}

MediaType DXorrisoEngine::mediaTypeProperty() const
{
    MediaType devMedia { MediaType::NoMedia };
    if (m_curDev.isEmpty())
        return devMedia;

    int ac, avail;
    char **av;
    Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Media current:"), &ac, &av, &avail, 0);
    if (ac < 1) {
        Xorriso__dispose_words(&ac, &av);
        return devMedia;
    }
    QString mt = av[0];
    const static QHash<QString, MediaType> kTypemap = {
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
    if (kTypemap.find(mt) != kTypemap.end()) {
        devMedia = kTypemap[mt];
    } else {
        devMedia = MediaType::NoMedia;
    }

    Xorriso__dispose_words(&ac, &av);

    return devMedia;
}

void DXorrisoEngine::mediaStorageProperty(quint64 *usedSize, quint64 *availSize, quint64 *blocks) const
{
    if (m_curDev.isEmpty())
        return;

    int ac, avail;
    char **av;
    Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Media summary:"), &ac, &av, &avail, 0);
    if (ac == 4) {
        const QString units = "kmg";
        *blocks = static_cast<quint64>(atoll(av[1]));
        *usedSize = static_cast<quint64>(atof(av[2]) * (1 << ((units.indexOf(*(QString(av[2]).rbegin())) + 1) * 10)));
        *availSize = static_cast<quint64>(atof(av[3]) * (1 << ((units.indexOf(*(QString(av[3]).rbegin())) + 1) * 10)));
    }
    Xorriso__dispose_words(&ac, &av);
}

bool DXorrisoEngine::mediaFormattedProperty() const
{
    bool formatted { true };
    if (m_curDev.isEmpty())
        return formatted;

    int ac, avail;
    char **av;
    Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Media status :"), &ac, &av, &avail, 0);

    if (ac == 1)
        formatted = QString(av[0]).contains("is blank");

    Xorriso__dispose_words(&ac, &av);

    return formatted;
}

QString DXorrisoEngine::mediaVolIdProperty() const
{
    QString volId;
    if (m_curDev.isEmpty())
        return volId;

    int ac, avail;
    char **av;
    Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Volume id    :"), &ac, &av, &avail, 0);
    if (ac == 1)
        volId = QString(av[0]);
    Xorriso__dispose_words(&ac, &av);
    return volId;
}

QStringList DXorrisoEngine::mediaSpeedProperty() const
{
    QStringList writeSpeed;
    if (m_curDev.isEmpty())
        return writeSpeed;

    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        return Xorriso_option_list_speeds(m_xorriso, 0);
    });
    if (r < 0)
        return writeSpeed;

    int ac, avail;
    char **av;
    do {
        Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Write speed  :"), &ac, &av, &avail, 0);
        if (ac == 2) {
            writeSpeed.push_back(QString(av[0]) + '\t' + QString(av[1]));
        }
        Xorriso__dispose_words(&ac, &av);
    } while (avail > 0);

    return writeSpeed;
}

/*!
 * @~english \brief Get a list of messages from xorriso.
 *
 * @~english This will clear the internal message buffer.
 *
 * @~english \return a list of messages from xorriso since the last command.
 */
QStringList DXorrisoEngine::takeInfoMessages()
{
    QStringList ret = m_xorrisomsg;
    m_xorrisomsg.clear();
    return ret;
}

bool DXorrisoEngine::doErase()
{
    Q_EMIT jobStatusChanged(JobStatus::Running, 0, m_curspeed);
    m_xorrisomsg.clear();

    ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        return Xorriso_option_abort_on(m_xorriso, ISOEnginePrivate::PCHAR("ABORT"), 0);
    });
    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        return Xorriso_option_blank(m_xorriso, ISOEnginePrivate::PCHAR("as_needed"), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    return true;
}

bool DXorrisoEngine::doWriteISO(const QString &isoPath, int speed)
{
    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0, m_curspeed);
    m_xorrisomsg.clear();

    QString spd = QString::number(speed) + "k";
    if (speed == 0)
        spd = "0";

    char **av = new char *[6];
    av[0] = strdup("cdrecord");
    av[1] = strdup("-v");
    av[2] = strdup((QString("dev=") + m_curDev).toUtf8().data());
    av[3] = strdup("blank=as_needed");
    av[4] = strdup((QString("speed=") + spd).toUtf8().data());
    av[5] = strdup(isoPath.toUtf8().data());

    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, av]() {
        int dummy { 0 };
        return Xorriso_option_as(m_xorriso, 6, av, &dummy, 1);
    });

    for (int i = 0; i < 6; ++i)
        free(av[i]);
    delete[] av;

    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    return true;
}

bool DXorrisoEngine::doDumpISO(quint64 dataBlocks, const QString &isoPath)
{
    m_curDatablocks = dataBlocks;
    if (dataBlocks == 0)
        return false;

    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0, m_curspeed);
    m_xorrisomsg.clear();

    char **av = new char *[2];
    av[0] = strdup((QString("use=outdev")).toUtf8().data());
    av[1] = strdup((QString("data_to=") + isoPath).toUtf8().data());

    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, av]() {
        int dummy { 0 };
        return Xorriso_option_check_media(m_xorriso, 2, av, &dummy, 0);
    });

    for (int i = 0; i < 2; ++i)
        free(av[i]);
    delete[] av;

    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    return true;
}

/*!
 * @~english \brief Perform a data integration check for the disc.
 * @~english \param dataBlocks is current device's blocks
 * @~english \param qgood if not null, will be set to the portion of sectors that can be read fast.
 * @~english \param qslow if not null, will be set to the portion of sectors that can still be read, but slowly.
 * @~english \param qbad if not null, will be set to the portion of sectors that are corrupt.
 * @~english \return true on success, false on failure (if for some reason the disc could not be checked)
 *
 * @~english The values returned should add up to 1 (or very close to 1).
 */
bool DXorrisoEngine::doCheckmedia(quint64 dataBlocks, double *qgood, double *qslow, double *qbad)
{
    m_curDatablocks = dataBlocks;
    if (dataBlocks == 0)
        return false;

    Q_EMIT jobStatusChanged(JobStatus::Running, 0, m_curspeed);

    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        int dummy { 0 };
        return Xorriso_option_check_media(m_xorriso, 0, nullptr, &dummy, 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    quint64 ngood = 0;
    quint64 nslow = 0;
    quint64 nbad = 0;

    int ac, avail;
    char **av;

    do {
        Xorriso_sieve_get_result(m_xorriso, ISOEnginePrivate::PCHAR("Media region :"), &ac, &av, &avail, 0);
        if (ac == 3) {
            quint64 szblk = static_cast<quint64>(QString(av[1]).toLongLong());
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

    if (qgood)
        *qgood = 1. * ngood / dataBlocks;

    if (qslow)
        *qslow = 1. * nslow / dataBlocks;

    if (qbad)
        *qbad = 1. * nbad / dataBlocks;

    Xorriso_sieve_clear_results(m_xorriso, 0);
    Q_EMIT jobStatusChanged(JobStatus::Finished, 0, m_curspeed);

    return true;
}

bool DXorrisoEngine::doBurn(const QPair<QString, QString> files, int speed, QString volId, DXorrisoEngine::JolietSupported joliet, DXorrisoEngine::RockRageSupported rockRage, DXorrisoEngine::KeepAppendable appendable)
{
    if (files.first.isEmpty())
        return false;

    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0, m_curspeed);
    m_xorrisomsg.clear();

    QString spd = QString::number(speed) + "k";
    if (speed == 0)
        spd = "0";

    // speed
    int r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, spd]() {
        return Xorriso_option_speed(m_xorriso, spd.toUtf8().data(), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // volid
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, volId]() {
        return Xorriso_option_volid(m_xorriso, volId.toUtf8().data(), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // overwrite
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        return Xorriso_option_overwrite(m_xorriso, ISOEnginePrivate::PCHAR("off"), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // joliet
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, joliet]() {
        return Xorriso_option_joliet(m_xorriso, ISOEnginePrivate::PCHAR(joliet == JolietSupported::True ? "on" : "off"), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // rockridge
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, rockRage]() {
        return Xorriso_option_rockridge(m_xorriso, ISOEnginePrivate::PCHAR(rockRage == RockRageSupported::True ? "on" : "off"), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // add files
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, files]() {
        return Xorriso_option_map(m_xorriso, files.first.toUtf8().data(),
                                  files.second.toUtf8().data(), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // close
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this, appendable]() {
        return Xorriso_option_close(m_xorriso, ISOEnginePrivate::PCHAR(appendable == KeepAppendable::True ? "off" : "on"), 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    // commit
    r = ISOEnginePrivate::XORRISO_OPT(m_xorriso, [this]() {
        return Xorriso_option_commit(m_xorriso, 0);
    });
    if (ISOEnginePrivate::JOBFAILED_IF(this, r, m_xorriso))
        return false;

    return true;
}

void DXorrisoEngine::messageReceived(int type, char *text)
{
    Q_UNUSED(type);

    QString msg(text);
    msg = msg.trimmed();
    fprintf(stderr, "msg from xorriso (%s) : %s\n", type ? " info " : "result", msg.toStdString().c_str());
    m_xorrisomsg.push_back(msg);

    // closing session
    if (msg.contains("UPDATE : Closing track/session.")) {
        Q_EMIT jobStatusChanged(JobStatus::Stalled, 1, m_curspeed);
        return;
    }

    // stalled
    if (msg.contains("UPDATE : Thank you for being patient.")) {
        Q_EMIT jobStatusChanged(JobStatus::Stalled, 0, m_curspeed);
        return;
    }

    // cdrecord / blanking
    QRegularExpression r("([0-9.]*)%\\s*(fifo|done)");
    QRegularExpressionMatch m = r.match(msg);
    if (m.hasMatch()) {
        double percentage = m.captured(1).toDouble();
        Q_EMIT jobStatusChanged(JobStatus::Running, static_cast<int>(percentage), m_curspeed);
    }

    // current speed
    r = QRegularExpression("([0-9]*\\.[0-9]x)[bBcCdD.]");
    m = r.match(msg);
    if (m.hasMatch()) {
        m_curspeed = m.captured(1);
    } else {
        m_curspeed.clear();
    }

    // commit
    r = QRegularExpression("([0-9]*)\\s*of\\s*([0-9]*) MB written");
    m = r.match(msg);
    if (m.hasMatch()) {
        double percentage = 100. * m.captured(1).toDouble() / m.captured(2).toDouble();
        Q_EMIT jobStatusChanged(JobStatus::Running, static_cast<int>(percentage), m_curspeed);
    }

    // check media
    r = QRegularExpression("([0-9]*) blocks read in ([0-9]*) seconds , ([0-9.]*)x");
    m = r.match(msg);
    if (m.hasMatch() && m_curDatablocks != 0) {
        double percentage = 100. * m.captured(1).toDouble() / m_curDatablocks;
        Q_EMIT jobStatusChanged(JobStatus::Running, static_cast<int>(percentage), m_curspeed);
    }

    // operation complete
    if (msg.contains("Blanking done") || msg.contains(QRegularExpression("Writing to .* completed successfully."))) {
        Q_EMIT jobStatusChanged(JobStatus::Finished, 0, m_curspeed);
    }
}

void DXorrisoEngine::initialize()
{
    int r = Xorriso_new(&m_xorriso, ISOEnginePrivate::PCHAR("xorriso"), 0);
    if (r <= 0) {
        m_xorriso = nullptr;
        return;
    }

    r = Xorriso_startup_libraries(m_xorriso, 0);
    if (r <= 0) {
        Xorriso_destroy(&m_xorriso, 0);
        return;
    }

    Xorriso_sieve_big(m_xorriso, 0);
    Xorriso_start_msg_watcher(m_xorriso, ISOEnginePrivate::xorrisoResultHandler,
                              this, ISOEnginePrivate::xorrisoInfoHandler, this, 0);
}

DBURN_END_NAMESPACE
