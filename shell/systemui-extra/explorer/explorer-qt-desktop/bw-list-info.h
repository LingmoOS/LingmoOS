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

#ifndef BWLISTINFO_H
#define BWLISTINFO_H

#include <QSet>
#include <QDebug>


namespace Peony {

#define BW_LIST_NORMAL  "normal"
#define BW_LIST_BLACK   "blacklist"
#define BW_LIST_WHITE   "whitelist"

class BWListInfo:public QObject
{
    Q_OBJECT

public:
    explicit BWListInfo(QObject *parent = nullptr);
    ~BWListInfo();

public:
    void setBWListModel(QString model);
    int addBWListElement(QString desktopName);
    int delBWListElement(QString desktopName);
    int clearBWlist();
    bool desktopNameExist(QString desktop);

    bool isBlackListMode();
    bool isWriteListMode();
    bool isNormalMode();

    QString getMode(){return m_workModel;}
    QSet<QString> getBWListInfo() {return m_bwListInfo;}

private:
    QSet<QString> m_bwListInfo;
    QString m_workModel;
};

}
#endif // BWLISTINFO_H
