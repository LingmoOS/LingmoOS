// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DZLIBIODEVICE_H
#define DZLIBIODEVICE_H

#include <QIODevice>

class DZlibIODevice : public QIODevice
{
    Q_OBJECT

public:
    explicit DZlibIODevice(QObject *parent = 0);
    explicit DZlibIODevice(QIODevice *m_device, QObject *parent = 0);
    ~DZlibIODevice();

    void setDevice(QIODevice *device);

    bool isSequential() const Q_DECL_OVERRIDE;

    bool open(OpenMode mode) Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;

    qint64 pos() const Q_DECL_OVERRIDE;
    qint64 size() const Q_DECL_OVERRIDE;
    bool atEnd() const Q_DECL_OVERRIDE;

    qint64 bytesAvailable() const Q_DECL_OVERRIDE;
    qint64 bytesToWrite() const Q_DECL_OVERRIDE;

    bool canReadLine() const Q_DECL_OVERRIDE;

    bool waitForReadyRead(int msecs) Q_DECL_OVERRIDE;
    bool waitForBytesWritten(int msecs) Q_DECL_OVERRIDE;

    QString errorString() const;

    static constexpr int metaDataSize();

protected:
    qint64 readData(char *data, qint64 maxlen)  Q_DECL_OVERRIDE;
    qint64 writeData(const char *data, qint64 len)  Q_DECL_OVERRIDE;

    QByteArray compress(const QByteArray &data) const;
    QByteArray uncompress(const QByteArray &data) const;

private:
    bool isReadMode() const;
    bool isWriteMode() const;
    void readNextBlock();
    bool writeToBlock();

    QIODevice *m_device;
    QByteArray m_readBuffer;
    QByteArray m_writeBuffer;
    qint64 m_currentBlock = -1;

    qint64 m_size = 0;
    qint64 m_blockCount = 0;
    qint32 m_lastBlockSize = 0;
};

#endif // DZLIBIODEVICE_H
