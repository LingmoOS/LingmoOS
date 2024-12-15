// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "miniframeswitchbtn.h"
#include "../windowedframe.h"
#include "../global_util/util.h"

#include <QHBoxLayout>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

DGUI_USE_NAMESPACE

/**
 * @brief MiniFrameSwitchBtn::MiniFrameSwitchBtn
 * 小窗口模式下 底部所有分类和返回按钮相关的控件
 * @param parent
 */
MiniFrameSwitchBtn::MiniFrameSwitchBtn(QWidget *parent)
    : QWidget(parent)
    , m_textLabel(new QLabel)
    , m_enterIcon(new QLabel)
    , m_allIconLabel(new QLabel)
{
    setFocusPolicy(Qt::StrongFocus);
    updateIcon();
    m_enterIcon->setAccessibleName("EnterIcon");
    m_allIconLabel->setAccessibleName("AllIcon");
    m_textLabel->setAccessibleName("Text");
    m_textLabel->setAccessibleDescription("This refers to the label with 'all categories'. Since this control will become 'return', it uses 'text' instead");
    DFontSizeManager::instance()->bind(m_textLabel, DFontSizeManager::T6);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameSwitchBtn::updateIcon);

    m_enterIcon->setFixedSize(16, 16);
    m_enterIcon->setVisible(false);

    m_allIconLabel->setFixedSize(24, 24);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    setObjectName("MiniFrameSwitchBtn");
    setFixedHeight(39);

    mainLayout->setContentsMargins(DLauncher::MINI_FRAME_LAYOUT_SPACE1, 0, 5, 0);
    mainLayout->addWidget(m_allIconLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addWidget(m_enterIcon);
}

void MiniFrameSwitchBtn::updateStatus(int status)
{
    if (status == WindowedFrame::All) {
        m_textLabel->setText(tr("All Categories"));
        m_enterIcon->setVisible(true);
    } else {
        m_textLabel->setText(tr("Back"));
        m_enterIcon->setVisible(false);
    }
}

void MiniFrameSwitchBtn::click()
{
    emit clicked();
}

void MiniFrameSwitchBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    if (hasFocus() || m_hover) {
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        p.fillPath(path, m_color);
    }
}

void MiniFrameSwitchBtn::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);

    m_hover = true;
    update();
}

void MiniFrameSwitchBtn::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);

    m_hover = false;
    update();
}

void MiniFrameSwitchBtn::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);

    emit clicked();
}

void MiniFrameSwitchBtn::updateIcon()
{
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        m_color.setRgb(255, 255, 255, 25);
        m_allIconLabel->setPixmap(renderSVG(":/widgets/images/all.svg", QSize(20, 20)));
        m_enterIcon->setPixmap(renderSVG(":/widgets/images/enter_details_normal.svg", QSize(16, 16)));

    } else {
        m_color.setRgb(0, 0, 0, 25);
        m_allIconLabel->setPixmap(renderSVG(":/widgets/images/all-dark.svg", QSize(20, 20)));
        m_enterIcon->setPixmap(renderSVG(":/widgets/images/enter_details_normal-dark.svg", QSize(16, 16)));
    }

    QPalette pa = m_textLabel->palette();
    pa.setBrush(QPalette::WindowText, pa.brightText());
    m_textLabel->setPalette(pa);
    update();
}
