// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SideBarImageListview.h"
#include "DocSheet.h"
#include "SideBarImageViewModel.h"
#include "Application.h"
#include "MsgHeader.h"

#include <QScroller>
#include <QScrollBar>
#include <QSet>
#include <QMouseEvent>

SideBarImageListView::SideBarImageListView(DocSheet *sheet, QWidget *parent)
    : DListView(parent)
    , m_docSheet(sheet)
{
    initControl();
    setAutoScroll(false);
    setProperty("adaptScale", 1.0);
    setSpacing(4);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeMode(QListView::Fixed);
    setViewMode(QListView::ListMode);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScroller::grabGesture(this, QScroller::TouchGesture);//滑动

    connect(verticalScrollBar(), &QScrollBar::sliderPressed, this, &SideBarImageListView::onRemoveThumbnailListSlideGesture);

    connect(verticalScrollBar(), &QScrollBar::sliderReleased, this, &SideBarImageListView::onSetThumbnailListSlideGesture);
}

void SideBarImageListView::initControl()
{
    m_pBookMarkMenu = nullptr;
    m_pNoteMenu = nullptr;
    m_listType = E_SideBar::SIDE_THUMBNIL;
    m_imageModel = new SideBarImageViewModel(m_docSheet, this);
    this->setModel(m_imageModel);
}

SideBarImageViewModel *SideBarImageListView::getImageModel()
{
    return m_imageModel;
}

void SideBarImageListView::showMenu()
{
    if (!this->currentIndex().isValid())
        return;

    const QRect &visualRect = this->visualRect(this->currentIndex());
    QPoint point = mapToGlobal(QPoint(this->width() / 2 - 4, visualRect.y() + visualRect.height() / 2 - 4));
    if (m_listType == E_SideBar::SIDE_NOTE) {
        showNoteMenu(point);
    } else if (m_listType == E_SideBar::SIDE_BOOKMARK) {
        showBookMarkMenu(point);
    }
}

void SideBarImageListView::setListType(int type)
{
    m_listType = type;
}

void SideBarImageListView::handleOpenSuccess()
{
    if (m_listType == E_SideBar::SIDE_THUMBNIL) {
        const QSet<int> &pageList = m_docSheet->getBookMarkList();
        for (int pageIndex : pageList) {
            m_imageModel->setBookMarkVisible(pageIndex, true, false);
        }

        QList<ImagePageInfo_t> pageSrclst;
        int pagesNum = m_docSheet->pageCount();
        for (int index = 0; index < pagesNum; index++)
            pageSrclst << ImagePageInfo_t(index);
        m_imageModel->initModelLst(pageSrclst);
    } else if (m_listType == E_SideBar::SIDE_BOOKMARK) {
        const QSet<int> &pageList = m_docSheet->getBookMarkList();
        QList<ImagePageInfo_t> pageSrclst;
        for (int pageIndex : pageList)
            pageSrclst << ImagePageInfo_t(pageIndex);
        m_imageModel->initModelLst(pageSrclst, true);
    } else if (m_listType == E_SideBar::SIDE_NOTE) {
        const QList< deepin_reader::Annotation * > &annotationlst = m_docSheet->annotations();
        QList<ImagePageInfo_t> pageSrclst;
        int pagesNum = annotationlst.size();
        for (int index = 0; index < pagesNum; index++) {
            deepin_reader::Annotation *annotion = annotationlst.at(index);
            if (!annotion->contents().isEmpty()) {
                int pageIndex = static_cast<int>(annotion->page) - 1;
                ImagePageInfo_t tImagePageInfo;
                tImagePageInfo.pageIndex = pageIndex;
                tImagePageInfo.strcontents = annotion->contents();
                tImagePageInfo.annotation = annotion;
                pageSrclst << tImagePageInfo;
            }
        }

        m_imageModel->initModelLst(pageSrclst);
    }
}

void SideBarImageListView::onItemClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        m_docSheet->jumpToIndex(m_imageModel->getPageIndexForModelIndex(index.row()));
        emit sigListItemClicked(index.row());
    }
}

void SideBarImageListView::onSetThumbnailListSlideGesture()
{
    QScroller::grabGesture(this, QScroller::TouchGesture);//缩略图列表滑动
}

