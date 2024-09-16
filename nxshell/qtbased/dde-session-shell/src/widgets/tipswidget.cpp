// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipswidget.h"
#include <QEvent>
#include <QTimer>
DWIDGET_USE_NAMESPACE

static constexpr int BlurMaskAlpha = 70;
static constexpr int BlurRadius = 6;

TipsWidget::TipsWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_content(nullptr)
{
    setMaskColor(DBlurEffectWidget::LightColor);
    setMaskAlpha(BlurMaskAlpha);
    setBlurRectXRadius(BlurRadius);
    setBlurRectYRadius(BlurRadius);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);
}

void TipsWidget::setContent(QWidget *content)
{
    if (m_content)
        m_content->removeEventFilter(this);
    if (content) {
        content->installEventFilter(this);
        content->setParent(this);
        content->show();
        content->move(0, 0);
    }
    m_content = content;
    resizeFromContent();
}

void TipsWidget::resizeFromContent()
{
    if (m_content.isNull()) {
        resize(0, 0);
        return;
    }
    const QRect contentRect = m_content->rect();
    const QMargins margins{5, 5, 5, 5};
    resize(contentRect.marginsAdded(margins).size());
    m_content->move(margins.left(), margins.top());
}

void TipsWidget::show(int x, int y)
{
    m_lastPos = QPoint(x, y);
    move(x - width() / 2, y - height()); // Position passed in is the bottom center of tips widget
    DBlurEffectWidget::show();
}

bool TipsWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o != m_content || e->type() != QEvent::Resize)
        return false;

    if (isVisible()) {

        QTimer::singleShot(10, this, [ = ] {
            if (isVisible()) {
                resizeFromContent();
                show(m_lastPos.x(), m_lastPos.y());
            }
        });
    }

    return DBlurEffectWidget::eventFilter(o, e);
}
