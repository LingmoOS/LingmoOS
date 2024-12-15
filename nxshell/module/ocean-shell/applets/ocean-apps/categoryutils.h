// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>

namespace CategoryUtils {

enum class Categorytype {
    CategoryInternet,
    CategoryChat,
    CategoryMusic,
    CategoryVideo,
    CategoryGraphics,
    CategoryGame,
    CategoryOffice,
    CategoryReading,
    CategoryDevelopment,
    CategorySystem,
    CategoryOthers,
    CategoryErr,
};

Categorytype parseBestMatchedCategory(QStringList categories);
Categorytype parseOCEANCategoryString(QString str);
QList<Categorytype> parseXdgCategoryString(QString str);

}
