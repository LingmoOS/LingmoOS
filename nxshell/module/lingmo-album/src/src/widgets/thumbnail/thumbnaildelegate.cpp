// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnaildelegate.h"
#include "thumbnaillistview.h"
#include "unionimage/imageutils.h"
#include "unionimage/baseutils.h"
#include "dbmanager/dbmanager.h"
#include "globalstatus.h"
//#include "statusbar.h"
//#include "application.h"
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
#include <DGuiApplicationHelper>
#include <DStyleOptionButton>
#include <DStyle>
#include <DApplication>
#include <DDciIcon>
//#include <timelinedatewidget.h>

DWIDGET_USE_NAMESPACE

//#include "imageengineapi.h"
#include "imageengine/imagedataservice.h"
//#include "signalmanager.h"
namespace {
const QString IMAGE_DEFAULTTYPE = "All pics";
const int CheckIcon_Size = 20;
}

const int NotSupportedOrDamagedWidth = 40;      //损坏图片宽度
const int NotSupportedOrDamagedHeigh = 40;
const int FavoriteIconSize = 25;                //收藏图标尺寸

ThumbnailDelegate::ThumbnailDelegate(DelegateType type, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_imageTypeStr(IMAGE_DEFAULTTYPE)
    , m_delegatetype(type)
{
}

void ThumbnailDelegate::setItemSize(QSize size)
{
    m_size = size;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!bneedpaint) {
        return;
    }
    const DBImgInfo data = itemData(index);
    switch (data.itemType) {
    case ItemType::ItemTypeBlank: {
        //空白项，什么都不绘制
        break;
    }
    case ItemType::ItemTypeTimeLineTitle: {
        break;
    }
    case ItemType::ItemTypeImportTimeLineTitle: {
        break;
    }
    case ItemType::ItemTypePic:
    case ItemType::ItemTypeVideo: {
        drawImgAndVideo(painter, option, index);
        break;
    }
    default:
        drawImgAndVideo(painter, option, index);
        break;
    }
    return;
}

