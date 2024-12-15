// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imgviewdelegate.h"
#include "unionimage/imageutils.h"
#include "unionimage/baseutils.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QThread>
#include <QTimer>
#include <QPainterPath>
#include <QMouseEvent>
#include <QImageReader>
#include <QApplication>

#include <DFontSizeManager>

#include "imgviewlistview.h"
namespace {
const QString IMAGE_DEFAULTTYPE = "All pics";
}

//const int NotSupportedOrDamagedWidth = 40;      //损坏图片宽度
//const int NotSupportedOrDamagedHeigh = 40;
const QString LOCMAP_SELECTED_DARK = ":/dark/images/58 drak.svg";
const QString LOCMAP_NOT_SELECTED_DARK = ":/dark/images/imagewithbg-dark.svg";
const QString LOCMAP_SELECTED_LIGHT = ":/light/images/58.svg";
const QString LOCMAP_NOT_SELECTED_LIGHT = ":/light/images/imagewithbg.svg";

//const QString DAMAGE_IMAGE_DARK_PICTURE = ":/dark/images/picture damaged_dark.svg";
//const QString DAMAGE_IMAGE_LIGHT_PICTURE = ":/light/images/picture damaged_light.svg";
const QString DAMAGE_IMAGE_DARK_PICTURE = ":/dark/images/picture_damaged-58_drak.svg";
const QString DAMAGE_IMAGE_LIGHT_PICTURE = ":/light/images/picture_damaged_58.svg";

const int NORMAL_ITEM_PAINT_OFFSET = 10;//绘制时普通项向下偏移大小
const int SELECT_ITEM_PAINT_OFFSET = 2;//绘制时选中项向下偏移大小

LibImgViewDelegate::LibImgViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_damageImage = QImage(DAMAGE_IMAGE_LIGHT_PICTURE);
    } else {
        m_damageImage = QImage(DAMAGE_IMAGE_DARK_PICTURE);
    }
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ] {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        {
            m_damageImage = QImage(DAMAGE_IMAGE_LIGHT_PICTURE);
        } else
        {
            m_damageImage = QImage(DAMAGE_IMAGE_DARK_PICTURE);
        }
    });
}

void LibImgViewDelegate::setItemSize(QSize size)
{
    Q_UNUSED(size);
//    m_size = size;
}
#include "service/imagedataservice.h"
void LibImgViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    QRect backgroundRect2 = option.rect;
//    painter->fillRect(backgroundRect2, QBrush(DGuiApplicationHelper::instance()->applicationPalette().highlight().color()));
//    return ;
    painter->save();
    QString pixmapstring;
    QImage _pixmap;
    const imageViewerSpace::ItemInfo data = itemData(index);
    if (data.itemType == imageViewerSpace::ItemTypeBlank) {
        painter->restore();
        return;
    }
    _pixmap = data.image;
//    _pixmap = ImageDataService::instance()->getThumnailImageByPath(data.path);
    if (_pixmap.isNull()) {
//        _pixmap = data.damagedPixmap;
        _pixmap = m_damageImage;
    }
//    bool selected = data.isSelected;
//    if (/*(option.state & QStyle::State_MouseOver) &&*/
//        (option.state & QStyle::State_Selected) != 0) {
//        selected = true;
//    }
    painter->setRenderHints(QPainter::HighQualityAntialiasing |
                            QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);
    QRect backgroundRect = option.rect;
    if (backgroundRect.width() != LibImgViewListView::ITEM_CURRENT_WH) {
        backgroundRect.setTopLeft(QPoint(backgroundRect.topLeft() + QPoint(0, NORMAL_ITEM_PAINT_OFFSET)));
        backgroundRect.setBottomRight(QPoint(backgroundRect.bottomRight() + QPoint(0, NORMAL_ITEM_PAINT_OFFSET)));
    } else {
        backgroundRect.setTopLeft(QPoint(backgroundRect.topLeft() + QPoint(0, SELECT_ITEM_PAINT_OFFSET)));
        backgroundRect.setBottomRight(QPoint(backgroundRect.bottomRight() + QPoint(0, SELECT_ITEM_PAINT_OFFSET)));
    }
    QRect pixmapRect;
    QBrush  backbrush;
    //当前显示项
    if (backgroundRect.width() == LibImgViewListView::ITEM_CURRENT_WH) {
        QPainterPath backgroundBp;
        backgroundBp.addRoundedRect(backgroundRect, 8, 8);
        painter->setClipPath(backgroundBp);

        backgroundRect.setX(backgroundRect.x() + 1);
        backgroundRect.setWidth(backgroundRect.width() - 1);
        painter->fillRect(backgroundRect, QBrush(DGuiApplicationHelper::instance()->applicationPalette().highlight().color()));

        pixmapstring = "";
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::DarkType) {
            pixmapstring = LOCMAP_SELECTED_DARK;
            backbrush = QBrush(Libutils::common::DARK_BACKGROUND_COLOR);
        } else {
            pixmapstring = LOCMAP_SELECTED_LIGHT;
            backbrush = QBrush(Libutils::common::LIGHT_BACKGROUND_COLOR);
        }

        //绘制默认选中背景
        QRect backRect(backgroundRect.x() + 4, backgroundRect.y() + 4, backgroundRect.width() - 8, backgroundRect.height() - 8);
        QPainterPath backBp;
        backBp.addRoundedRect(backRect, 4, 4);
        painter->setClipPath(backBp);
        painter->fillRect(backRect, backbrush);

        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        if (!_pixmap.isNull()) {
            pixmapRect.setX(backgroundRect.x() + 4);
            pixmapRect.setY(backgroundRect.y() + 4);
            pixmapRect.setWidth(backgroundRect.width() - 8);
            pixmapRect.setHeight(backgroundRect.height() - 8);
            bg.addRoundedRect(pixmapRect, 4, 4);
            painter->setClipPath(bg);
        }
    } else {
        pixmapRect.setX(backgroundRect.x() + 1);
        pixmapRect.setY(backgroundRect.y() + 0);
        pixmapRect.setWidth(backgroundRect.width() - 2);
        pixmapRect.setHeight(backgroundRect.height() - 40);

        pixmapstring = "";
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::DarkType) {
            pixmapstring = LOCMAP_NOT_SELECTED_DARK;
        } else {
            pixmapstring = LOCMAP_NOT_SELECTED_LIGHT;
        }

//        QPixmap pixmap = utils::base::renderSVG(pixmapstring, QSize(32, 40));
        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        painter->setClipPath(bg);
    }

    //对略缩图的拉伸做了截取，使显示为未拉伸的图片
    QPainterPath bp1;
    bp1.addRoundedRect(pixmapRect, 4, 4);
    painter->setClipPath(bp1);
    _pixmap = _pixmap.scaled(pixmapRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    qreal adjustx = _pixmap.width() - pixmapRect.width();
    qreal adjusty = _pixmap.height() - pixmapRect.height();
    painter->drawImage(pixmapRect,_pixmap,_pixmap.rect().adjusted(adjustx / 2, -adjusty / 2, -adjustx / 2, adjusty / 2));

    painter->restore();
}

QSize LibImgViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.data(Qt::SizeHintRole).value<QSize>();
}

imageViewerSpace::ItemInfo LibImgViewDelegate::itemData(const QModelIndex &index) const
{
    imageViewerSpace::ItemInfo data = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    data.isSelected = index.data(Qt::UserRole).toBool();
    return data;
}

bool LibImgViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(option);
    Q_UNUSED(event);
    Q_UNUSED(model);
    Q_UNUSED(index);
    return false;
}
