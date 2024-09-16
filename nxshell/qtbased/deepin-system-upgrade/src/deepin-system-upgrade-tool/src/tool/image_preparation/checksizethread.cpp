// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QFile>
#include <QString>

#include "checksizethread.h"

// Read Buffer Size: 512K
#define READ_BUFFER_SZ 512*1024

CheckSizeThread::CheckSizeThread(const QString &urlString): m_url(urlString) {}

void CheckSizeThread::run()
{
    char buf[READ_BUFFER_SZ + 1];
    FILE *pipe = popen(QString("curl -sS -I '%1' 2>err_curl.log | grep -i content-length | awk '{print $2}'").arg(m_url).toStdString().c_str(), "r");
    fgets(buf, READ_BUFFER_SZ, pipe);
    bool ok;
    long long totalSize = QString(buf).toLongLong(&ok);
    if (!ok)
    {
        QFile curlErrFile("err_curl.log");
        QString errMsg(tr("Unable to get the image file."));
        if (curlErrFile.open(QFile::ReadOnly | QFile::Text))
        {
            errMsg += QString("\n%1").arg(QString(curlErrFile.readAll()));
            curlErrFile.remove();
        }

        emit error(errMsg);
    }
    else
    {
        emit done(totalSize);
    }
}
