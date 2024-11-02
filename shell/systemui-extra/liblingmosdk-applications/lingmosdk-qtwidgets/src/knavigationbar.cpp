/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "knavigationbar.h"
#include <QListView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QScrollBar>
#include <QGSettings>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QLinearGradient>
#include <QDebug>
#include <QToolTip>
#include <QtMath>
#include "themeController.h"
#include "parmscontroller.h"

#define ColorDifference 10

namespace kdk
{

enum ItemType
{
    StandardItem = 0,
    SubItem,
    TagItem
};

class Delegate:public QStyledItemDelegate,public ThemeController
{
public:
    Delegate(QObject*parent,QListView*view);
    bool isPixmapPureColor(const QPixmap &pixmap) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QListView *m_listView;
};

class ListView : public QListView
{
public:
    ListView(QWidget*parent);

protected:
    void mousePressEvent(QMouseEvent* event);
};

class KNavigationBarPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KNavigationBar)

public:
    KNavigationBarPrivate(KNavigationBar*parent);
    void changeTheme();
private:
    KNavigationBar* q_ptr;
    ListView* m_pView;
    QStandardItemModel* m_pModel;
    Delegate* m_pDelegate;
};


KNavigationBar::KNavigationBar(QWidget* parent)
    :QScrollArea(parent),
      d_ptr(new KNavigationBarPrivate(this))
{
    Q_D(KNavigationBar);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    d->m_pView = new ListView (this);
    d->m_pView->setResizeMode(QListView::Adjust);
    d->m_pModel = new QStandardItemModel(d->m_pView);
    d->m_pView->setModel(d->m_pModel);
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    vLayout->addWidget(d->m_pView);
    d->m_pView->setFocus();
    QPalette p = this->palette();
    QColor color(0,0,0,0);
    p.setColor(QPalette::Base,color);
    d->m_pView->setPalette(p);
    this->setPalette(p);
    d->m_pView->setFrameStyle(0);
    d->m_pDelegate = new Delegate(this,d->m_pView);
    d->m_pView->setItemDelegate(d->m_pDelegate);
    d->m_pView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setFrameStyle(0);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    d->changeTheme();
    connect(d->m_pDelegate->m_gsetting,&QGSettings::changed,this,[=](){d->changeTheme();});
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
//        if(Parmscontroller::isTabletMode()) //解决导航栏滑动条切换主题为白条
//         d->m_pView->setStyleSheet("QListView item {height : 48}");
//        else
//         d->m_pView->setStyleSheet("QListView item {height : 36}");
        updateGeometry();
    });
}

void KNavigationBar::addItem(QStandardItem *item)
{
    Q_D(KNavigationBar);
    item->setData(ItemType::StandardItem,Qt::UserRole);
    d->m_pModel->appendRow(item);
}

void KNavigationBar::addSubItem(QStandardItem *subItem)
{
    Q_D(KNavigationBar);
    subItem->setData(ItemType::SubItem,Qt::UserRole);
    QPixmap pix(24,24);
    pix.fill(Qt::transparent);
    QIcon icon(pix);
    subItem->setIcon(icon);
    d->m_pModel->appendRow(subItem);
}


void KNavigationBar::addGroupItems(QList<QStandardItem *> items, const QString &tag)
{
    Q_D(KNavigationBar);
    QStandardItem *item = new QStandardItem(tag);
    item->setEnabled(false);
    item->setData(ItemType::TagItem,Qt::UserRole);
    d->m_pModel->appendRow(item);
    for (auto item : items)
    {
        item->setData(ItemType::StandardItem,Qt::UserRole);
        d->m_pModel->appendRow(item);
    }
}

void KNavigationBar::addTag(const QString &tag)
{
    Q_D(KNavigationBar);
    QStandardItem *item = new QStandardItem(tag);
    item->setEnabled(false);
    item->setData(ItemType::TagItem,Qt::UserRole);
    d->m_pModel->appendRow(item);
}

QStandardItemModel *KNavigationBar::model()
{
    Q_D(KNavigationBar);
    return d->m_pModel;
}

QListView *KNavigationBar::listview()
{
    Q_D(KNavigationBar);
    return d->m_pView;
}

KNavigationBarPrivate::KNavigationBarPrivate(KNavigationBar *parent)
    :q_ptr(parent)
{
    setParent(parent);
}

void KNavigationBarPrivate::changeTheme()
{
    Q_Q(KNavigationBar);
    m_pDelegate->initThemeStyle();
}

