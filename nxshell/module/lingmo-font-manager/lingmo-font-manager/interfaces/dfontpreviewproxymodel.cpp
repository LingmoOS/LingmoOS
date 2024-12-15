// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewproxymodel.h"
#include "dfontpreviewitemdef.h"
#include "views/dsplitlistwidget.h"
#include "dfontpreviewlistdatathread.h"

DFontPreviewProxyModel::DFontPreviewProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_filterGroup(DSplitListWidget::AllFont)
    , m_useSystemFilter(true)
    , m_fontNamePattern("")
{
}

DFontPreviewProxyModel::~DFontPreviewProxyModel()
{
}

/*************************************************************************
 <Function>      setFilterGroup
 <Description>   设置当前字体组
 <Author>        null
 <Input>
    <param1>     filterGroup            Description:字体组编号
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewProxyModel::setFilterGroup(int filterGroup)
{
    m_useSystemFilter = false;
    m_filterGroup = filterGroup;

    invalidateFilter();
}

int DFontPreviewProxyModel::getFilterGroup()
{
    return m_filterGroup;
}

/*************************************************************************
 <Function>      setFilterFontNamePattern
 <Description>   根据输入内容设置比对的模板
 <Author>        null
 <Input>
    <param1>     pattern            Description:用户输入内容
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewProxyModel::setFilterFontNamePattern(const QString &pattern)
{
    m_useSystemFilter = false;
    m_fontNamePattern = pattern;

    invalidateFilter();
}

//bool DFontPreviewProxyModel::getEditStatus() const
//{
//    return m_editStatus;
//}

//void DFontPreviewProxyModel::setEditStatus(bool editStatus)
//{
//    m_editStatus = editStatus;
//}

/*************************************************************************
 <Function>      isFontNameContainsPattern
 <Description>   判断用户字体名是否含有用户输入的内容
 <Author>        null
 <Input>
    <param1>     fontName            Description:字体名
 <Return>        bool                Description:字体名是否包含用户输入内容
 <Note>          null
*************************************************************************/
bool DFontPreviewProxyModel::isFontNameContainsPattern(QString fontName) const
{
    if (m_fontNamePattern.length() > 0) {
        return fontName.contains(m_fontNamePattern, Qt::CaseInsensitive);
    }

    return true;
}


/*************************************************************************
 <Function>      isCustomFilterAcceptsRow
 <Description>   根据字体的信息进行分组,决定各个界面显示的字体
 <Author>        null
 <Input>
    <param1>     modelIndex            Description:model索引
 <Return>        bool                  Description:是否包含
 <Note>          null
*************************************************************************/
bool DFontPreviewProxyModel::isCustomFilterAcceptsRow(const QModelIndex &modelIndex) const
{
    QVariant varModel = sourceModel()->data(modelIndex, Qt::DisplayRole);
    if (varModel.isValid() == false) {
        return false;
    }

    //zhangya 20200313  fix varModel is not DFontPreviewItemData crash
    if (varModel.canConvert<FontData>() == false) {
        return false;
    }

    FontData fdata = varModel.value<FontData>();

    const QString &fontName = fdata.strFontName;

    switch (m_filterGroup) {
    //显示所有字体
    case DSplitListWidget::AllFont: {
        if (m_fontNamePattern.length() > 0) {
            return isFontNameContainsPattern(fontName);
        }
        return true;
    }
    //只显示系统字体
    case DSplitListWidget::SysFont: {
        if (fdata.isSystemFont &&
                isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //只显示用户字体
    case DSplitListWidget::UserFont: {
        if ((!fdata.isSystemFont) &&
                isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //只显示收藏字体
    case DSplitListWidget::CollectFont: {
        if (fdata.isCollected() && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //已激活(启用)字体
    case DSplitListWidget::ActiveFont: {
        if (fdata.isEnabled() && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //中文字体
    case DSplitListWidget::ChineseFont: {
        if (fdata.isChinese() && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //等宽字体
    case DSplitListWidget::EqualWidthFont: {
        if (fdata.isMonoSpace() && isFontNameContainsPattern(fontName)) {
            return true;
        }

    } break;
    default:
        break;
    }

    return false;
}

/*************************************************************************
 <Function>      filterAcceptsRow
 <Description>   判断给定项是否包含在模型中
 <Author>        null
 <Input>
    <param1>     source_row               Description:null
    <param2>     source_parent            Description:null
 <Return>        bool            Description:是否包含
 <Note>          null
*************************************************************************/
bool DFontPreviewProxyModel::filterAcceptsRow(int source_row,
                                              const QModelIndex &source_parent) const
{
    if (m_useSystemFilter) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    QModelIndex modelIndex = sourceModel()->index(source_row, 0, source_parent);

    if (modelIndex.isValid() == false) {
        return false;
    }

    if (isCustomFilterAcceptsRow(modelIndex)) {
        return true;
    } else {
        return false;
    }
}

/*************************************************************************
 <Function>      rowCount
 <Description>   获取行数
 <Author>        null
 <Input>
    <param1>     parent            Description:无用参数
 <Return>        int               Description:行数
 <Note>          null
*************************************************************************/
int DFontPreviewProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    int filterRowCount = QSortFilterProxyModel::rowCount();
//    qDebug() << "filterRowCount" << filterRowCount;

//    unsigned int bShow = 0;
//    if (0 == filterRowCount) {
//        if (getEditStatus()) {
//            bShow = 2;
//        } else {
//            bShow = 1;
//        }
//    } else {
//        bShow = 0;

//    }

//    emit this->onFilterFinishRowCountChangedInt(bShow);

    return filterRowCount;
}
