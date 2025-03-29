/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cfilesizejob.h"
#include <QQueue>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <KIO/CopyJob>

CFileSizeJob::CFileSizeJob(QObject *parent)
    : QThread(parent)
{

}

CFileSizeJob::~CFileSizeJob()
{
}

qint64 CFileSizeJob::totalSize() const
{
    return m_totalSize;
}

void CFileSizeJob::start(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    m_urls = urls;
    m_running = true;

    QThread::start();
}

void CFileSizeJob::stop()
{
    m_running = false;

    QThread::wait();
}

void CFileSizeJob::run()
{
    m_totalSize = 0;
    m_filesCount = 0;
    m_directoryCount = 0;

    for (QUrl &url : m_urls) {
        if (!m_running)
            return;

        QFileInfo i(url.toLocalFile());

        if (i.filePath() == "/proc/kcore" || i.filePath() == "/dev/core")
            continue;

        if (i.isSymLink() && i.symLinkTarget() == "/proc/kcore")
            continue;

        if (i.isFile()) {
            m_totalSize += i.size();
            m_filesCount++;
            emit sizeChanged();
        } else if (i.isDir()) {
            m_directoryCount++;

            QDirIterator it(url.toLocalFile(), QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

            while (it.hasNext()) {
                if (!m_running)
                    return;

                QFileInfo info(it.next());

                if (info.filePath() == "/proc/kcore" || info.filePath() == "/dev/core")
                    continue;

                if (info.isSymLink())
                    continue;

                if (info.isFile())
                    m_filesCount++;
                else if (info.isDir())
                    m_directoryCount++;

                m_totalSize += info.size();

                emit sizeChanged();
            }
        }
    }

    emit result();
}
