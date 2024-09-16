// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SideBarImageViewModel.h"
#include "DocSheet.h"
#include "PageRenderThread.h"
#include "Application.h"

#include <QTimer>
#include <QtDebug>

ImagePageInfo_t::ImagePageInfo_t(int index): pageIndex(index)
{

}

bool ImagePageInfo_t::operator == (const ImagePageInfo_t &other) const
{
    return (this->pageIndex == other.pageIndex);
}

bool ImagePageInfo_t::operator < (const ImagePageInfo_t &other) const
{
    return (this->pageIndex < other.pageIndex);
}

bool ImagePageInfo_t::operator > (const ImagePageInfo_t &other) const
{
    return (this->pageIndex > other.pageIndex);
}

SideBarImageViewModel::SideBarImageViewModel(DocSheet *sheet, QObject *parent)
    : QAbstractListModel(parent)
    , m_parent(parent), m_sheet(sheet)
{
    connect(m_sheet, &DocSheet::sigPageModified, this, &SideBarImageViewModel::onUpdateImage);
}

void SideBarImageViewModel::resetData()
{
    beginResetModel();
    m_pagelst.clear();
    endResetModel();
}

void SideBarImageViewModel::initModelLst(const QList<ImagePageInfo_t> &pagelst, bool sort)
{
    beginResetModel();

    m_pagelst = pagelst;

    if (sort)
        std::sort(m_pagelst.begin(), m_pagelst.end());

    endResetModel();
}

void SideBarImageViewModel::changeModelData(const QList<ImagePageInfo_t> &pagelst)
{
    m_pagelst = pagelst;
}

void SideBarImageViewModel::setBookMarkVisible(int index, bool visible, bool updateIndex)
{
    m_cacheBookMarkMap.insert(index, visible);
    if (updateIndex) {
        const QList<QModelIndex> &modelIndexlst = getModelIndexForPageIndex(index);
        for (const QModelIndex &modelIndex : modelIndexlst)
            emit dataChanged(modelIndex, modelIndex);
    }
}

int SideBarImageViewModel::rowCount(const QModelIndex &) const
{
    return m_pagelst.size();
}

int SideBarImageViewModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant SideBarImageViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int nRow = m_pagelst.at(index.row()).pageIndex;

    if (-1 == nRow)
        return QVariant();

    if (role == ImageinfoType_e::IMAGE_PIXMAP) {
        QPixmap pixmap = m_sheet->thumbnail(nRow);

        if (pixmap.isNull()) {
            //先填充空白
            QPixmap emptyPixmap(174, 174);
            emptyPixmap.fill(Qt::white);
            m_sheet->setThumbnail(nRow, emptyPixmap);

            //使用线程
            DocPageThumbnailTask task;
            task.sheet = m_sheet;
            task.index = nRow;
            task.model = const_cast<SideBarImageViewModel *>(this);
            PageRenderThread::appendTask(task);
        }

        return QVariant::fromValue(pixmap);
    } else if (role == ImageinfoType_e::IMAGE_BOOKMARK) {
        if (m_cacheBookMarkMap.contains(nRow)) {
            return QVariant::fromValue(m_cacheBookMarkMap.value(nRow));
        }
        return QVariant::fromValue(false);
    } else if (role == ImageinfoType_e::IMAGE_ROTATE) {
        return QVariant::fromValue(m_sheet->operation().rotation * 90);
    } else if (role == ImageinfoType_e::IMAGE_INDEX_TEXT) {
        return QVariant::fromValue(tr("Page %1").arg(nRow + 1));
    } else if (role == ImageinfoType_e::IMAGE_CONTENT_TEXT) {
        return QVariant::fromValue(m_pagelst.at(index.row()).strcontents);
    } else if (role == ImageinfoType_e::IMAGE_SEARCH_COUNT) {
        return QVariant::fromValue(m_pagelst.at(index.row()).strSearchcount);
    } else if (role == Qt::AccessibleTextRole) {
        return index.row();
    } else if (role == ImageinfoType_e::IMAGE_PAGE_SIZE) {
        return QVariant::fromValue(m_sheet->pageSizeByIndex(nRow));
    }
    return QVariant();
}

