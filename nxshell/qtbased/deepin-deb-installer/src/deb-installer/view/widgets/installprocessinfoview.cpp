// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "installprocessinfoview.h"
#include "droundbgframe.h"
#include "utils/utils.h"
#include "ShowInstallInfoTextEdit.h"

#include <QVBoxLayout>
#include <QScroller>

#include <DApplicationHelper>

InstallProcessInfoView::InstallProcessInfoView(int w, int h, QWidget *parent)
    : QWidget(parent)
    , m_editor(new ShowInstallInfoTextEdit(this))  // 修改为自写控件
{
    initUI(w, h);

    // 数据更新后，直接跳转到最后一行
    connect(m_editor, &QTextEdit::textChanged, this, &InstallProcessInfoView::slotMoveCursorToEnd);
    // 设置TextEdit和InfoView为无焦点
    this->setFocusPolicy(Qt::NoFocus);
    m_editor->setFocusPolicy(Qt::NoFocus);
}

void InstallProcessInfoView::slotMoveCursorToEnd()
{
    m_editor->moveCursor(QTextCursor::End);
}
void InstallProcessInfoView::initUI(int w, int h)
{
    // 设置控件背景色
    DRoundBgFrame *bgFrame = new DRoundBgFrame(this);
    bgFrame->setFixedSize(w, h);

    // edit的布局
    QVBoxLayout *editLayout = new QVBoxLayout(this);
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(5, 1, 0, 5);  // 设置上下左右边距
    bgFrame->setLayout(editLayout);

    editLayout->addWidget(m_editor);

    // 设置控件的字体类型和字体大小
    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, 11, QFont::Light);

    // 设置字体颜色
    DebApplicationHelper *pdebhelp = DebApplicationHelper::instance();
    if (nullptr == pdebhelp) {
        return;
    }

    DPalette pa = m_editor->palette();
    m_colorType = DPalette::TextTips;
    pa.setColor(DPalette::Text, pa.color(m_colorType));
    pa.setColor(DPalette::HighlightedText, QColor(Qt::white));
    pa.setColor(DPalette::Highlight, DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    m_editor->setPalette(pa);

    // 设置只读，不允许对其进行修改
    m_editor->setReadOnly(true);
    m_editor->setFrameShape(QFrame::NoFrame);                       // 设置frame类型为noframe
    m_editor->viewport()->setBackgroundRole(QPalette::Background);  // 设置内容的背景色
    m_editor->viewport()->setAutoFillBackground(false);

    QTextCursor textCursor = m_editor->textCursor();  // 获取游标
    QTextBlockFormat textBlockFormat;
    // 设置行高
    textBlockFormat.setLineHeight(20, QTextBlockFormat::FixedHeight);
    // 设置行间距
    textBlockFormat.setBottomMargin(1);
    textCursor.setBlockFormat(textBlockFormat);
    m_editor->setTextCursor(textCursor);
}

void InstallProcessInfoView::setTextFontSize(int fontSize, int fontWeight)
{
    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, fontSize, fontWeight);
}

void InstallProcessInfoView::setTextColor(DPalette::ColorType ct)
{
    m_colorType = ct;  // 保存传入的颜色类型
    DPalette pa = m_editor->palette();
    pa.setColor(DPalette::Text, pa.color(m_colorType));  // editor设置传入的颜色类型
    pa.setColor(DPalette::HighlightedText, QColor(Qt::white));
    pa.setColor(DPalette::Highlight, DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    m_editor->setPalette(pa);
}

void InstallProcessInfoView::appendText(QString text)
{
    m_editor->append(text);
}

InstallProcessInfoView::~InstallProcessInfoView() {}

void InstallProcessInfoView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    DPalette pa = m_editor->palette();

    // 获取当前的主题
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    if (themeType == DGuiApplicationHelper::LightType) {  // 当前是浅色主题
        pa.setColor(DPalette::Text, QColor(96, 157, 200));
    } else if (themeType == DGuiApplicationHelper::DarkType) {  // 当前是深色主题
        pa.setColor(DPalette::Text, QColor(109, 124, 136));
    } else {  // 默认使用浅色主题
        pa.setColor(DPalette::Text, QColor(96, 157, 200));
    }

    pa.setColor(DPalette::HighlightedText, QColor(Qt::white));
    pa.setColor(DPalette::Highlight, DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    m_editor->setPalette(pa);
}

/**
 * @brief 清除当前展示框中的内容
 *
 */
void InstallProcessInfoView::clearText()
{
    m_editor->clear();
}

void InstallProcessInfoView::setTextCursor(QTextCursor::MoveOperation operation)
{
    QTextCursor textCursor = m_editor->textCursor();
    textCursor.movePosition(operation, QTextCursor::MoveAnchor);
    m_editor->setTextCursor(textCursor);
}
