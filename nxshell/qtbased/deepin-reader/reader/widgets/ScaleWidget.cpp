// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ScaleWidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <DApplication>
#include <DFontSizeManager>

#define LineEditSpacing 24
#define NormalModeArrowBtnSize 32   //普通模式按钮大小
#define CompactModeArrowBtnSize 20  //紧凑模式按钮大小

ScaleWidget::ScaleWidget(DWidget *parent)
    : DWidget(parent)
{
    initWidget();
}

ScaleWidget::~ScaleWidget()
{

}

void ScaleWidget::initWidget()
{
    QHBoxLayout *m_layout = new QHBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    setLayout(m_layout);

    m_lineEdit = new DLineEdit(this);
    m_lineEdit->setObjectName("scaleEdit_P");
    m_lineEdit->lineEdit()->setObjectName("scaleEdit");
    Dtk::Widget::DFontSizeManager::instance()->bind(m_lineEdit->lineEdit(), Dtk::Widget::DFontSizeManager::T6, true);

    connect(dynamic_cast<QGuiApplication *>(DApplication::instance()), &DApplication::fontChanged,
            this, &ScaleWidget::onEditFinished);

    m_lineEdit->setFixedSize(120, 36);

    m_arrowBtn = new DIconButton(QStyle::SP_ArrowDown, m_lineEdit);
    m_arrowBtn->setObjectName("editArrowBtn");
    //添加显示比例值默认值
    m_arrowBtn->setFixedSize(NormalModeArrowBtnSize, NormalModeArrowBtnSize);
    m_arrowBtn->move(m_lineEdit->width() - m_arrowBtn->width() - 2, m_lineEdit->height() / 2 - m_arrowBtn->height() / 2);
#ifdef DTKWIDGET_CLASS_DSizeMode
    qInfo() << "Compact mode support!!!";
    onSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
    connect(DGuiApplicationHelper::instance(), SIGNAL(sizeModeChanged(DGuiApplicationHelper::SizeMode)), this, SLOT(onSizeModeChanged(DGuiApplicationHelper::SizeMode)));
#else
    qInfo() << "Compact mode is not supported!!!";
#endif
    m_lineEdit->lineEdit()->setTextMargins(0, 0, m_arrowBtn->width(), 0);
    m_lineEdit->setClearButtonEnabled(false);

    connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(onReturnPressed()));
    connect(m_lineEdit, SIGNAL(editingFinished()), SLOT(onEditFinished()));
    connect(m_arrowBtn, SIGNAL(clicked()), SLOT(onArrowBtnlicked()));

    DIconButton *pPreBtn = new DIconButton(DStyle::SP_DecreaseElement, this);
    pPreBtn->setObjectName("SP_DecreaseElement");
    DStyle::setFrameRadius(pPreBtn, 12);
    pPreBtn->setFixedSize(24, 24);
    connect(pPreBtn, SIGNAL(clicked()), SLOT(onPrevScale()));

    DIconButton *pNextBtn = new DIconButton(this/*DStyle::SP_IncreaseElement*/);
    pNextBtn->setIcon(QIcon::fromTheme("dr_button_add"));
    pNextBtn->setObjectName("SP_IncreaseElement");
    DStyle::setFrameRadius(pNextBtn, 12);
    pNextBtn->setFixedSize(24, 24);
    connect(pNextBtn, SIGNAL(clicked()), SLOT(onNextScale()));

    m_layout->addWidget(pPreBtn);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(pNextBtn);
}

void ScaleWidget::onPrevScale()
{
    if (m_sheet.isNull())
        return;

    m_sheet->zoomout();
}

void ScaleWidget::onNextScale()
{
    if (m_sheet.isNull())
        return;

    m_sheet->zoomin();
}

void ScaleWidget::onReturnPressed()
{
    if (m_sheet.isNull())
        return;

    qreal value = m_lineEdit->text().replace("%", "").toDouble() / 100.00;
    value = qBound(0.1, value, m_sheet->maxScaleFactor());
    m_sheet->setScaleFactor(value);
}

void ScaleWidget::onArrowBtnlicked()
{
    m_lineEdit->lineEdit()->setFocus(Qt::MouseFocusReason);

    ScaleMenu scaleMenu;
    QPoint point = m_lineEdit->mapToGlobal(QPoint(0, m_lineEdit->height() + 2));
    scaleMenu.readCurDocParam(m_sheet.data());
    scaleMenu.exec(point);
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void ScaleWidget::onSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        qInfo() << "Size Mode Changed! Current SizeMode is CompactMode";
        m_arrowBtn->setFixedSize(CompactModeArrowBtnSize, CompactModeArrowBtnSize);
    } else {
        qInfo() << "Size Mode Changed! Current SizeMode is " << sizeMode;
        m_arrowBtn->setFixedSize(NormalModeArrowBtnSize, NormalModeArrowBtnSize);
    }
    m_arrowBtn->move(m_lineEdit->width() - m_arrowBtn->width() - 2, m_lineEdit->height() / 2 - m_arrowBtn->height() / 2);
}
#endif

void ScaleWidget::onEditFinished()
{
    if (nullptr == m_sheet)
        return;

    QString text = QString::number(QString::number(m_sheet->operation().scaleFactor * 100, 'f', 2).toDouble()) + "%";
    m_lineEdit->setText(m_lineEdit->fontMetrics().elidedText(text, Qt::ElideRight, m_lineEdit->width() - m_arrowBtn->width() - 2 - LineEditSpacing));
}

void ScaleWidget::setSheet(DocSheet *sheet)
{
    m_sheet = sheet;
    if (nullptr == sheet)
        return;
    else if (!sheet->opened()) {
        clear();
        return;
    }

    m_lineEdit->clear();
    QString text = QString::number(QString::number(m_sheet->operation().scaleFactor * 100, 'f', 2).toDouble()) + "%";
    m_lineEdit->setText(m_lineEdit->fontMetrics().elidedText(text, Qt::ElideRight, m_lineEdit->width() - m_arrowBtn->width() - 2 - LineEditSpacing));
    m_lineEdit->lineEdit()->setCursorPosition(0);
}

void ScaleWidget::clear()
{
    m_lineEdit->clear();
}
