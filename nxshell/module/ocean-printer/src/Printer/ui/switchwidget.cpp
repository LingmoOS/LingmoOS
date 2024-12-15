// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "switchwidget.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QToolTip>
#include <QPainter>

#include <DFontSizeManager>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

SwitchLabel::SwitchLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel (parent,f)
{
    setAccessibleName("SwitchLabel");
}

void SwitchLabel::resizeEvent(QResizeEvent *event)
{
    if (m_sourceText.isEmpty()) {
        m_sourceText = this->text();
    }

    m_actualSize = event->size();
    QFontMetrics fontMetrics(this->font());

    QString str = m_sourceText;
    int len = fontMetrics.horizontalAdvance(m_sourceText);
    if (len > m_actualSize.width()) {
        str = fontMetrics.elidedText(str, Qt::ElideRight, m_actualSize.width());
        this->setText(str);
    } else {
        this->setText(m_sourceText);
    }

    QLabel::resizeEvent(event);
}


SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_leftWidget(new SwitchLabel)
    , m_switchBtn(new DSwitchButton)
{
    m_switchBtn->setAccessibleName(title);
    qobject_cast<SwitchLabel*>(m_leftWidget)->setText(title);
    init();
}

SwitchWidget::SwitchWidget(QWidget *parent, QWidget *widget)
    : QFrame(parent)
    , m_leftWidget(widget)
    , m_switchBtn(new DSwitchButton)
{
    if (!m_leftWidget) {
        m_leftWidget = new SwitchLabel();
    }

    init();
}

void SwitchWidget::init()
{
    setMinimumHeight(36);
    QHBoxLayout *lableLayout = new QHBoxLayout;
    lableLayout->addWidget(m_leftWidget);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 2, 8, 2);

    m_mainLayout->addLayout(lableLayout, 0);
    m_mainLayout->addWidget(m_switchBtn, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    DFontSizeManager::instance()->bind(m_leftWidget, DFontSizeManager::T6, QFont::Medium);
    connect(m_switchBtn, &DSwitchButton::toggled, this, &SwitchWidget::checkedChanged);
}

void SwitchWidget::setChecked(const bool checked)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

QString SwitchWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

   return QString();
}

void SwitchWidget::setTitle(const QString &title)
{
    SwitchLabel *label = qobject_cast<SwitchLabel *>(m_leftWidget);
    if (label) {
        label->setWordWrap(true);
        label->setText(title);
    }
    setAccessibleName(title);
    m_switchBtn->setAccessibleName(title);
}

bool SwitchWidget::checked() const
{
    return m_switchBtn->isChecked();
}

void SwitchWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_switchBtn->geometry().contains(event->pos())) {
        Q_EMIT clicked();
    }

    return QFrame::mouseReleaseEvent(event);
}

bool SwitchWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        if (toolTip().isEmpty()) {
            SwitchLabel *label = qobject_cast<SwitchLabel *>(m_leftWidget);
            if (label && label->text() != label->sourceTitle()) {
                QToolTip::showText(QCursor::pos(), label->sourceTitle());
                return true;
            }
        }
    }

    return QFrame::event(event);
}

void SwitchWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const DPalette &dp = DApplicationHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));
    p.drawRoundedRect(rect(), 8, 8);
    QFrame::paintEvent(event);
}
