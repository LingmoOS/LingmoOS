#include "modelmanagebutton.h"
#include "llmwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyle>

#include <DPalette>
#include <DGuiApplicationHelper>
#include <DStyle>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

ModelManageButton::ModelManageButton(QWidget * parent)
    : DPushButton(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setFlat(true);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, &ModelManageButton::updateRectSize);
}

ModelManageButton::ModelManageButton(const QString &text, QWidget * parent)
    : DPushButton(text, parent)
{
    setText(text);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setFlat(true);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, &ModelManageButton::updateRectSize);
}

void ModelManageButton::paintEvent(QPaintEvent* e)
{
    QRectF rect = this->rect();
    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing, true);
    DPalette parentPb = DGuiApplicationHelper::instance()->applicationPalette();
    QColor textColor = parentPb.color(DPalette::Normal, DPalette::Highlight);

    bool isDarkType = false;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        isDarkType = true;

    if (isDarkType)
        textColor.setAlphaF(0.7);

    if (isEnabled() && m_isHover) {
        if (isDarkType)
            textColor.setAlphaF(1);
        else
            textColor.setAlphaF(0.7);
    }

    if (!this->isEnabled() || !isActiveWindow()) {
        textColor.setAlphaF(0.4);
    }

    rect.setWidth(rect.width() - 20);
    pa.setPen(textColor);
    QTextOption textOption = Qt::AlignVCenter | Qt::AlignLeft;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setAlignment(Qt::AlignLeft);
    pa.drawText(rect, text(), textOption);

    rect.setX(rect.width() + 4);
    rect.setY((this->height() - 6) / 2);
    rect.setSize(QSizeF(6, 6));
    pa.setPen(QPen(textColor, 1.2));
    if (property("modelStatus").toInt() == InstallAndUpdate)
        DDrawUtils::drawArrowDown(&pa, rect);
}

void ModelManageButton::enterEvent(QEvent *event)
{
    m_isHover = true;
    update();
    return DPushButton::enterEvent(event);
}

void ModelManageButton::leaveEvent(QEvent *event)
{
    m_isHover = false;
    update();
    return DPushButton::leaveEvent(event);
}

void ModelManageButton::showEvent(QShowEvent *event)
{
    m_isHover = false;
    update();
    updateRectSize();
    return DPushButton::showEvent(event);
}

void ModelManageButton::updateRectSize()
{
    QFont font = this->font();
    QFontMetrics fm(font);
    int textWidth = fm.width(text());
    this->setFixedWidth(textWidth + 20);
    this->setFixedHeight(fm.height());
    adjustSize();
}
