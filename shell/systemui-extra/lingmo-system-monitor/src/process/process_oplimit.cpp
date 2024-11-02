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

#include "process_oplimit.h"

#include <stdio.h>
#include <QStringList>
#include <QRegExp>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

ProcessOpLimit::ProcessOpLimit(QObject *parent)
    : QObject(parent)
{
    m_isInited = false;
    m_listProtected.clear();
}

ProcessOpLimit::~ProcessOpLimit()
{

}

void ProcessOpLimit::init()
{
    if (!m_isInited) {
        m_fsWatcher.addPath(KSEC_PROCPROTECTED_FILE);
        connect(&m_fsWatcher,SIGNAL(fileChanged(QString)),this, SLOT(onFileChanged(QString)));
        parseProtectedXml(KSEC_PROCPROTECTED_FILE, m_listProtected);
        m_isInited = true;
    }
}

bool ProcessOpLimit::isProtectedProc(QString strCmd)
{
    if (!isAppSecurityOpened()) {
        return false;
    }
    init();
    QStringList strList = strCmd.split(QRegExp("\\s+"));
    if (!strList.isEmpty()) {
        for (int n = 0; n < m_listProtected.size(); n ++) {
            if (m_listProtected.contains(strList[0])) {
                return true;
            }
        }
    }
    return false;
}

void ProcessOpLimit::parseProtectedXml(QString strFilePath, QList<QString>& listPath)
{
    QFile file(strFilePath);
    listPath.clear();
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error: Cannot read file " << strFilePath <<",errMsg:"
                << file.errorString();
        return;
    }
    QXmlStreamReader reader;
    reader.setDevice(&file);

    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isStartElement()) {
            if (reader.name() == "ppro_root") {
                reader.readNext();
                while (!reader.atEnd()) {
                    if (reader.isEndElement()) {
                        reader.readNext();
                        continue;
                    }
                    if (reader.isStartElement()) {
                        if (reader.name() == "ppro_node") {
                            QStringRef strPathRef = reader.attributes().value("path");
                            if (!strPathRef.isNull()) {
                                listPath.append(strPathRef.toString());
                            }
                            reader.readNext();
                            while (!reader.atEnd()) {
                                if (reader.isEndElement()) {
                                    reader.readNext();
                                    break;
                                }
                                if (reader.isStartElement()) {
                                    if (reader.name() == "type") {
                                        QString strType = reader.readElementText();
                                        if (!strType.isEmpty()) {
                                            int nType = strType.toInt();
                                            if (nType == 1) { // 防杀死
                                            }
                                        }
                                    } else if (reader.name() == "hash") {
                                    }
                                }
                                reader.readNext();
                            }
                            continue;
                        }
                    }
                    reader.readNext();
                }
                continue;
            }
        }
        reader.readNext();
    }
    file.close();
    qInfo()<<"PprocList:"<<listPath;
}

void ProcessOpLimit::onFileChanged(QString strFile)
{
    qInfo()<<"PProcsChanged:"<<strFile;
    if (strFile == KSEC_PROCPROTECTED_FILE) {
        parseProtectedXml(KSEC_PROCPROTECTED_FILE, m_listProtected);
    }
}

bool ProcessOpLimit::isAppSecurityOpened()
{
    FILE *pFile = fopen(KSEC_PROCPROTECTED_SWITCH,"r");
    if (pFile) {
        char chSwitch = 0;
        size_t readSize = fread(&chSwitch, sizeof(char), 1, pFile);
        fclose(pFile);
        pFile = NULL;
        if (readSize > 0 && chSwitch != '0') {
            return true;
        }
    }
    return false;
}
