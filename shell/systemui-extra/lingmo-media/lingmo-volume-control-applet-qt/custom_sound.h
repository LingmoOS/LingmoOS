/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef CUSTOMSOUND_H
#define CUSTOMSOUND_H
//#include <QtXml>
#include <QDomDocument>
#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QDebug>

class CustomSound
{
public:
    CustomSound();
    ~CustomSound();
    bool createAudioFile();
    bool isExist(QString nodeName);
    int addXmlNode(QString nodeNane, bool initState);
    bool isFirstRun();

private:
    QDomDocument *doc;
    QString audioPath;
};

#endif // CUSTOMSOUND_H