void SideBarImageListView::onRemoveThumbnailListSlideGesture()
{
    QScroller::grabGesture(this, QScroller::MiddleMouseButtonGesture);//缩略图列表不滑动
}

bool SideBarImageListView::scrollToIndex(int index, bool scrollTo)
{
    const QList<QModelIndex> &indexlst = m_imageModel->getModelIndexForPageIndex(index);
    if (indexlst.size() > 0) {
        const QModelIndex &index = indexlst.first();
        if (scrollTo)
            this->scrollTo(index);
        this->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        this->setCurrentIndex(index);
        return true;
    } else {
        this->setCurrentIndex(QModelIndex());
        this->clearSelection();
        return false;
    }
}

void SideBarImageListView::scrollToModelInexPage(const QModelIndex &index, bool scrollto)
{
    if (index.isValid()) {
        this->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        this->setCurrentIndex(index);
        if (scrollto) this->scrollTo(index);
    }
}

void SideBarImageListView::mousePressEvent(QMouseEvent *event)
{
    DListView::mousePressEvent(event);
    onItemClicked(this->indexAt(event->pos()));
    //Menu
    if (event->button() == Qt::RightButton) {
        const QModelIndex &modelIndex = this->indexAt(event->pos());
        if (modelIndex.isValid()) {
            if (m_listType == E_SideBar::SIDE_NOTE) {
                showNoteMenu(QCursor::pos());
            } else if (m_listType == E_SideBar::SIDE_BOOKMARK) {
                showBookMarkMenu(QCursor::pos());
            }
        }
    }
}

void SideBarImageListView::showNoteMenu(const QPoint &point)
{
    if (m_pNoteMenu == nullptr) {
        m_pNoteMenu = new DMenu(this);
        m_pNoteMenu->setAccessibleName("Menu_Note");

        QAction *copyAction = m_pNoteMenu->addAction(tr("Copy"));
        connect(copyAction, &QAction::triggered, [this]() {
            emit sigListMenuClick(E_NOTE_COPY);
        });

        QAction *delAction = m_pNoteMenu->addAction(tr("Remove annotation"));
        connect(delAction, &QAction::triggered, [this]() {
            emit sigListMenuClick(E_NOTE_DELETE);
        });

        QAction *delAllAction = m_pNoteMenu->addAction(tr("Remove all"));
        connect(delAllAction, &QAction::triggered, [this]() {
            emit sigListMenuClick(E_NOTE_DELETE_ALL);
        });
    }
    m_pNoteMenu->exec(point);
}

void SideBarImageListView::showBookMarkMenu(const QPoint &point)
{
    if (m_pBookMarkMenu == nullptr) {
        m_pBookMarkMenu = new DMenu(this);
        m_pBookMarkMenu->setAccessibleName("Menu_BookMark");

        QAction *dltBookMarkAction = m_pBookMarkMenu->addAction(tr("Remove bookmark"));
        connect(dltBookMarkAction, &QAction::triggered, [this]() {
            emit sigListMenuClick(E_BOOKMARK_DELETE);
        });

        QAction *dltAllBookMarkAction = m_pBookMarkMenu->addAction(tr("Remove all"));
        connect(dltAllBookMarkAction, &QAction::triggered, [this]() {
            emit sigListMenuClick(E_BOOKMARK_DELETE_ALL);
        });
    }
    m_pBookMarkMenu->exec(point);
}

int  SideBarImageListView::getModelIndexForPageIndex(int pageIndex)
{
    const QList<QModelIndex> &indexlst = m_imageModel->getModelIndexForPageIndex(pageIndex);
    if (indexlst.size() > 0) {
        return indexlst.first().row();
    }
    return -1;
}

int  SideBarImageListView::getPageIndexForModelIndex(int row)
{
    return m_imageModel->getPageIndexForModelIndex(row);
}

QModelIndex SideBarImageListView::pageUpIndex()
{
    return DListView::moveCursor(QAbstractItemView::MovePageUp, Qt::NoModifier);
}

QModelIndex SideBarImageListView::pageDownIndex()
{
    return DListView::moveCursor(QAbstractItemView::MovePageDown, Qt::NoModifier);
}
