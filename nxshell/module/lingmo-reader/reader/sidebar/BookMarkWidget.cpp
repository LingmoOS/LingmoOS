// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BookMarkWidget.h"
#include "DocSheet.h"
#include "SideBarImageListview.h"
#include "SideBarImageViewModel.h"
#include "BookMarkDelegate.h"
#include "SaveDialog.h"
#include "MsgHeader.h"

#include <DHorizontalLine>
#include <DPushButton>
#include <DGuiApplicationHelper>

#include <QHBoxLayout>
#include <QSet>

DGUI_USE_NAMESPACE

const int LEFTMINHEIGHT = 80;
BookMarkWidget::BookMarkWidget(DocSheet *sheet, DWidget *parent)
    : BaseWidget(parent), m_sheet(sheet)
{
    initWidget();
    onUpdateTheme();
}

BookMarkWidget::~BookMarkWidget()
{

}

void BookMarkWidget::initWidget()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &BookMarkWidget::onUpdateTheme);

    m_pImageListView = new SideBarImageListView(m_sheet, this);
    m_pImageListView->setAccessibleName("View_ImageList");
    m_pImageListView->setListType(E_SideBar::SIDE_BOOKMARK);
    BookMarkDelegate *imageDelegate = new BookMarkDelegate(m_pImageListView);
    m_pImageListView->setItemDelegate(imageDelegate);

    m_pAddBookMarkBtn = new DPushButton(this);
    m_pAddBookMarkBtn->setObjectName("BookmarkAddBtn");
    m_pAddBookMarkBtn->setAccessibleName("BookmarkAdd");
    m_pAddBookMarkBtn->setMinimumWidth(170);
    m_pAddBookMarkBtn->setText(tr("Add bookmark"));
    DFontSizeManager::instance()->bind(m_pAddBookMarkBtn, DFontSizeManager::T6);
    connect(m_pAddBookMarkBtn, SIGNAL(clicked()), this, SLOT(onAddBookMarkClicked()));

    QHBoxLayout *pHBoxLayout = new QHBoxLayout;
    pHBoxLayout->setContentsMargins(10, 6, 10, 6);
    pHBoxLayout->addWidget(m_pAddBookMarkBtn);

    QVBoxLayout *pVBoxLayout = new QVBoxLayout;
    pVBoxLayout->setContentsMargins(0, 10, 0, 0);
    pVBoxLayout->setSpacing(0);

    pVBoxLayout->addWidget(m_pImageListView);
    DHorizontalLine *line = new DHorizontalLine(this);
    line->setAccessibleName("BookMarkLine");
    pVBoxLayout->addWidget(line);
    pVBoxLayout->addItem(pHBoxLayout);
    this->setLayout(pVBoxLayout);

    connect(m_pImageListView, SIGNAL(sigListMenuClick(const int &)), SLOT(onListMenuClick(const int &)));
    m_pImageListView->setItemSize(QSize(LEFTMINWIDTH, LEFTMINHEIGHT));
}


void BookMarkWidget::prevPage()
{
    if (m_sheet.isNull())
        return;

    int curPage = m_pImageListView->currentIndex().row() - 1;
    if (curPage < 0)
        return;

    m_sheet->jumpToIndex(m_pImageListView->getPageIndexForModelIndex(curPage));
}

void BookMarkWidget::pageUp()
{
    if (m_sheet.isNull())
        return;

    const QModelIndex &pageIndex = m_pImageListView->pageUpIndex();
    if (!pageIndex.isValid())
        return;

    m_sheet->jumpToIndex(m_pImageListView->getPageIndexForModelIndex(pageIndex.row()));
}

void BookMarkWidget::nextPage()
{
    if (m_sheet.isNull())
        return;

    int curPage = m_pImageListView->currentIndex().row() + 1;
    m_sheet->jumpToIndex(m_pImageListView->getPageIndexForModelIndex(curPage));
}

void BookMarkWidget::pageDown()
{
    if (m_sheet.isNull())
        return;

    const QModelIndex &pageIndex = m_pImageListView->pageDownIndex();
    if (!pageIndex.isValid())
        return;

    m_sheet->jumpToIndex(m_pImageListView->getPageIndexForModelIndex(pageIndex.row()));
}

void BookMarkWidget::handleOpenSuccess()
{
    if (bIshandOpenSuccess)
        return;
    bIshandOpenSuccess = true;
    const QSet<int> &pageList = m_sheet->getBookMarkList();
    if (pageList.contains(m_sheet->currentIndex()))
        m_pAddBookMarkBtn->setEnabled(false);
    m_pImageListView->handleOpenSuccess();
}

void BookMarkWidget::handlePage(int index)
{
    bool result = m_pImageListView->scrollToIndex(index);
    m_pAddBookMarkBtn->setDisabled(result);
}

void BookMarkWidget::handleBookMark(int index, int state)
{
    if (state) {
        int nCurIndex = m_sheet->currentIndex();
        if (nCurIndex == index) m_pAddBookMarkBtn->setEnabled(false);
        m_pImageListView->getImageModel()->insertPageIndex(index);
    } else {
        int nCurIndex = m_sheet->currentIndex();
        if (nCurIndex == index) m_pAddBookMarkBtn->setEnabled(true);
        m_pImageListView->getImageModel()->removePageIndex(index);
    }
    m_pImageListView->scrollToIndex(m_sheet->currentIndex(), true);
}

void BookMarkWidget::deleteItemByKey()
{
    int curIndex = m_pImageListView->getPageIndexForModelIndex(m_pImageListView->currentIndex().row());
    if (curIndex >= 0)
        m_sheet->setBookMark(curIndex, false);
}

void BookMarkWidget::deleteAllItem()
{
    QList<int> bookmarks;
    int itemsize = m_pImageListView->model()->rowCount();
    for (int i = 0; i < itemsize; i++) {
        int curIndex = m_pImageListView->getPageIndexForModelIndex(i);
        if (curIndex >= 0) {
            bookmarks << curIndex;
        }
    }
    m_sheet->setBookMarks(bookmarks, false);
}

void BookMarkWidget::onAddBookMarkClicked()
{
    if (m_sheet.isNull()) return;
    int nPage = m_sheet->currentIndex();
    m_sheet->setBookMark(nPage, true);
}

void BookMarkWidget::adaptWindowSize(const double &scale)
{
    m_pImageListView->setProperty("adaptScale", scale);
    m_pImageListView->setItemSize(QSize(static_cast<int>(LEFTMINWIDTH * scale), LEFTMINHEIGHT));
    m_pImageListView->reset();
    m_pImageListView->scrollToIndex(m_sheet->currentIndex(), false);
}

void BookMarkWidget::showMenu()
{
    if (m_pImageListView && m_pImageListView->count()) {
        m_pImageListView->showMenu();
    }
}

void BookMarkWidget::onUpdateTheme()
{
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
    setPalette(plt);

    m_pAddBookMarkBtn->setPalette(plt);
}

void BookMarkWidget::onListMenuClick(const int &iType)
{
    if (iType == E_BOOKMARK_DELETE) {
        deleteItemByKey();
    } else if (iType == E_BOOKMARK_DELETE_ALL) {
        int result = SaveDialog::showTipDialog(tr("Are you sure you want to delete all bookmarks?") ,this);
        if (result == 1) {
            deleteAllItem();
        }
    }
}

void BookMarkWidget::setTabOrderWidget(QList<QWidget *> &tabWidgetlst)
{
    tabWidgetlst << m_pAddBookMarkBtn;
}
