/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef RESULTITEMPRIVATE_H
#define RESULTITEMPRIVATE_H
#include "result-item.h"
#include "result-item-private.h"
namespace LingmoUISearch {
class ResultItemPrivate
{
public:
    explicit ResultItemPrivate(ResultItem *parent);
    ~ResultItemPrivate();
    void setSearchId(size_t searchId);
    void setItemKey(QString itemKey);
    void setExtral(QVariantList extral);
    size_t getSearchId();
    QString getItemKey();
    QVariantList getExtral();

private:
    size_t m_searchId = 0;
    QString m_itemKey;
    QVariantList m_extral;
    //and something else...

    ResultItem *q;

};

}
#endif // RESULTITEMPRIVATE_H
