// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CatalogWidget.h"
#include "CatalogTreeView.h"
#include "DocSheet.h"

#include <QVBoxLayout>

#include <DFontSizeManager>

CatalogWidget::CatalogWidget(DocSheet *sheet, DWidget *parent)
    : BaseWidget(parent), m_sheet(sheet)
{
    initWidget();
}

CatalogWidget::~CatalogWidget()
{

}

void CatalogWidget::initWidget()
{
    QHBoxLayout *titleLayout = new QHBoxLayout;

    titleLayout->setSpacing(0);

    titleLayout->setContentsMargins(10, 0, 10, 0);

    titleLabel = new DLabel("", this);

    titleLabel->setAccessibleName("Label_title");

    titleLabel->setForegroundRole(DPalette::TextTips);

    titleLabel->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T8);

    titleLayout->addWidget(titleLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(titleLayout);

    m_pTree = new CatalogTreeView(m_sheet, this);
    m_pTree->setAccessibleName("View_CatalogTree");

    mainLayout->addWidget(m_pTree);
    this->setLayout(mainLayout);
}

void CatalogWidget::resizeEvent(QResizeEvent *event)
{
    if (m_strTheme != "" && titleLabel) {
        setTitleTheme();
    }

    BaseWidget::resizeEvent(event);
}

void CatalogWidget::setTitleTheme()
{
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);

    QFontMetrics fm(font);

    QString sTheme = fm.elidedText(m_strTheme, Qt::ElideMiddle, this->width() - 40, Qt::TextSingleLine);

    titleLabel->setText(sTheme);

}

void CatalogWidget::handleOpenSuccess()
{
    if (nullptr == m_sheet || bIshandOpenSuccess)
        return;

    bIshandOpenSuccess = true;
    deepin_reader::FileInfo fileInfo;
    m_sheet->docBasicInfo(fileInfo);
    m_strTheme = fileInfo.theme;
    if (m_strTheme != "") {
        setTitleTheme();
    }
    m_pTree->handleOpenSuccess();
}

void CatalogWidget::handlePage(int index)
{
    m_pTree->setIndex(index);
    m_pTree->setRightControl(true);
}

void CatalogWidget::nextPage()
{
    m_pTree->nextPage();
}

void CatalogWidget::prevPage()
{
    m_pTree->prevPage();
}

void CatalogWidget::pageDown()
{
    m_pTree->pageDownPage();
}

void CatalogWidget::pageUp()
{
    m_pTree->pageUpPage();
}
