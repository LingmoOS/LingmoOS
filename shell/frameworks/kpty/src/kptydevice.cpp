/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2010 KDE e.V. <kde-ev-board@kde.org>
    SPDX-FileContributor: 2010 Adriaan de Groot <groot@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kptydevice.h"
#include "kpty_p.h"

#include <config-pty.h>

#include <QSocketNotifier>

#include <KLocalizedString>

#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#if HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
// "the other end's output queue size" -- that is is our end's input
#define PTY_BYTES_AVAILABLE TIOCOUTQ
#elif defined(TIOCINQ)
// "our end's input queue size"
#define PTY_BYTES_AVAILABLE TIOCINQ
#else
// likewise. more generic ioctl (theoretically)
#define PTY_BYTES_AVAILABLE FIONREAD
#endif

#define KMAXINT ((int)(~0U >> 1))

/////////////////////////////////////////////////////
// Helper. Remove when QRingBuffer becomes public. //
/////////////////////////////////////////////////////

#include <QByteArray>
#include <QList>

#define CHUNKSIZE 4096

class KRingBuffer
{
public:
    KRingBuffer()
    {
        clear();
    }

    void clear()
    {
        buffers.clear();
        QByteArray tmp;
        tmp.resize(CHUNKSIZE);
        buffers << tmp;
        head = tail = 0;
        totalSize = 0;
    }

    inline bool isEmpty() const
    {
        return buffers.count() == 1 && !tail;
    }

    inline int size() const
    {
        return totalSize;
    }

    inline int readSize() const
    {
        return (buffers.count() == 1 ? tail : buffers.first().size()) - head;
    }

    inline const char *readPointer() const
    {
        Q_ASSERT(totalSize > 0);
        return buffers.first().constData() + head;
    }

    void free(int bytes)
    {
        totalSize -= bytes;
        Q_ASSERT(totalSize >= 0);

        for (;;) {
            int nbs = readSize();

            if (bytes < nbs) {
                head += bytes;
                if (head == tail && buffers.count() == 1) {
                    buffers.first().resize(CHUNKSIZE);
                    head = tail = 0;
                }
                break;
            }

            bytes -= nbs;
            if (buffers.count() == 1) {
                buffers.first().resize(CHUNKSIZE);
                head = tail = 0;
                break;
            }

            buffers.removeFirst();
            head = 0;
        }
    }

    char *reserve(int bytes)
    {
        totalSize += bytes;

        char *ptr;
        if (tail + bytes <= buffers.last().size()) {
            ptr = buffers.last().data() + tail;
            tail += bytes;
        } else {
            buffers.last().resize(tail);
            QByteArray tmp;
            tmp.resize(qMax(CHUNKSIZE, bytes));
            ptr = tmp.data();
            buffers << tmp;
            tail = bytes;
        }
        return ptr;
    }

    // release a trailing part of the last reservation
    inline void unreserve(int bytes)
    {
        totalSize -= bytes;
        tail -= bytes;
    }

    inline void write(const char *data, int len)
    {
        memcpy(reserve(len), data, len);
    }

    // Find the first occurrence of c and return the index after it.
    // If c is not found until maxLength, maxLength is returned, provided
    // it is smaller than the buffer size. Otherwise -1 is returned.
    int indexAfter(char c, int maxLength = KMAXINT) const
    {
        int index = 0;
        int start = head;
        QList<QByteArray>::ConstIterator it = buffers.begin();
        for (;;) {
            if (!maxLength) {
                return index;
            }
            if (index == size()) {
                return -1;
            }
            const QByteArray &buf = *it;
            ++it;
            int len = qMin((it == buffers.end() ? tail : buf.size()) - start, maxLength);
            const char *ptr = buf.data() + start;
            if (const char *rptr = (const char *)memchr(ptr, c, len)) {
                return index + (rptr - ptr) + 1;
            }
            index += len;
            maxLength -= len;
            start = 0;
        }
    }

    inline int lineSize(int maxLength = KMAXINT) const
    {
        return indexAfter('\n', maxLength);
    }

    inline bool canReadLine() const
    {
        return lineSize() != -1;
    }

    int read(char *data, int maxLength)
    {
        int bytesToRead = qMin(size(), maxLength);
        int readSoFar = 0;
        while (readSoFar < bytesToRead) {
            const char *ptr = readPointer();
            int bs = qMin(bytesToRead - readSoFar, readSize());
            memcpy(data + readSoFar, ptr, bs);
            readSoFar += bs;
            free(bs);
        }
        return readSoFar;
    }

    int readLine(char *data, int maxLength)
    {
        return read(data, lineSize(qMin(maxLength, size())));
    }

private:
    QList<QByteArray> buffers;
    int head, tail;
    int totalSize;
};