bool SideBarImageViewModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
    if (!index.isValid())
        return false;
    return QAbstractListModel::setData(index, data, role);
}

QList<QModelIndex> SideBarImageViewModel::getModelIndexForPageIndex(int pageIndex)
{
    QList<QModelIndex> modelIndexlst;

    int pageSize = m_pagelst.size();

    for (int index = 0; index < pageSize; index++) {
        if (m_pagelst.at(index) == ImagePageInfo_t(pageIndex))
            modelIndexlst << this->index(index);
    }
    return modelIndexlst;
}

int SideBarImageViewModel::getPageIndexForModelIndex(int row)
{
    if (row >= 0 && row < m_pagelst.size())
        return m_pagelst.at(row).pageIndex;
    return -1;
}

void SideBarImageViewModel::onUpdateImage(int index)
{
    DocPageThumbnailTask task;
    task.sheet = m_sheet;
    task.index = index;
    task.model = const_cast<SideBarImageViewModel *>(this);
    PageRenderThread::appendTask(task);
}

void SideBarImageViewModel::insertPageIndex(int pageIndex)
{
    if (!m_pagelst.contains(ImagePageInfo_t(pageIndex))) {
        int iterIndex = 0;
        int rowCount = m_pagelst.size();
        for (iterIndex = 0; iterIndex < rowCount; iterIndex++) {
            if (pageIndex < m_pagelst.at(iterIndex).pageIndex)
                break;
        }
        ImagePageInfo_t tImageinfo(pageIndex);
        m_pagelst.insert(iterIndex, tImageinfo);
        beginInsertRows(this->index(iterIndex).parent(), iterIndex, iterIndex);
        endInsertRows();
    }
}

void SideBarImageViewModel::insertPageIndex(const ImagePageInfo_t &tImagePageInfo)
{
    int index = -1;
    if (tImagePageInfo.annotation == nullptr) {
        index = m_pagelst.indexOf(tImagePageInfo);
    } else {
        index = findItemForAnno(tImagePageInfo.annotation);
    }

    if (index == -1) {
        int iterIndex = 0;
        int rowCount = m_pagelst.size();
        for (iterIndex = 0; iterIndex < rowCount; iterIndex++) {
            if (tImagePageInfo.pageIndex < m_pagelst.at(iterIndex).pageIndex)
                break;
        }
        m_pagelst.insert(iterIndex, tImagePageInfo);
        beginInsertRows(this->index(iterIndex).parent(), iterIndex, iterIndex);
        endInsertRows();
    } else {
        m_pagelst[index].strcontents = tImagePageInfo.strcontents;
        emit dataChanged(this->index(index), this->index(index));
    }
}

void SideBarImageViewModel::removePageIndex(int pageIndex)
{
    if (m_pagelst.contains(ImagePageInfo_t(pageIndex))) {
        beginResetModel();
        m_pagelst.removeAll(ImagePageInfo_t(pageIndex));
        endResetModel();
    }
}

void SideBarImageViewModel::removeItemForAnno(deepin_reader::Annotation *annotation)
{
    int index = findItemForAnno(annotation);
    if (index >= 0) {
        beginResetModel();
        m_pagelst.removeAt(index);
        endResetModel();
    }
}

void SideBarImageViewModel::getModelIndexImageInfo(int modelIndex, ImagePageInfo_t &tImagePageInfo)
{
    if (modelIndex >= 0 && modelIndex < m_pagelst.size()) {
        tImagePageInfo = m_pagelst.at(modelIndex);
    }
}

int SideBarImageViewModel::findItemForAnno(deepin_reader::Annotation *annotation)
{
    int count = m_pagelst.size();
    for (int index = 0; index < count; index++) {
        if (annotation == m_pagelst.at(index).annotation) {
            return index;
        }
    }
    return -1;
}

void SideBarImageViewModel::handleRenderThumbnail(int index, QPixmap pixmap)
{
    pixmap.setDevicePixelRatio(dApp->devicePixelRatio());
    m_sheet->setThumbnail(index, pixmap);

    const QList<QModelIndex> &modelIndexlst = getModelIndexForPageIndex(index);
    for (const QModelIndex &modelIndex : modelIndexlst)
        emit dataChanged(modelIndex, modelIndex);
}
