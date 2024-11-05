/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */

#include "bw-list-info.h"

using namespace Peony;

BWListInfo::BWListInfo(QObject *parent):QObject(parent)
{
    m_workModel = BW_LIST_NORMAL;
}

BWListInfo::~BWListInfo()
{
    clearBWlist();
}

void BWListInfo::setBWListModel(QString model)
{
    m_workModel = model;
}

int BWListInfo::addBWListElement(QString desktopName)
{
    QStringList strList = desktopName.split("/");
   // qDebug()<<"add bw list element" << strList.last();
    m_bwListInfo.insert(strList.last());

    return 0;
}

int BWListInfo::delBWListElement(QString desktopName)
{
    m_bwListInfo.remove(desktopName);
    return 0;
}

int BWListInfo::clearBWlist()
{
    if (!m_bwListInfo.isEmpty())
    {
        m_bwListInfo.clear();
    }

    m_workModel = BW_LIST_NORMAL;
    \
    return 0;
}

bool BWListInfo::desktopNameExist(QString desktop)
{
    //qDebug()<<"check desktop exsit" << desktop;
    return m_bwListInfo.contains(desktop);
}

bool BWListInfo::isBlackListMode()
{
    return m_workModel == BW_LIST_BLACK;
}

bool BWListInfo::isWriteListMode()
{
    return m_workModel == BW_LIST_WHITE;
}

bool BWListInfo::isNormalMode()
{
    return m_workModel == BW_LIST_NORMAL;
}
