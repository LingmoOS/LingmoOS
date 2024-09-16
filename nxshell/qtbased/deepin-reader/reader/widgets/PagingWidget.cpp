// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PagingWidget.h"
#include "DocSheet.h"
#include "TMFunctionThread.h"

#include <QValidator>

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DFontSizeManager>

#define LineEditSpacing  24

PagingWidget::PagingWidget(DocSheet *sheet, DWidget *parent)
    : BaseWidget(parent), m_sheet(sheet)
{
    initWidget();

    slotUpdateTheme();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &PagingWidget::slotUpdateTheme);

    m_tmFuncThread = new TMFunctionThread(this);
    connect(m_tmFuncThread, &TMFunctionThread::finished, this, &PagingWidget::onFuncThreadFinished);
}

PagingWidget::~PagingWidget()
{

}

/**
 * @brief PagingWidget::initWidget
 * 初始化界面
 */
void PagingWidget::initWidget()
{
    m_pTotalPagesLab = new DLabel(this);
    m_pTotalPagesLab->setAccessibleName("Label_TotalPage");

    Dtk::Widget::DFontSizeManager::instance()->bind(m_pTotalPagesLab, Dtk::Widget::DFontSizeManager::T6, true);

    m_pTotalPagesLab->setForegroundRole(DPalette::Text);

    m_pJumpPageLineEdit = new DLineEdit(this);
    m_pJumpPageLineEdit->setAccessibleName("Page");
    m_pJumpPageLineEdit->setObjectName("Edit_Page_P");
    m_pJumpPageLineEdit->lineEdit()->setObjectName("Edit_Page");
    m_pJumpPageLineEdit->lineEdit()->setAccessibleName("pageEdit");
    m_pJumpPageLineEdit->setFixedWidth(60);
    m_pJumpPageLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_pJumpPageLineEdit->setClearButtonEnabled(false);
    connect(m_pJumpPageLineEdit, SIGNAL(returnPressed()), SLOT(SlotJumpPageLineEditReturnPressed()));
    connect(m_pJumpPageLineEdit, SIGNAL(editingFinished()), SLOT(onEditFinished()));

    Dtk::Widget::DFontSizeManager::instance()->bind(m_pJumpPageLineEdit->lineEdit(), Dtk::Widget::DFontSizeManager::T6, true);

    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
                this, &PagingWidget::onEditFinished);

    m_pJumpPageLineEdit->setForegroundRole(DPalette::Text);

    m_pPrePageBtn = new DIconButton(DStyle::SP_ArrowLeft);
    m_pPrePageBtn->setAccessibleName("Button_ThumbnailPre");
    m_pPrePageBtn->setObjectName("thumbnailPreBtn");
    m_pPrePageBtn->setMinimumSize(QSize(24, 24));
    connect(m_pPrePageBtn, SIGNAL(clicked()), SLOT(slotPrePageBtnClicked()));

    m_pNextPageBtn = new DIconButton(DStyle::SP_ArrowRight);
    m_pPrePageBtn->setMinimumSize(QSize(24, 24));
    m_pNextPageBtn->setAccessibleName("Button_ThumbnailNext");
    m_pNextPageBtn->setObjectName("thumbnailNextBtn");
    connect(m_pNextPageBtn, SIGNAL(clicked()), SLOT(slotNextPageBtnClicked()));

    m_pCurrentPageLab = new DLabel(this);
    m_pCurrentPageLab->setAccessibleName("CurrentPage");
    Dtk::Widget::DFontSizeManager::instance()->bind(m_pCurrentPageLab, Dtk::Widget::DFontSizeManager::T6, true);

    m_pCurrentPageLab->setForegroundRole(DPalette::Text);
    m_pCurrentPageLab->setVisible(false);

    auto hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(10, 6, 10, 6);
    hLayout->addWidget(m_pJumpPageLineEdit);
    hLayout->addSpacing(5);

    hLayout->addWidget(m_pCurrentPageLab);
    hLayout->addWidget(m_pTotalPagesLab);
    hLayout->addStretch(1);
    hLayout->addWidget(m_pPrePageBtn);
    hLayout->addWidget(m_pNextPageBtn);

    this->setLayout(hLayout);
}

void PagingWidget::slotUpdateTheme()
{
    if (m_pTotalPagesLab) {
        m_pTotalPagesLab->setForegroundRole(DPalette::Text);
    }

    if (m_pCurrentPageLab) {
        m_pCurrentPageLab->setForegroundRole(DPalette::Text);
    }
}

