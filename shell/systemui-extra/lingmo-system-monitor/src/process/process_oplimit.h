/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PROCESS_OPLIMIT_H__
#define __PROCESS_OPLIMIT_H__

#include <QObject>
#include <QList>
#include <QFileSystemWatcher>

#define KSEC_PROCPROTECTED_FILE "/etc/kysec/ppro/ppro.xml"
#define KSEC_PROCPROTECTED_SWITCH "/sys/kernel/security/kysec/ppro"

class ProcessOpLimit : public QObject
{
    Q_OBJECT
public:
    ProcessOpLimit(QObject *parent = nullptr);
    ~ProcessOpLimit();

    bool isProtectedProc(QString strCmd);
    void parseProtectedXml(QString strFilePath, QList<QString>& listPath);

public slots:
    void onFileChanged(QString);

private:
    void init();
    bool isAppSecurityOpened();

private:
    QList<QString> m_listProtected;
    bool m_isInited;
    QFileSystemWatcher m_fsWatcher;
};

#endif // __PROCESS_OPLIMIT_H__
