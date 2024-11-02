/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "search-line-edit.h"
#include "icon-loader.h"
#include <KWindowEffects>
#include <QApplication>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
QT_END_NAMESPACE
using namespace LingmoUISearch;
/**
 * @brief UKuiSearchLineEdit  全局搜索的输入框
 */
SearchLineEdit::SearchLineEdit(int radius, QWidget *parent) : QLineEdit(parent), m_radius(radius)
{
    setStyle(new LineEditStyle(m_radius));
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFixedSize(680, 50);
    this->setTextMargins(35, 0, 0, 0);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setDragEnabled(true);
    m_queryIcon = new QLabel;
    QPixmap pixmap = QPixmap(IconLoader::loadIconQt("system-search-symbolic", QIcon(":/res/icons/system-search.symbolic.png")).pixmap(QSize(18, 18)));
    m_queryIcon->setProperty("useIconHighlightEffect", 0x02);
    m_queryIcon->setFixedSize(pixmap.size() / pixmap.devicePixelRatio());
    m_queryIcon->setPixmap(pixmap);

    m_ly = new QHBoxLayout(this);
    m_ly->addSpacing(8);
    m_ly->addWidget(m_queryIcon);
    m_ly->addStretch();

    this->setPlaceholderText(tr("Search"));
    this->setMaxLength(100);

    m_timer = new QTimer;
    connect(m_timer, &QTimer::timeout, this, [ = ]() {
        m_timer->stop();
        Q_EMIT this->requestSearchKeyword(this->text());
    });
    connect(this, &SearchLineEdit::textChanged, this, [ = ](QString text) {
        if(m_isEmpty) {
            m_isEmpty = false;
            Q_EMIT this->requestSearchKeyword(text);
        } else {
            if(text == "") {
                m_isEmpty = true;
                m_timer->stop();
                Q_EMIT this->requestSearchKeyword(text);
                return;
            }
            m_timer->start(0.1 * 1000);
        }
    });

    //跟随主题透明度变化
    connect(qApp, &QApplication::paletteChanged, this, [=]() {
        update();
    });
}

SearchLineEdit::~SearchLineEdit() {

}

void SearchLineEdit::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(palette().base());
    p.setOpacity(GlobalSettings::getInstance().getValue(TRANSPARENCY_KEY).toDouble());
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), m_radius, m_radius);
    return QLineEdit::paintEvent(e);
}

void SearchLineEdit::focusOutEvent(QFocusEvent *e)
{
    Q_UNUSED(e)
    this->setFocus();
}

void SearchLineEdit::setRadius(int radius)
{
    m_radius = radius;
    auto style = dynamic_cast<LineEditStyle *>(this->style());
    if(style) {
        style->setRadius(m_radius);
    }
    update();
}

SearchBarWidget::SearchBarWidget(QWidget *parent): QWidget(parent)
{

    m_ly = new QHBoxLayout(this);
    m_radius = GlobalSettings::getInstance().getValue(WINDOW_RADIUS_KEY).toInt();
    m_searchLineEdit = new SearchLineEdit(m_radius, this);
    connect(&GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [&](const QString& key, const QVariant& value){
        if(key == WINDOW_RADIUS_KEY) {
            m_radius = value.toInt();
            m_searchLineEdit->setRadius(m_radius);
        }
    });
    this->setFixedSize(m_searchLineEdit->width()+20, m_searchLineEdit->height()+20);
    m_ly->setContentsMargins(0,0,0,0);
    m_ly->addWidget(m_searchLineEdit);
    this->setFocusProxy(m_searchLineEdit);
    connect(m_searchLineEdit, &SearchLineEdit::requestSearchKeyword, this, &SearchBarWidget::requestSearchKeyword);

}

SearchBarWidget::~SearchBarWidget() {
}

void SearchBarWidget::clear()
{
    m_searchLineEdit->clear();
}

void SearchBarWidget::reSearch()
{
    Q_EMIT this->m_searchLineEdit->requestSearchKeyword(m_searchLineEdit->text());
}

void SearchBarWidget::setText(QString keyword)
{
    m_searchLineEdit->setText(keyword);
}

void SearchBarWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), m_radius, m_radius);


    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    //      pixmapPainter.setCompositionMode(QPainter::CompositionMode_Difference);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();


    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);


    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);


    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());
}

void LineEditStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_PanelLineEdit://LINGMO Text edit style
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            const bool enable = f->state & State_Enabled;
            const bool focus = f->state & State_HasFocus;

            if (!enable) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (f->state & State_ReadOnly) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Active, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, m_radius, m_radius);
                painter->restore();
                return;
            }

            if (!focus) {
                QStyleOptionButton button;
                button.state = option->state & ~(State_Sunken | State_On);
                button.rect = option->rect;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);
            }
            return;
        }
        break;
    }
    default:
        return;
//        return QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

LineEditStyle::LineEditStyle(int radius)
{
    m_radius = radius;
}

void LineEditStyle::setRadius(int radius)
{
    m_radius = radius;
}
