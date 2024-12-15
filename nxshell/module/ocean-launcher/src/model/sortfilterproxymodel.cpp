// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortfilterproxymodel.h"
#include "appslistmodel.h"
#include "iteminfo.h"
#include "languagetranformation.h"

#include <QDebug>

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel (parent)
    , m_languageSwitch(LanguageTransformation::instance())
{
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const ItemInfo_v1 &info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    QString jianpinStr = m_languageSwitch->zhToJianPin(info.m_name);
    QString pinyinStr = m_languageSwitch->zhToPinYin(info.m_name);
    QString searchedText = filterRegExp().pattern();

    return info.m_desktop.contains(searchedText, Qt::CaseInsensitive) ||
           info.m_name.contains(searchedText, Qt::CaseInsensitive) ||
           info.m_key.contains(searchedText, Qt::CaseInsensitive) ||
           jianpinStr.contains(searchedText, Qt::CaseInsensitive) ||
           pinyinStr.contains(searchedText, Qt::CaseInsensitive);
}