Delegate::Delegate(QObject *parent, QListView *view)
    :QStyledItemDelegate(parent),
      m_listView(view)
{
}

bool Delegate::isPixmapPureColor(const QPixmap &pixmap) const
{
    QColor symbolic_color = QColor(38, 38, 38);

        if (pixmap.isNull()) {
            qWarning("pixmap is null!");
            return false;
        }
        QImage image = pixmap.toImage();

        QVector<QColor> vector;
        int total_red = 0;
        int total_green = 0;
        int total_blue = 0;
        bool pure = true;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (image.pixelColor(x, y).alphaF() > 0.3) {
                    QColor color = image.pixelColor(x, y);
                    vector << color;
                    total_red += color.red();
                    total_green += color.green();
                    total_blue += color.blue();
                    int dr = qAbs(color.red() - symbolic_color.red());
                    int dg = qAbs(color.green() - symbolic_color.green());
                    int db = qAbs(color.blue() - symbolic_color.blue());
                    if (dr > ColorDifference || dg > ColorDifference || db > ColorDifference)
                        pure = false;
                }
            }
        }

        if (pure)
            return true;

        qreal squareRoot_red = 0;
        qreal squareRoot_green = 0;
        qreal squareRoot_blue = 0;
        qreal average_red = total_red / vector.count();
        qreal average_green = total_green / vector.count();
        qreal average_blue = total_blue / vector.count();
        for (QColor color : vector) {
            squareRoot_red += (color.red() - average_red) * (color.red() - average_red);
            squareRoot_green += (color.green() - average_green) * (color.green() - average_green);
            squareRoot_blue += (color.blue() - average_blue) * (color.blue() - average_blue);
        }

        qreal arithmeticSquareRoot_red = qSqrt(squareRoot_red / vector.count());
        qreal arithmeticSquareRoot_green = qSqrt(squareRoot_green / vector.count());
        qreal arithmeticSquareRoot_blue = qSqrt(squareRoot_blue / vector.count());
        return arithmeticSquareRoot_red < 2.0 && arithmeticSquareRoot_green < 2.0 && arithmeticSquareRoot_blue < 2.0;
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLinearGradient m_linearGradient;
    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y()+1);
    rect.setWidth(option.rect.width());
