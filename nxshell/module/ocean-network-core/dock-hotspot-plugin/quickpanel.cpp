// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanel.h"
#include <DFontSizeManager>
#include <QScreen>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QApplication>

HOTSPOTPLUGIN_BEGIN_NAMESPACE

QuickPanel::QuickPanel(QWidget *parent)
    : QWidget(parent),
      m_icon(new DIconButton(this)),
      m_text(new DLabel(this))
{
    connect(m_icon,&DIconButton::clicked,this,&QuickPanel::iconClicked);
    initUi();
}

const QString QuickPanel::text() const
{
    return m_text->text();
}

void QuickPanel::updateState(DGuiApplicationHelper::ColorType type,bool enabled){
    QString iconStr{"network-hotspot"};
    if(type == DGuiApplicationHelper::ColorType::LightType)
        iconStr += "-dark";
    QPixmap pix = QIcon::fromTheme(iconStr).pixmap(QSize(24, 24));
    if (enabled) {
        QPainter pa(&pix);
        pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
        pa.fillRect(pix.rect(), qApp->palette().highlight());
    }
    setIcon(QIcon(pix));
    m_icon->setChecked(enabled);
}

void QuickPanel::setText(const QString &text){
    m_text->setText(text);
}

void QuickPanel::setIcon(const QIcon& icon){
    m_icon->setIcon(icon);
}

void QuickPanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_icon->rect().contains(event->pos()) && rect().contains(event->pos())) {
        emit iconClicked();
    }
}

void QuickPanel::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignVCenter);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // icon
    m_icon->setFlat(true);
    m_icon->setIconSize(QSize{24,24});
    m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_icon->installEventFilter(this);
    m_icon->setCheckable(true);

    // text
    m_text->setFixedHeight(20);
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setFont(Dtk::Widget::DFontSizeManager::instance()->t10());
    m_text->setText(tr("HotSpot"));

    mainLayout->addWidget(m_icon,0,Qt::AlignCenter);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(m_text);
}

HOTSPOTPLUGIN_END_NAMESPACE