void PagingWidget::setBtnState(const int &currntPage, const int &totalPage)
{
    if (currntPage == 1) {// 第一页
        m_pPrePageBtn->setEnabled(false);
        if (totalPage == 1) {// 也是最后一页
            m_pNextPageBtn->setEnabled(false);
        } else {
            m_pNextPageBtn->setEnabled(true);
        }
    } else if (currntPage == totalPage) {// 最后一页
        m_pPrePageBtn->setEnabled(true);
        m_pNextPageBtn->setEnabled(false);
    } else {// 中间页
        m_pPrePageBtn->setEnabled(true);
        m_pNextPageBtn->setEnabled(true);
    }
}

void PagingWidget::resizeEvent(QResizeEvent *event)
{
    if (m_curIndex >= 0)
        setIndex(m_curIndex);

    this->QWidget::resizeEvent(event);
}

void PagingWidget::setIndex(int index)
{
    if (nullptr == m_sheet)
        return;

    m_curIndex = index;
    int totalPage = m_sheet->pageCount();
    int inputData = index;
    int currntPage = inputData + 1;     //  + 1 是为了 数字 从1 开始显示
    setBtnState(currntPage, totalPage);

    int iControlMaxWidth = (this->width() - m_pJumpPageLineEdit->width() - m_pPrePageBtn->width() - m_pNextPageBtn->width() - (10 + 6) * 2) / 2;
    m_pTotalPagesLab->setText(m_pCurrentPageLab->fontMetrics().elidedText(QString("/ %1").arg(totalPage), Qt::ElideRight, iControlMaxWidth));
    m_pTotalPagesLab->setToolTip(QString("%1").arg(totalPage));

    if (m_bHasLabel) {
        m_pCurrentPageLab->setText(m_pCurrentPageLab->fontMetrics().elidedText(QString::number(currntPage), Qt::ElideRight, iControlMaxWidth));
        m_pCurrentPageLab->setToolTip(QString::number(currntPage));

        QString sPage = m_sheet->getPageLabelByIndex(inputData);
        m_pJumpPageLineEdit->setText(m_pJumpPageLineEdit->fontMetrics().elidedText(sPage, Qt::ElideRight, m_pJumpPageLineEdit->width() - LineEditSpacing));
    } else {
        m_pJumpPageLineEdit->setText(m_pJumpPageLineEdit->fontMetrics().elidedText(QString::number(currntPage), Qt::ElideRight, m_pJumpPageLineEdit->width() - LineEditSpacing));
    }
}

void PagingWidget::handleOpenSuccess()
{
    if (nullptr == m_sheet)
        return;

    m_tmFuncThread->func = [ this ]()->bool{
        //当快递过快时崩溃
        return m_sheet->haslabel();
    };
    m_tmFuncThread->start();

    int currentIndex = m_sheet->currentIndex();
    setIndex(currentIndex);
}

void PagingWidget::SlotJumpPageLineEditReturnPressed()
{
    if (m_bHasLabel) {
        pageNumberJump();
    } else {
        normalChangePage();
    }
}

void PagingWidget::onEditFinished()
{
    if (m_curIndex >= 0)
        setIndex(m_curIndex);
}

void PagingWidget::normalChangePage()
{
    QString sText = m_pJumpPageLineEdit->text();
    int iPage = sText.toInt();
    if (iPage <= 0 || iPage > m_sheet->pageCount()) {
        m_sheet->showTips(tr("Invalid page number"), 1);
    } else {
        m_sheet->jumpToIndex(iPage - 1);
    }
}

void PagingWidget::pageNumberJump()
{
    int nPageSum = m_sheet->pageCount();
    QString sText = m_pJumpPageLineEdit->text();
    int iPage = m_sheet->getIndexByPageLable(sText);

    if (iPage > -1 && iPage < nPageSum) {   //  输入的页码 必须在 0-最大值 之间, 才可以
        m_sheet->jumpToIndex(iPage);
    } else {
        normalChangePage();
    }
}

void PagingWidget::slotPrePageBtnClicked()
{
    m_sheet->jumpToPrevPage();
}

void PagingWidget::slotNextPageBtnClicked()
{
    m_sheet->jumpToNextPage();
}

void PagingWidget::setTabOrderWidget(QList<QWidget *> &tabWidgetlst)
{
    tabWidgetlst << m_pJumpPageLineEdit;
    tabWidgetlst << m_pPrePageBtn;
    tabWidgetlst << m_pNextPageBtn;
}

void PagingWidget::onFuncThreadFinished()
{
    m_bHasLabel = m_tmFuncThread->result.toBool();
    m_pCurrentPageLab->setVisible(m_bHasLabel);
    int currentIndex = m_sheet->currentIndex();
    setIndex(currentIndex);
}
