// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <QDebug>

Fifo::Fifo()
    : m_fd(-1)
{
    m_fifoPath = QString(getenv("HOME")) + "/.cache/dde-session-fifo";
}

Fifo::~Fifo()
{
    if (m_fd > 0) {
        close(m_fd);
    }
}

int Fifo::OpenRead()
{
    if(mkfifo(m_fifoPath.toStdString().c_str(), 0666) < 0 && errno != EEXIST) {
        qDebug() << "mkfifo error";
        return -1;
    }
    m_fd = open(m_fifoPath.toStdString().c_str(), O_RDONLY);
    if (m_fd < 0) {
        qDebug() << "open fifo error";
        return -1;
    }
    return 0;
}

int Fifo::Write(QString data)
{
    if (m_fd < 0) {
        qDebug() << "write fifo error";
        return -1;
    }
    write(m_fd, data.toStdString().c_str(), size_t(data.length()));

    return 0;
}

int Fifo::Read(QString &data)
{
    if (m_fd < 0) {
        qDebug() << "read fifo error";
        return -1;
    }
    int len = 0;
    char buf[1024] = { 0 };
    len = read(m_fd, buf, sizeof(buf));
    if (len > 0) {
        data = QString::fromStdString(buf);
    }

    return len;
}

int Fifo::OpenWrite()
{
    if(mkfifo(m_fifoPath.toStdString().c_str(), 0666) < 0 && errno != EEXIST) {
        qDebug() << "mkfifo error";
        return -1;
    }
    // signal(SIGPIPE, [](int ret){
    //     qDebug() << "SIGQUIT catched!";
    // });
    m_fd = open(m_fifoPath.toStdString().c_str(), O_WRONLY);
    if (m_fd < 0) {
        qDebug() << "open fifo error";
        return -1;
    }
    return 0;
}
