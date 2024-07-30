/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kptydev_h
#define kptydev_h

#include "kpty.h"

#include <QIODevice>

class KPtyDevicePrivate;

/**
 * Encapsulates KPty into a QIODevice, so it can be used with Q*Stream, etc.
 */
class KPTY_EXPORT KPtyDevice : public QIODevice, public KPty // krazy:exclude=dpointer (via macro)
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(KPty::d_ptr, KPtyDevice)

public:
    /**
     * Constructor
     */
    explicit KPtyDevice(QObject *parent = nullptr);

    /**
     * Destructor:
     *
     *  If the pty is still open, it will be closed. Note, however, that
     *  an utmp registration is @em not undone.
     */
    ~KPtyDevice() override;

    /**
     * Create a pty master/slave pair.
     *
     * @return true if a pty pair was successfully opened
     */
    bool open(OpenMode mode = ReadWrite | Unbuffered) override;

    /**
     * Open using an existing pty master. The ownership of the fd
     * remains with the caller, i.e., close() will not close the fd.
     *
     * This is useful if you wish to attach a secondary "controller" to an
     * existing pty device such as a terminal widget.
     * Note that you will need to use setSuspended() on both devices to
     * control which one gets the incoming data from the pty.
     *
     * @param fd an open pty master file descriptor.
     * @param mode the device mode to open the pty with.
     * @return true if a pty pair was successfully opened
     */
    bool open(int fd, OpenMode mode = ReadWrite | Unbuffered);

    /**
     * Close the pty master/slave pair.
     */
    void close() override;

    /**
     * Sets whether the KPtyDevice monitors the pty for incoming data.
     *
     * When the KPtyDevice is suspended, it will no longer attempt to buffer
     * data that becomes available from the pty and it will not emit any
     * signals.
     *
     * Do not use on closed ptys.
     * After a call to open(), the pty is not suspended. If you need to
     * ensure that no data is read, call this function before the main loop
     * is entered again (i.e., immediately after opening the pty).
     */
    void setSuspended(bool suspended);

    /**
     * Returns true if the KPtyDevice is not monitoring the pty for incoming
     * data.
     *
     * Do not use on closed ptys.
     *
     * See setSuspended()
     */
    bool isSuspended() const;

    /**
     * @return always true
     */
    bool isSequential() const override;

    /**
     * @reimp
     */
    bool canReadLine() const override;

    /**
     * @reimp
     */
    bool atEnd() const override;

    /**
     * @reimp
     */
    qint64 bytesAvailable() const override;

    /**
     * @reimp
     */
    qint64 bytesToWrite() const override;

    bool waitForBytesWritten(int msecs = -1) override;
    bool waitForReadyRead(int msecs = -1) override;

Q_SIGNALS:
    /**
     * Emitted when EOF is read from the PTY.
     *
     * Data may still remain in the buffers.
     */
    void readEof();

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 readLineData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
};

#endif