//////////////////
// private data //
//////////////////

// Lifted from Qt. I don't think they would mind. ;)
// Re-lift again from Qt whenever a proper replacement for pthread_once appears
static void qt_ignore_sigpipe()
{
    static QBasicAtomicInt atom = Q_BASIC_ATOMIC_INITIALIZER(0);
    if (atom.testAndSetRelaxed(0, 1)) {
        struct sigaction noaction;
        memset(&noaction, 0, sizeof(noaction));
        noaction.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &noaction, nullptr);
    }
}

/* clang-format off */
#define NO_INTR(ret, func) \
    do { \
        ret = func; \
    } while (ret < 0 && errno == EINTR)
/* clang-format on */

class KPtyDevicePrivate : public KPtyPrivate
{
    Q_DECLARE_PUBLIC(KPtyDevice)
public:
    KPtyDevicePrivate(KPty *parent)
        : KPtyPrivate(parent)
        , emittedReadyRead(false)
        , emittedBytesWritten(false)
        , readNotifier(nullptr)
        , writeNotifier(nullptr)
    {
    }

    bool _k_canRead();
    bool _k_canWrite();

    bool doWait(int msecs, bool reading);
    void finishOpen(QIODevice::OpenMode mode);

    bool emittedReadyRead;
    bool emittedBytesWritten;
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
    KRingBuffer readBuffer;
    KRingBuffer writeBuffer;
};

bool KPtyDevicePrivate::_k_canRead()
{
    Q_Q(KPtyDevice);
    qint64 readBytes = 0;

    int available;
    if (!::ioctl(q->masterFd(), PTY_BYTES_AVAILABLE, (char *)&available)) {
        char *ptr = readBuffer.reserve(available);
        NO_INTR(readBytes, read(q->masterFd(), ptr, available));
        if (readBytes < 0) {
            readBuffer.unreserve(available);
            q->setErrorString(i18n("Error reading from PTY"));
            return false;
        }
        readBuffer.unreserve(available - readBytes); // *should* be a no-op
    }

    if (!readBytes) {
        readNotifier->setEnabled(false);
        Q_EMIT q->readEof();
        return false;
    } else {
        if (!emittedReadyRead) {
            emittedReadyRead = true;
            Q_EMIT q->readyRead();
            emittedReadyRead = false;
        }
        return true;
    }
}

bool KPtyDevicePrivate::_k_canWrite()
{
    Q_Q(KPtyDevice);

    writeNotifier->setEnabled(false);
    if (writeBuffer.isEmpty()) {
        return false;
    }

    qt_ignore_sigpipe();
    int wroteBytes;
    NO_INTR(wroteBytes, write(q->masterFd(), writeBuffer.readPointer(), writeBuffer.readSize()));
    if (wroteBytes < 0) {
        q->setErrorString(i18n("Error writing to PTY"));
        return false;
    }
    writeBuffer.free(wroteBytes);

    if (!emittedBytesWritten) {
        emittedBytesWritten = true;
        Q_EMIT q->bytesWritten(wroteBytes);
        emittedBytesWritten = false;
    }

    if (!writeBuffer.isEmpty()) {
        writeNotifier->setEnabled(true);
    }
    return true;
}

#ifndef timeradd
// Lifted from GLIBC
/* clang-format off */
#define timeradd(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000) { \
            ++(result)->tv_sec; \
            (result)->tv_usec -= 1000000; \
        } \
    } while (0)

#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif
/* clang-format on */

bool KPtyDevicePrivate::doWait(int msecs, bool reading)
{
    Q_Q(KPtyDevice);
#ifndef Q_OS_LINUX
    struct timeval etv;
#endif
    struct timeval tv;
    struct timeval *tvp;

    if (msecs < 0) {
        tvp = nullptr;
    } else {
        tv.tv_sec = msecs / 1000;
        tv.tv_usec = (msecs % 1000) * 1000;
#ifndef Q_OS_LINUX
        gettimeofday(&etv, nullptr);
        timeradd(&tv, &etv, &etv);
#endif
        tvp = &tv;
    }

    while (reading ? readNotifier->isEnabled() : !writeBuffer.isEmpty()) {
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        if (readNotifier->isEnabled()) {
            FD_SET(q->masterFd(), &rfds);
        }
        if (!writeBuffer.isEmpty()) {
            FD_SET(q->masterFd(), &wfds);
        }

#ifndef Q_OS_LINUX
        if (tvp) {
            gettimeofday(&tv, nullptr);
            timersub(&etv, &tv, &tv);
            if (tv.tv_sec < 0) {
                tv.tv_sec = tv.tv_usec = 0;
            }
        }
#endif

        switch (select(q->masterFd() + 1, &rfds, &wfds, nullptr, tvp)) {
        case -1:
            if (errno == EINTR) {
                break;
            }
            return false;
        case 0:
            q->setErrorString(i18n("PTY operation timed out"));
            return false;
        default:
            if (FD_ISSET(q->masterFd(), &rfds)) {
                bool canRead = _k_canRead();
                if (reading && canRead) {
                    return true;
                }
            }
            if (FD_ISSET(q->masterFd(), &wfds)) {
                bool canWrite = _k_canWrite();
                if (!reading) {
                    return canWrite;
                }
            }
            break;
        }
    }
    return false;
}