//    rect.setHeight(option.rect.height()-2);
    rect.setHeight(option.rect.height()-Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatInterval));

    //QPainterPath画圆角矩形
    const qreal radius = 6; //圆角半径6px
    QPainterPath path;
    QPalette palette = option.palette;
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
         rect = rect.adjusted(1,0,-1,0);
         path.moveTo(rect.topRight());
         path.lineTo(rect.bottomRight());
         path.lineTo(rect.bottomLeft());
         path.lineTo(rect.topLeft());
         path.lineTo(rect.topRight());
    }
    else
    {
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));
    }
    int flag = index.model()->data(index,Qt::UserRole).toInt();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    QColor color;
    QColor borColor;
    m_linearGradient = QLinearGradient(rect.width()/2,rect.y(),rect.width()/2,rect.height()+rect.y());
    if(!(option.state & QStyle::State_Enabled))
    {
        color=QColor("#FFB3B3B3");
        borColor = QColor(0, 0, 73);
    }
    else if(((m_listView->currentIndex() == index) ||(option.state & QStyle::State_Selected)|| (option.state & QStyle::State_MouseOver))
            && flag != 2)
        {
            if((m_listView->currentIndex() == index) ||(option.state & QStyle::State_Selected))
            {
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    if(ThemeController::themeMode() == LightTheme)
                    {
                        color = ThemeController::highlightClick(false,palette);
                        color = ThemeController::adjustH(color,-3);
                        color = ThemeController::adjustS(color,-3);
                        color = ThemeController::adjustL(color,-2);

                        borColor = ThemeController::highlightClick(false,palette);
                        borColor = ThemeController::adjustH(borColor,0);
                        borColor = ThemeController::adjustS(borColor,-34);
                        borColor = ThemeController::adjustL(borColor,-30);
                    }
                    else
                    {
                        color=ThemeController::highlightClick(true,palette);
                        color = ThemeController::adjustH(color,-2);
                        color = ThemeController::adjustS(color,-23);
                        color = ThemeController::adjustL(color,11);

                        borColor = ThemeController::highlightClick(true,palette);
                        borColor = ThemeController::adjustH(borColor,-2);
                        borColor = ThemeController::adjustS(borColor,2);
                        borColor = ThemeController::adjustL(borColor,46);
                    }
                }
                else
                {
                    color = option.palette.highlight().color();
                    if(ThemeController::themeMode() == LightTheme)
                    {
                        m_linearGradient.setColorAt(0,color);
                        m_linearGradient.setColorAt(1,color);
                    }
                    else
                    {
                        m_linearGradient.setColorAt(0,color);
                        m_linearGradient.setColorAt(1,color);
                    }
                }
            }
            else {
                //hover时
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    if(ThemeController::themeMode() == LightTheme)
                    {
                        color = ThemeController::highlightHover(false,palette);
                        color = ThemeController::adjustH(color,-3);
                        color = ThemeController::adjustS(color,-8);
                        color = ThemeController::adjustL(color,-3);

                        borColor = ThemeController::highlightHover(false,palette);
                        borColor = ThemeController::adjustH(borColor,-3);
                        borColor = ThemeController::adjustS(borColor,-24);
                        borColor = ThemeController::adjustL(borColor,-24);
                    }
                    else
                    {
                        color=ThemeController::highlightHover(true,palette);
                        color = ThemeController::adjustH(color,-2);
                        color = ThemeController::adjustS(color,-23);
                        color = ThemeController::adjustL(color,11);

                        borColor = ThemeController::highlightHover(true,palette);
                        borColor = ThemeController::adjustH(borColor,-2);
                        borColor = ThemeController::adjustS(borColor,-2);
                        borColor = ThemeController::adjustL(borColor,36);
                    }
                }
                else if(ThemeController::widgetTheme() == FashionTheme)
                {
                    if(ThemeController::themeMode() == LightTheme)
                    {
                        color = option.palette.windowText().color();
                        color.setAlphaF(0.05);

                        QColor m_color("#E6E6E6");
                        QColor startColor = mixColor(m_color,QColor(Qt::black),0.05);
                        QColor endLightColor = mixColor(m_color,QColor(Qt::black),0.2);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endLightColor);
                    }
                    else
                    {
                        color = option.palette.windowText().color();
                        color.setAlphaF(0.05);

                        QColor color("#373737");
                        QColor startColor = mixColor(color,QColor(Qt::white),0.2);
                        QColor endLightColor = mixColor(color,QColor(Qt::white),0.05);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endLightColor);
                    }
                }
                else
                {
                    color = option.palette.windowText().color();
                    color.setAlphaF(0.05);
                }
            }
            painter->save();
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                painter->setPen(borColor);
            }
            else
            {
                painter->setPen(QPen(Qt::NoPen));
            }
            if(ThemeController::widgetTheme() == FashionTheme )
            {
                painter->setBrush(m_linearGradient);
            }
            else
            {
                painter->setBrush(color);
            }
            if(painter->pen().width() == 1)
                painter->translate(0.5,0.5);
            painter->drawPath(path);
            painter->restore();
        }

    switch (flag) {
    case 0://standardItem
    {
        QRect iconRect;
        if(ThemeController::systemLang())
            iconRect = QRect(rect.right()-32,rect.y()+(rect.height()-16)/2,16,16); //图片大小16*16 左边距16
        else
            iconRect = QRect(rect.x()+16,rect.y()+(rect.height()-16)/2,16,16);
        auto *model =dynamic_cast<QStandardItemModel*>(const_cast<QAbstractItemModel*>(index.model()));
        auto icon = model->item(index.row())->icon();

        if(isPixmapPureColor(icon.pixmap(16,16)))
        {
            if(ThemeController::themeMode() == DarkTheme)
                  icon = ThemeController::drawSymbolicColoredPixmap(icon.pixmap(16,16));
            if((m_listView->currentIndex() == index)|| (option.state & QStyle::State_Selected))
                if(ThemeController::widgetTheme() != ClassicTheme)
                    icon = ThemeController::drawColoredPixmap(icon.pixmap(16,16),QColor(255,255,255));
            if(!(option.state & QStyle::State_Enabled))
                icon = ThemeController::drawColoredPixmap(icon.pixmap(16,16),QColor("#FF979797"));
        }
        icon.paint(painter,iconRect);
        QFontMetrics fm = painter->fontMetrics();
        QString elidedText = fm.elidedText(index.model()->data(index,Qt::DisplayRole).toString(),Qt::ElideRight,rect.width()-56); //左边距+图片宽度+文本图片间距+右边距

        QString mainText = index.data(Qt::DisplayRole).toString();
        if(fm.width(mainText) > rect.width()-56)
            model->item(index.row())->setToolTip(mainText);
        painter->save();
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if((m_listView->currentIndex() == index)||(option.state & QStyle::State_Selected))
                painter->setPen(option.palette.text().color());
        }
        else
        {
            if((m_listView->currentIndex() == index)||(option.state & QStyle::State_Selected))
                painter->setPen(option.palette.highlightedText().color());
        }
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        if(!(option.state & QStyle::State_Enabled))
        {
            painter->setPen(color);
        }
        painter->setFont(font);
        if(!icon.isNull()){
            if(ThemeController::systemLang())
            {

                painter->drawText(QRect(rect.left() + 16,rect.y(),
                                    rect.width()-56,rect.height()),Qt::AlignVCenter,elidedText); //文本 图片间距8px
            }
            else
                painter->drawText(QRect(iconRect.right()+8,rect.y(),
                                    rect.width()-56,rect.height()),Qt::AlignVCenter,elidedText);
        }
        else {
            if(ThemeController::systemLang())
                painter->drawText(QRect(rect.x() + 16,rect.y(),
                                    rect.width() - 56,rect.height()),Qt::AlignVCenter,elidedText);
            else
                painter->drawText(QRect(rect.x()+16,rect.y(),
                                    rect.width()-56,rect.height()),Qt::AlignVCenter,elidedText);
        }
        painter->restore();
        break;
    }
    case 1://subItem
    {
        QRect iconRect=QRect(rect.x()+16,rect.y()+(rect.height()-16)/2,16,16); //图片大小16*16 左边距16
        auto *model =dynamic_cast<QStandardItemModel*>(const_cast<QAbstractItemModel*>(index.model()));

        QFontMetrics fm = painter->fontMetrics();
        QString elidedText = fm.elidedText(index.model()->data(index,Qt::DisplayRole).toString(),Qt::ElideRight,rect.width()-56); //左边距+图片宽度+文本图片间距+右边距

        QString mainText = index.data(Qt::DisplayRole).toString();
        if(fm.width(mainText) > rect.width()-56)
            model->item(index.row())->setToolTip(mainText);
        painter->save();
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            if(/*(option.state & QStyle::State_HasFocus) &&*/ (option.state & QStyle::State_Selected))
                painter->setPen(option.palette.text().color());
        }
        else
        {
            if(/*(option.state & QStyle::State_HasFocus) &&*/ (option.state & QStyle::State_Selected))
                painter->setPen(option.palette.highlightedText().color());
        }
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        if(!(option.state & QStyle::State_Enabled))
        {
            painter->setPen(color);
        }
        painter->setFont(font);
        if(ThemeController::systemLang())
            painter->drawText(QRect(rect.x() + 16,rect.y(),
                                rect.width() - 56,rect.height()),Qt::AlignVCenter,elidedText); //文本 图片间距8px
        else
            painter->drawText(QRect(iconRect.right()+8,rect.y(),
                                    rect.width()-56,rect.height()),Qt::AlignVCenter,elidedText);
        painter->restore();
        break;
    }
    case 2://tagItem
    {
        painter->save();
        auto *model =dynamic_cast<QStandardItemModel*>(const_cast<QAbstractItemModel*>(index.model()));
        QFontMetrics fm = painter->fontMetrics();
        QString elidedText = fm.elidedText(index.model()->data(index,Qt::DisplayRole).toString(),Qt::ElideRight,rect.width()-56); //左边距+图片宽度+文本图片间距+右边距

        QString mainText = index.data(Qt::DisplayRole).toString();
        if(fm.width(mainText) > rect.width()-56)
            model->item(index.row())->setToolTip(mainText);
        if(ThemeController::themeMode() == ThemeFlag::DarkTheme)
            painter->setPen(QColor(115,115,115));
        else
            painter->setPen(QColor(140,140,140));

        QRect textRect = option.rect;
        textRect = textRect.adjusted(16,12,0,0);
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        painter->setFont(font);
        painter->drawText(textRect,Qt::AlignVCenter,elidedText);
        painter->restore();
        break;
    }
    default:
        break;
    }
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int flag = index.model()->data(index,Qt::UserRole).toInt();
    QSize size;
    switch (flag) {
    case 2://tagItem
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatHeight)+12);
        break;
    default:
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatHeight)+Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatInterval));
        break;
    }
    size.setWidth(option.rect.width());
    return size;
}

ListView::ListView(QWidget *parent)
    :QListView(parent)
{

}

void ListView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() & Qt::RightButton)
        return ;
    else if(event->button() & Qt::LeftButton)
        QListView::mousePressEvent(event);
}

}

#include "knavigationbar.moc";
#include "moc_knavigationbar.cpp";
