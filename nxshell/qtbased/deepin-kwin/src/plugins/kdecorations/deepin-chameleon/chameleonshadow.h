/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef CHAMELEONSHADOW_H
#define CHAMELEONSHADOW_H

#include "chameleontheme.h"

#include <KDecoration2/DecorationShadow>

#include <QSharedPointer>

class ChameleonShadow
{
public:
    static ChameleonShadow *instance();

    static QString buildShadowCacheKey(const ChameleonTheme::ThemeConfig *config, qreal scale);
    QSharedPointer<KDecoration2::DecorationShadow> getShadow(const ChameleonTheme::ThemeConfig *config, qreal scale);

    void clearCache();

protected:
    ChameleonShadow();

private:
    QMap<QString, QSharedPointer<KDecoration2::DecorationShadow>> m_shadowCache;
    QSharedPointer<KDecoration2::DecorationShadow> m_emptyShadow;
};

#endif // CHAMELEONSHADOW_H
