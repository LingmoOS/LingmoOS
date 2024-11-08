/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: MeihongHe <hemeihong@kylinos.cn>
 *
 */

#include "tab-status-bar.h"
#include "file-info.h"
#include "file-utils.h"
#include "search-vfs-uri-parser.h"
#include "tab-widget.h"
#include "file-info.h"

#include "global-settings.h"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyle>

#include <QUrl>

#include <QToolBar>
#include <QSlider>
#include <QDebug>
#include <QVariantAnimation>

TabStatusBar::TabStatusBar(TabWidget *tab, QWidget *parent) : QStatusBar(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_styled_toolbar = new QToolBar;

    setContentsMargins(0, 0, 0, 0);
    setStyleSheet("padding: 0;");
    setSizeGripEnabled(false);
    setMinimumHeight(30);

    m_tab = tab;
    m_label = new ElidedLabel(this);
    m_label->setContentsMargins(25, 0, 0, 0);
    addWidget(m_label, 1);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setFocusPolicy(Qt::FocusPolicy(Qt::WheelFocus & ~Qt::TabFocus));
    m_slider->setRange(0, 100);
    //设置状态栏下的搜索进度
    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(1000);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(360.0);
    connect(m_animation,&QVariantAnimation::valueChanged, m_label, [=](){
        m_label->setValue(m_animation->currentValue().toDouble());
    });
    connect(m_animation, &QVariantAnimation::finished, this, [=](){
        if(m_searching) {
            m_animation->start();
        }
    });

    auto mainWindow = qobject_cast<MainWindow *>(this->topLevelWidget());
    auto settings = Peony::GlobalSettings::getInstance();
    int defaultZoomLevel = settings->getValue(DEFAULT_VIEW_ID).toInt();
    if (mainWindow) {
        defaultZoomLevel = mainWindow->currentViewZoomLevel();
    }
    m_slider->setValue(defaultZoomLevel);

    connect(m_slider, &QSlider::valueChanged, this, &TabStatusBar::zoomLevelChangedRequest);
    connect(qApp, &QApplication::paletteChanged, this, [=]() {
        this->update();
    });
}

TabStatusBar::~TabStatusBar()
{
    m_styled_toolbar->deleteLater();
}

int TabStatusBar::currentZoomLevel()
{
    if (m_slider->isEnabled()) {
        m_slider->value();
    }
    return -1;
}

void TabStatusBar::update()
{
    if (!m_tab)
        return;
    if (m_searching) {
        m_label->setText(tr("Searching for files ..."));
        return;
    }
    //qDebug() << "TabStatusBar::update";
    auto selections = m_tab->getCurrentSelectionFileInfos();
    //show current path files count
    if (selections.count() ==0)
    {
        auto uris = m_tab->getCurrentAllFileInfos();
        if (uris.count() == 0)
            m_label->setText("");
        else
            m_label->setText(tr(" \%1 items ").arg(uris.count()));

        return;
    }

    //fix select special item issue
    if (selections.count() == 1 && (selections.first()->uri().isNull()
        || (selections.first()->uri() == "network:///"
        || selections.first()->uri() == "computer:///")))
    {
        m_label->setText("");
        return;
    }

    int specialCount = 0;
    goffset size = 0;
    for (auto selection : selections) {
        //not count special path
        if (selection->uri() == "network:///"
           || selection->uri() == "computer:///")
        {
            specialCount++;
            continue;
        }

        if(! selection->isDir() && ! selection->isVolume()){
            size += selection->size();
        }
    }

   //Calculated by 1024 bytes
    auto format_size_GIB = g_format_size_full(size, G_FORMAT_SIZE_IEC_UNITS);
    QString format_size(format_size_GIB);
    //状态栏以GB为显示单位
    format_size.replace("iB", "B");
    if (size > 0)
        m_label->setText(tr(" selected \%1 items    \%2").arg(selections.count()).arg(format_size));
    else
        m_label->setText(tr(" selected \%1 items").arg(selections.count()- specialCount));

    g_free(format_size_GIB);
}

void TabStatusBar::update(const QString &message)
{
    m_label->setText(message);
}

void TabStatusBar::updateZoomLevelState(int zoomLevel)
{
    m_slider->setValue(zoomLevel);
}

void TabStatusBar::onZoomRequest(bool zoomIn)
{
    int value = m_slider->value();
    if (zoomIn) {
        value++;
    } else {
        value--;
    }
    m_slider->setValue(value);
}

//显示隐藏文件，更新项目个数
void TabStatusBar::updateItemsNum()
{
    this->update();
}

void TabStatusBar::paintEvent(QPaintEvent *e)
{
    return;
}

