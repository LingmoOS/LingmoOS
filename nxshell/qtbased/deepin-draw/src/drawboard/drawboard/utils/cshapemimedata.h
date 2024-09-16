// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSHAPEMIMEDATA_H
#define CSHAPEMIMEDATA_H

#include "sitemdata.h"
#include <QMimeData>

class PageItem;

class DRAWLIB_EXPORT DrawBoardMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit DrawBoardMimeData(const UnitList &u);
    ~DrawBoardMimeData();

    bool hasFormat(const QString &mimetype) const;

    UnitList unit()const;

private:
    UnitList ut;
};

#endif // CSHAPEMIMEDATA_H