void KPtyDevicePrivate::finishOpen(QIODevice::OpenMode mode)
{
    Q_Q(KPtyDevice);

    q->QIODevice::open(mode);
    fcntl(q->masterFd(), F_SETFL, O_NONBLOCK);
    readBuffer.clear();
    readNotifier = new QSocketNotifier(q->masterFd(), QSocketNotifier::Read, q);
    writeNotifier = new QSocketNotifier(q->masterFd(), QSocketNotifier::Write, q);
    QObject::connect(readNotifier, &QSocketNotifier::activated, q, [this]() {
        _k_canRead();
    });
    QObject::connect(writeNotifier, &QSocketNotifier::activated, q, [this]() {
        _k_canWrite();
    });
    readNotifier->setEnabled(true);
}

/////////////////////////////
// public member functions //
/////////////////////////////

KPtyDevice::KPtyDevice(QObject *parent)
    : QIODevice(parent)
    , KPty(new KPtyDevicePrivate(this))
{
}

KPtyDevice::~KPtyDevice()
{
    close();
}

bool KPtyDevice::open(OpenMode mode)
{
    Q_D(KPtyDevice);

    if (masterFd() >= 0) {
        return true;
    }

    if (!KPty::open()) {
        setErrorString(i18n("Error opening PTY"));
        return false;
    }

    d->finishOpen(mode);

    return true;
}

bool KPtyDevice::open(int fd, OpenMode mode)
{
    Q_D(KPtyDevice);

    if (!KPty::open(fd)) {
        setErrorString(i18n("Error opening PTY"));
        return false;
    }

    d->finishOpen(mode);

    return true;
}

void KPtyDevice::close()
{
    Q_D(KPtyDevice);

    if (masterFd() < 0) {
        return;
    }

    delete d->readNotifier;
    delete d->writeNotifier;

    QIODevice::close();

    KPty::close();
}

bool KPtyDevice::isSequential() const
{
    return true;
}

bool KPtyDevice::canReadLine() const
{
    Q_D(const KPtyDevice);
    return QIODevice::canReadLine() || d->readBuffer.canReadLine();
}

bool KPtyDevice::atEnd() const
{
    Q_D(const KPtyDevice);
    return QIODevice::atEnd() && d->readBuffer.isEmpty();
}

qint64 KPtyDevice::bytesAvailable() const
{
    Q_D(const KPtyDevice);
    return QIODevice::bytesAvailable() + d->readBuffer.size();
}

qint64 KPtyDevice::bytesToWrite() const
{
    Q_D(const KPtyDevice);
    return d->writeBuffer.size();
}

bool KPtyDevice::waitForReadyRead(int msecs)
{
    Q_D(KPtyDevice);
    return d->doWait(msecs, true);
}

bool KPtyDevice::waitForBytesWritten(int msecs)
{
    Q_D(KPtyDevice);
    return d->doWait(msecs, false);
}

void KPtyDevice::setSuspended(bool suspended)
{
    Q_D(KPtyDevice);
    d->readNotifier->setEnabled(!suspended);
}

bool KPtyDevice::isSuspended() const
{
    Q_D(const KPtyDevice);
    return !d->readNotifier->isEnabled();
}

// protected
qint64 KPtyDevice::readData(char *data, qint64 maxlen)
{
    Q_D(KPtyDevice);
    return d->readBuffer.read(data, (int)qMin<qint64>(maxlen, KMAXINT));
}

// protected
qint64 KPtyDevice::readLineData(char *data, qint64 maxlen)
{
    Q_D(KPtyDevice);
    return d->readBuffer.readLine(data, (int)qMin<qint64>(maxlen, KMAXINT));
}

// protected
qint64 KPtyDevice::writeData(const char *data, qint64 len)
{
    Q_D(KPtyDevice);
    Q_ASSERT(len <= KMAXINT);

    d->writeBuffer.write(data, len);
    d->writeNotifier->setEnabled(true);
    return len;
}

#include "moc_kptydevice.cpp"
