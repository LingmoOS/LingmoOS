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

#ifndef CFILESIZEJOB_H
#define CFILESIZEJOB_H

#include <QThread>
#include <QList>
#include <QUrl>

class CFileSizeJob : public QThread
{
    Q_OBJECT

public:
    explicit CFileSizeJob(QObject *parent = nullptr);
    ~CFileSizeJob();

    qint64 totalSize() const;

    void start(const QList<QUrl> &urls);
    void stop();

signals:
    void sizeChanged();
    void result();

protected:
    void run() override;

private:
    bool m_running;
    QList<QUrl> m_urls;
    qint64 m_totalSize;
    int m_filesCount;
    int m_directoryCount;
};

#endif // CFILESIZEJOB_H