void TabStatusBar::mousePressEvent(QMouseEvent *e)
{
    return;
}

void TabStatusBar::resizeEvent(QResizeEvent *e)
{
    QStatusBar::resizeEvent(e);
    auto pos = this->rect().topRight();
    auto size = m_slider->size();
    m_slider->move(pos.x() - size.width() - 20, this->size().height()/2 - size.height()/2);
}

void TabStatusBar::updateSearchProgress(bool searching)
{
    m_searching = searching;
    if(m_searching) {
        if(m_animation->state() != QAbstractAnimation::Running) {
            m_animation->start();
        }
        m_label->setText(tr("Searching for files ..."));
    } else {
        m_animation->stop();
        update();
    }
    m_label->setSearch(searching);
}

ElidedLabel::ElidedLabel(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(30, 0, 120, 0);
    QIcon icon = QIcon::fromTheme("lingmo-loading-2.symbolic");
    m_disc = icon.pixmap(QSize(16,16));
}

void ElidedLabel::setText(const QString &text)
{
    m_text = text;
    this->update();
}

void ElidedLabel::setValue(double value)
{
    m_val = value;
    this->update();
}

void ElidedLabel::setSearch(bool searching)
{
    m_searching = searching;
}

void ElidedLabel::paintEvent(QPaintEvent *event)
{
    /*!
     * \note
     * paint status bar background and text.
     *
     * designer want to layout status bar into the view, however the status bar belongs to main window now.
     * that means status bar will be layout over the view. it is not possible to archive the ui desginer goals untils we
     * destroy the current directory view frameworks.
     *
     * it is a complex problem, not an easy one. so do not think about changing the layout here to solve the problem easily.
     */
    QStyleOption opt;
    opt.initFrom(this);
    bool active = opt.state &QStyle::State_Active;

    QColor base = active? qApp->palette().color(QPalette::Active, QPalette::Base): qApp->palette().color(QPalette::Inactive, QPalette::Base);

    QFontMetrics fm(this->font());
    auto elidedText = fm.elidedText(m_text, Qt::TextElideMode::ElideRight, this->size().width() - 150);
    QPainter p(this);
//    //p.setCompositionMode(QPainter::CompositionMode_SourceIn);
//    QLinearGradient linearGradient;
//    linearGradient.setStart(QPoint(10, this->height()));
//    linearGradient.setFinalStop(QPoint(10, 0));
//    linearGradient.setColorAt(0, base);
//    linearGradient.setColorAt(0.75, base);
//    linearGradient.setColorAt(1, Qt::transparent);

    int overlap = qApp->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap);
    int layoutWidth = qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int adjustedY2 = qMin(0, overlap - layoutWidth);

    QPainterPath path;
    path.addRect(this->rect().adjusted(0, 0, adjustedY2 - this->height() + 14, 0));

    p.fillPath(path, base);

    if (m_searching) {
        p.save();
        QRect rect = this->rect().adjusted(0, 0, adjustedY2 - this->height() + 14, 0);
        QRect adjustedRect = rect.adjusted(30, 0, -120, 0);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.translate(adjustedRect.topLeft().x(), this->rect().height()/2 - m_disc.height()/2);
        p.translate(m_disc.width()/2,m_disc.height()/2);
        /* 旋转的角度 */
        p.rotate(m_val);
        /* 恢复中心点 */
        p.translate(-m_disc.width()/2,-m_disc.height()/2);

        /* 画图操作 */
        p.drawPixmap(0,0 ,m_disc.width(),m_disc.height(), m_disc);
        p.restore();

        p.translate(m_disc.width()+5 , 0);
    }
//    QPainterPath path2;

//    int radius = this->height();
//    QPoint pos = QPoint(this->width() + adjustedY2 - this->height(), this->height());
//    QRect targetRect = QRect(pos.x() - radius, pos.y() - radius, radius*2, radius*2);
//    path2.moveTo(pos);
//    path2.arcTo(targetRect, 0, 90);

//    QRadialGradient radialGradient;
//    radialGradient.setCenter(pos);
//    radialGradient.setFocalPoint(pos);
//    radialGradient.setRadius(radius);
//    radialGradient.setColorAt(0, base);
//    radialGradient.setColorAt(0.75, base);
//    radialGradient.setColorAt(1, Qt::transparent);
//    p.fillPath(path2, radialGradient);

    style()->drawItemText(&p, this->rect().adjusted(30, 0, -120, 0), Qt::AlignLeft | Qt::AlignVCenter, qApp->palette(), this->isEnabled(), elidedText, QPalette::WindowText);
}

