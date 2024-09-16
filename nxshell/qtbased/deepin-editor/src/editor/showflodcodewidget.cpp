// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "showflodcodewidget.h"
#include <QFileInfo>
#include <QDebug>
#include <QPalette>
#include <QVBoxLayout>
#include <DWindowManagerHelper>
#include <QGraphicsDropShadowEffect>
#include <QXmlStreamReader>

namespace KSyntaxHighlighting {
    class SyntaxHighlighter;
}

ShowFlodCodeWidget::ShowFlodCodeWidget(DWidget *parent)
    : DFrame(parent)
{
    //setFrameRounded(false);
    QGraphicsDropShadowEffect *effert = new QGraphicsDropShadowEffect(this);
    effert->setOffset(0,6);
    effert->setBlurRadius(20);
    QColor color(0,0,0);
    color.setAlphaF(0.2);
    effert->setColor(color);
    this->setGraphicsEffect(effert);
    QVBoxLayout *pSubLayout = new QVBoxLayout();
    m_pContentEdit = new DPlainTextEdit(this);
    m_pContentEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    m_pContentEdit->setFrameStyle(0);
    m_pContentEdit->setReadOnly(true);
    m_pContentEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pContentEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pSubLayout->addWidget(m_pContentEdit);
    this->setLayout(pSubLayout);
    m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(m_pContentEdit->document());

    // 更新单独添加的高亮格式文件
    m_repository.addCustomSearchPath(KF5_HIGHLIGHT_PATH);
}

ShowFlodCodeWidget::~ShowFlodCodeWidget()
{
    m_highlighter->deleteLater();
    m_highlighter = nullptr;
}

void ShowFlodCodeWidget::clear()
{
    m_pContentEdit->document()->clear();
    m_nTextWidth = 0;
    adjustSize();
}

void ShowFlodCodeWidget::initHighLight(QString filepath, bool bIsLight)
{
    if (m_highlighter != nullptr) {
        if (!bIsLight) {
            m_highlighter->setTheme(m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme));
        } else {
            m_highlighter->setTheme(m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));
        }
    }
   // m_highlighter->rehighlight();
    const auto def = m_repository.definitionForFileName(QFileInfo(filepath).fileName());
    m_highlighter->setDefinition(def);
}

void ShowFlodCodeWidget::setStyle(bool bIsLineWrap)
{
    QPalette pa = palette();
    QColor color(25,25,25);

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
        color.setAlphaF(0.8);
        pa.setColor(QPalette::Base,color);
        m_pContentEdit->setPalette(pa);
        pa.setColor(QPalette::Base,QColor(25,25,25));
        setPalette(pa);
    } else {
        color = QColor(247,247,247);
        color.setAlphaF(0.6);
        pa.setColor(QPalette::Base,color);
        m_pContentEdit->setPalette(pa);
        pa.setColor(QPalette::Base,QColor(247,247,247));
        setPalette(pa);
    }

    if (bIsLineWrap) {
        m_pContentEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    } else {
        m_pContentEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
}

void ShowFlodCodeWidget::hideFirstBlock()
{
    m_pContentEdit->document()->firstBlock().setVisible(false);
    m_pContentEdit->document()->lastBlock().setVisible(false);
    int editHight = 0;

    for (auto block = m_pContentEdit->document()->firstBlock(); block.isValid(); block = block.next()) {

        if (block == m_pContentEdit->document()->lastBlock()) {
            break;
        }

        editHight += m_pContentEdit->document()->documentLayout()->blockBoundingRect(block).toRect().height();
    }

    m_pContentEdit->setFixedHeight(editHight + 10);
}

void ShowFlodCodeWidget::appendText(QString strText, int maxWidth)
{
    int textWidth = m_pContentEdit->fontMetrics().width(strText) + 10;

    if (m_nTextWidth < textWidth) {
        m_nTextWidth = textWidth;
        if (m_nTextWidth > maxWidth - 50) {
            m_nTextWidth = maxWidth - 50;
        }
    }

    m_pContentEdit->setFixedWidth(m_nTextWidth);

    if (m_pContentEdit->document()->isEmpty()) {
        m_pContentEdit->setPlainText(strText);
    } else {
        m_pContentEdit->appendPlainText(strText);
    }

    QTextCursor cursor = m_pContentEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_pContentEdit->setTextCursor(cursor);
}