void ThumbnailDelegate::drawImgAndVideo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    const DBImgInfo data = itemData(index);
    bool selected = false;
    if ((option.state & QStyle::State_Selected) != 0) {
        selected = true;
    }
    painter->setRenderHints(QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);
    QRect backgroundRect = option.rect;
    QImage img = ImageDataService::instance()->getThumnailImageByPathRealTime(data.filePath, COMMON_STR_TRASH == m_imageTypeStr);
    if (img.isNull()) {
        if (data.itemType == ItemTypeVideo) {
            img = m_videoDefault.toImage();
        } else {
            img = m_damagePixmap.toImage();
        }
    }

    // QPen oldPen = painter->pen();
    // painter->setPen(Qt::red);
    // painter->drawRect(backgroundRect);
    // painter->setPen(oldPen);
    backgroundRect = updatePaintedRect(backgroundRect, img);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    //选中阴影框
    if (selected) {
        QPainterPath backgroundBp;
        backgroundBp.addRoundedRect(backgroundRect, Libutils::common::SHADOW_BORDER_RADIUS, Libutils::common::SHADOW_BORDER_RADIUS);
        painter->setClipPath(backgroundBp);

        QBrush shadowbrush;

        if (themeType == DGuiApplicationHelper::LightType) {
            shadowbrush = QBrush(QColor("#DEDEDE"));
        }
        if (themeType == DGuiApplicationHelper::DarkType) {
            shadowbrush = QBrush(QColor("#4F4F4F"));
        }
        painter->fillRect(backgroundRect, shadowbrush);

        //绘制选中默认背景 屏蔽了好像也没影响
        //QRect backRect(backgroundRect.x() + 8, backgroundRect.y() + 8, backgroundRect.width() - 16, backgroundRect.height() - 16);
        //QPainterPath backBp;
        //backBp.addRoundedRect(backRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
        //painter->setClipPath(backBp);
        //painter->fillRect(backRect, shadowbrush);
    }

    QRect pixmapRect;
    if (img.isNull()) {
        pixmapRect.setX(backgroundRect.x() + backgroundRect.width() / 2 - NotSupportedOrDamagedWidth / 2);
        pixmapRect.setY(backgroundRect.y() + backgroundRect.height() / 2 - NotSupportedOrDamagedHeigh / 2);
        pixmapRect.setWidth(NotSupportedOrDamagedWidth);
        pixmapRect.setHeight(NotSupportedOrDamagedHeigh);
    } else {
        pixmapRect.setX(backgroundRect.x() + 8);
        pixmapRect.setY(backgroundRect.y() + 8);
        pixmapRect.setWidth(backgroundRect.width() - 16);
        pixmapRect.setHeight(backgroundRect.height() - 16);
    }
    //2020/6/9 DJH UI 透明图片背景
    QBrush transparentbrush;
    if (themeType == DGuiApplicationHelper::LightType) {
        transparentbrush = QBrush(QColor("#FFFFFF"));
    } else if (themeType == DGuiApplicationHelper::DarkType) { //#BUG77517，去除下方的虚化代码，改为直接填充黑色
        transparentbrush = QBrush(QColor("#000000"));
    }
    QRect transparentRect(backgroundRect.x() + 8, backgroundRect.y() + 8, backgroundRect.width() - 16, backgroundRect.height() - 16);
    QPainterPath transparentBp;
    transparentBp.addRoundedRect(transparentRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
    painter->setClipPath(transparentBp);
    painter->fillRect(transparentRect, transparentbrush);

    QPainterPath bp1;
    bp1.addRoundedRect(pixmapRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
    painter->setClipPath(bp1);

    if (!ImageDataService::instance()->imageIsLoaded(data.filePath, COMMON_STR_TRASH == m_imageTypeStr)) {
        painter->drawPixmap(pixmapRect, m_default);
    } else {
        if (img.isNull()) {
            if (data.itemType == ItemTypeVideo) {
                painter->drawPixmap(pixmapRect, m_videoDefault);
            } else {
                painter->drawPixmap(pixmapRect, m_damagePixmap);
            }
        } else {
            painter->drawPixmap(pixmapRect, QPixmap::fromImage(img));
        }
    }

    //绘制选中图标
    if (selected) {
        QRect rc = backgroundRect;
        rc.setSize({CheckIcon_Size, CheckIcon_Size});
        rc.moveTopRight(QPoint(backgroundRect.right() - 1, backgroundRect.top() + 1));

        // 使用dtk风格绘制选中图标，以便选中图标背景色跟随主题高亮颜色改变
        DStyleOptionButton check;
        check.state = DStyle::State_Enabled | DStyle::State_On;
        check.rect = rc;

        QPainterPath selectedBp;
        selectedBp.addRect(rc);
        painter->setClipPath(selectedBp);
        DApplication::style()->drawPrimitive(DStyle::PE_IndicatorItemViewItemCheck, &check, painter);
    }

    //绘制剩余天数
    if (COMMON_STR_TRASH == m_imageTypeStr) {
        QPainterPath bp;
        bp.addRoundedRect(backgroundRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
        painter->setClipPath(bp);
        painter->setPen(QColor(85, 85, 85, 170)); //边框颜色：灰色
        //字符串的像素宽度
        QString str(QString::number(data.remainDays) + ThumbnailListView::tr("days"));
        const int m_Width = painter->fontMetrics().boundingRect(str).width();
        painter->setBrush(QBrush(QColor(85, 85, 85, 170)));//填充颜色：灰色
        QFontMetrics Text(str);

        //2020/3/13-xiaolong
        int textwidth = m_Width + 6;        //阴影图框：6：左3像素+右3像素
        int textheight = DFontSizeManager::instance()->fontPixelSize(painter->font());
        int rectwidth = backgroundRect.width() - 8; //缩略图宽度：总宽度减去选中框宽度8
        if (textwidth > rectwidth) { //容纳文字像素的宽度大于缩略图宽度
            textwidth = rectwidth - 4;//减少距离：左右各2
        }
        int tempcha = (rectwidth - textwidth > 4) ? (rectwidth - textwidth - 4) : 4;
        int posx = backgroundRect.x() + tempcha;    //剩余天数起始坐标
        //文字背景圆角矩形框弧度，与字号相关
        int radious = 6;
        if (textheight < 14) {
            radious = 4;
        }

        painter->drawRoundedRect(posx, backgroundRect.y() + backgroundRect.height() - textheight - 14,
                                 textwidth, textheight + 2, radious, radious); //Y参数：backgroundRect宽度-文字宽度-14边距

        painter->setPen(QColor(255, 255, 255));
        if (m_Width - textwidth > 0) {
            str = Text.elidedText(str, Qt::ElideRight, textwidth);
        }
        painter->drawText(posx + 3, backgroundRect.y() + backgroundRect.height() - 15, str);//在框中绘制文字，起点位置离最下方15像素
    }
    //绘制视频时间
    if (data.itemType == ItemTypeVideo) {
        QString str(ImageDataService::instance()->getMovieDurationStrByPath(data.filePath));
        if (str == "-") {
            str = "00:00:00";
        }
        //TODO 暂时屏蔽最近删除界面的视频时间显示，待需求明确显示位置后修改并解除屏蔽
        if (/*dApp->signalM->getSliderValue() > 2 && */!str.isEmpty() && m_delegatetype != AlbumViewTrashType) {
            QPainterPath bp;
            bp.addRoundedRect(backgroundRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
            painter->setClipPath(bp);
            painter->setPen(QColor(85, 85, 85, 170)); //边框颜色：灰色
            //字符串的像素宽度
            painter->setBrush(QBrush(QColor(85, 85, 85, 170)));//填充颜色：灰色

            const int m_Width = painter->fontMetrics().boundingRect(str).width();
            QFontMetrics Text(str);

            //2020/3/13-xiaolong
            int textwidth = m_Width + 6;        //阴影图框：6：左3像素+右3像素
            int textheight = DFontSizeManager::instance()->fontPixelSize(painter->font());
            int rectwidth = backgroundRect.width() - 8; //缩略图宽度：总宽度减去选中框宽度8
            if (textwidth > rectwidth) { //容纳文字像素的宽度大于缩略图宽度
                textwidth = rectwidth - 4;//减少距离：左右各2
            }
            int posx = backgroundRect.x() + backgroundRect.width() - m_Width - 16;  //时长起始坐标
            //文字背景圆角矩形框弧度，与字号相关
            int radious = 6;
            if (textheight < 14) {
                radious = 4;
            }

            painter->drawRoundedRect(posx, backgroundRect.y() + backgroundRect.height() - textheight - 14,
                                     textwidth, textheight + 2, radious, radious); //Y参数：backgroundRect宽度-文字宽度-14边距

            painter->setPen(QColor(255, 255, 255));
            if (m_Width - textwidth > 0) {
                str = Text.elidedText(str, Qt::ElideRight, textwidth);
            }
            painter->drawText(posx + 3, backgroundRect.y() + backgroundRect.height() - 15, str);
        }
    }

    //绘制心形图标
    bool bFavorited = DBManager::instance()->isAllImgExistInAlbum(DBManager::SpUID::u_Favorite, QStringList(data.filePath), AlbumDBType::Favourite);
    if (option.state & QStyle::StateFlag::State_MouseOver || bFavorited) {
        QPainterPath bp;
        bp.addRoundedRect(backgroundRect, Libutils::common::BORDER_RADIUS, Libutils::common::BORDER_RADIUS);
        painter->setClipPath(bp);

        DDciIcon dciIcon(bFavorited ? QString(":/dsg/icons/collected.dci") : QString(":/dsg/icons/collection2.dci"));
        DDciIconMatchResult dciIconMatched = dciIcon.matchIcon(20, themeType == DGuiApplicationHelper::LightType ? DDciIcon::Light : DDciIcon::Dark, DDciIcon::Normal, DDciIcon::DontFallbackMode);
        QPixmap favPixmap = dciIcon.pixmap(1, FavoriteIconSize, dciIconMatched);

        QRect favoriteRect(backgroundRect.x() + 10, backgroundRect.y() + backgroundRect.height() - favPixmap.height() - 10, FavoriteIconSize, FavoriteIconSize);
        //painter->drawRect(favoriteRect);
        painter->drawPixmap(favoriteRect.x(), favoriteRect.y(), favPixmap);
    }

    painter->restore();
}

void ThumbnailDelegate::onThemeTypeChanged(int themeType)
{
    Q_UNUSED(themeType)
    m_damagePixmap = Libutils::image::getDamagePixmap(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType);

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_default = Libutils::base::renderSVG(":/icons/lingmo/builtin/icons/light/picture_default_light.svg", QSize(60, 45));
        m_videoDefault = Libutils::base::renderSVG(":/icons/lingmo/builtin/icons/light/video_default_light.svg", QSize(60, 45));
    } else {
        m_default = Libutils::base::renderSVG(":/icons/lingmo/builtin/icons/dark/picture_default_dark.svg", QSize(60, 45));
        m_videoDefault = Libutils::base::renderSVG(":/icons/lingmo/builtin/icons/dark/video_default_dark.svg", QSize(60, 45));
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_UNUSED(option)
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
    if (data.itemType == ItemType::ItemTypeBlank
            || data.itemType == ItemType::ItemTypeTimeLineTitle
            || data.itemType == ItemType::ItemTypeImportTimeLineTitle) {
        QSize size(data.imgWidth, data.imgHeight);
        return size;
    } else {
        return m_size;
    }
}

DBImgInfo ThumbnailDelegate::itemData(const QModelIndex &index) const
{
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
    data.isSelected = index.data(Qt::UserRole).toBool();
    return data;
}

QRect ThumbnailDelegate::updatePaintedRect(const QRectF &boundingRect, const QImage& image) const
{
    QRectF destRect;

    QSizeF scaled = image.size();

    QSizeF size = boundingRect.size();
    scaled.scale(boundingRect.size(), Qt::KeepAspectRatio);
    destRect = QRectF(QPoint(0, 0), scaled);
    destRect.moveCenter(boundingRect.center().toPoint());

    return destRect.toRect();
}

bool ThumbnailDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(model);
    if (!index.isValid())
        return false;
    //跟随收藏按钮变化位置
    QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
    {
        if (event->type() == QEvent::MouseButtonPress) {
            const DBImgInfo data = itemData(index);
            QPoint pos = pMouseEvent->pos();
            QRect backgroundRect = option.rect;
            QImage img = ImageDataService::instance()->getThumnailImageByPathRealTime(data.filePath, COMMON_STR_TRASH == m_imageTypeStr);
            if (img.isNull()) {
                if (data.itemType == ItemTypeVideo) {
                    img = m_videoDefault.toImage();
                } else {
                    img = m_damagePixmap.toImage();
                }
            }
            QRect favoriteRect = updatePaintedRect(backgroundRect, img);
            favoriteRect.moveTo(favoriteRect.x() + 10, favoriteRect.bottom() - FavoriteIconSize - 10);
            favoriteRect.setSize(QSize(FavoriteIconSize, FavoriteIconSize));
            if (favoriteRect.contains(pos)) {
                bool bFavorited = DBManager::instance()->isAllImgExistInAlbum(DBManager::SpUID::u_Favorite, QStringList(data.filePath), AlbumDBType::Favourite);
                if (bFavorited)
                    DBManager::instance()->removeFromAlbum(DBManager::SpUID::u_Favorite, QStringList() << data.filePath, AlbumDBType::Favourite);
                else
                    DBManager::instance()->insertIntoAlbum(DBManager::SpUID::u_Favorite, QStringList() << data.filePath, AlbumDBType::Favourite);
                GlobalStatus::instance()->setBRefreshFavoriteIconFlag(!GlobalStatus::instance()->bRefreshFavoriteIconFlag());
            }
        }
    }
    return false;
}
