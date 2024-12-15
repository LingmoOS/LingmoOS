// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FIFO_H
#define FIFO_H

#include <QObject>
#include <QString>

class Fifo : public QObject
{
public:
    Fifo();
    ~Fifo();

    int Read(QString &data);
    int Write(QString data);
    int OpenRead();
    int OpenWrite();

private:
    int m_fd;
    QString m_fifoPath;
};

#endif //FIFO_H
