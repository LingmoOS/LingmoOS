// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitemdelegate.h"
#include "constants.h"
#include "calculate_util.h"
#include "util.h"
#include "appslistmodel.h"

#include <QDebug>
#include <QPixmap>
#include <QVariant>
#include <QApplication>

#define ICONTOLETF  12
#define ICONTOTOP  6
#define TEXTTOICON  2
#define TEXTTOLEFT  10
#define RECT_REDIUS 18

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_calcUtil(CalculateUtil::instance())
    , m_blueDotPixmap(QIcon(":/skin/images/new_install_indicator.svg").pixmap(QSize(10, 10)))
    , m_autoStartPixmap(QIcon(":/skin/images/emblem-autostart.svg").pixmap(QSize(24, 24)))
{
}

void AppItemDelegate::setCurrentIndex(const QModelIndex &index)
{
    if (CurrentIndex == index)
        return;

    const QModelIndex previousIndex = CurrentIndex;
    CurrentIndex = index;

    emit requestUpdate(previousIndex);
    emit requestUpdate(CurrentIndex);
}

void AppItemDelegate::setDirModelIndex(QModelIndex dragIndex, QModelIndex dropIndex)
{
    m_dragIndex = dragIndex;
    m_dropIndex = dropIndex;
}

void AppItemDelegate::setItemList(const ItemInfoList_v1 &items)
{
    m_itemList = items;
}

void AppItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(AppsListModel::AppItemIsDraggingRole).value<bool>() && !(option.features & QStyleOptionViewItem::HasDisplay))
        return;

#ifdef QT_DEBUG
    painter->setBrush(Qt::red);
    painter->drawRect(option.rect);
#endif

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::white);
    painter->setBrush(QBrush(Qt::transparent));

    const ItemInfo_v1 itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();
    const QPixmap iconPix = index.data(AppsListModel::AppIconRole).value<QPixmap>();
    const bool itemIsDir = index.data(AppsListModel::ItemIsDirRole).toBool();
    const ItemInfoList_v1 itemList = index.data(AppsListModel::DirItemInfoRole).value<ItemInfoList_v1>();
    const int itemStatus = index.data(AppsListModel::AppItemStatusRole).toInt();

    QList<QPixmap> pixmapList;

    const int fontPixelSize = index.data(AppsListModel::AppFontSizeRole).value<int>();
    const bool drawBlueDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    const bool is_current = CurrentIndex == index;
    const QRect itemBoundRect = itemBoundingRect(option.rect);
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize();

    QFont appNamefont(painter->font());
    if (fontPixelSize <= 0)
        return;

    appNamefont.setPixelSize(fontPixelSize);
    const QFontMetrics fm(appNamefont);
    painter->setOpacity(1);
    const static double x1 = 0.26418192;
    const static double x2 = -0.38890932;
    const double result = x1 * itemBoundRect.width() + x2 * iconSize.width();
    int margin = result > 0 ? result * 0.71 : 1;

    // adjust
    QRect br;
    QRect iconRect;
    QRectF appNameRect;
    QString appNameResolved;
    bool adjust = false;
    bool  TextSecond = 0;
    do {
        // adjust
        if (adjust)
            --margin;
        br = itemBoundRect.marginsRemoved(QMargins(margin, 1, margin, margin * 2));

        // calc icon rect
        const int iconLeftMargins = (br.width() - iconSize.width()) / 2;
        int iconTopMargin = ICONTOTOP;
        iconRect = QRect(br.topLeft() + QPoint(iconLeftMargins, iconTopMargin - 2), iconSize);

        //31是字体设置20的时候的高度
        br.setHeight(ICONTOTOP + iconRect.height() + TEXTTOICON + 31 + fontPixelSize * TextSecond + TEXTTOLEFT);
        if (br.height() > itemBoundRect.height())
            br.setHeight(itemBoundRect.height() - 1);

        // calc text
        appNameRect = itemTextRect(br, iconRect, drawBlueDot);

        bool showSuffix = ConfigWorker::getValue(DLauncher::SHOW_LINGLONG_SUFFIX).toBool();
        bool isLingLongApp = itemInfo.isLingLongApp();
        bool isDir = itemInfo.m_isDir;
        const QString &displayName = (showSuffix && isLingLongApp && !isDir) ? (QString("%1(%2)").arg(itemInfo.m_name).arg(tr("LingLong"))) : itemInfo.m_name;
        const QPair<QString, bool> appTextResolvedInfo = holdTextInRect(fm, displayName, appNameRect.toRect());
        appNameResolved = appTextResolvedInfo.first;

        if ((fm.width(appNameResolved) + (drawBlueDot ? (m_blueDotPixmap.width() + 10) : 0)) >= appNameRect.width())
            TextSecond = 1;

        if (margin == 1 || appTextResolvedInfo.second) {
            br.setHeight(ICONTOTOP + iconRect.height() + TEXTTOICON + 31 + fontPixelSize * TextSecond + TEXTTOLEFT);
            appNameRect = itemTextRect(br, iconRect, drawBlueDot);
            break;
        }

        adjust = true;
    } while (true);

    // 绘制选中样式
   if (is_current && !(option.features & QStyleOptionViewItem::HasDisplay) && !itemIsDir && m_calcUtil->fullscreen()) {
        QColor brushColor(Qt::white);
        brushColor.setAlpha(51);

        painter->setPen(Qt::transparent);
        painter->setBrush(brushColor);
        int drawBlueDotWidth = 0;
        if (drawBlueDot)
            drawBlueDotWidth = m_blueDotPixmap.width();

        if (iconSize.width() > (fm.width(appNameResolved) + drawBlueDotWidth)) {
            br.setX(iconRect.x() - ICONTOLETF);
            br.setWidth(iconSize.width() + ICONTOLETF * 2);
        }

        painter->drawRoundedRect(br, RECT_REDIUS, RECT_REDIUS);
    } else if (is_current && !m_calcUtil->fullscreen()) {
       QColor brushColor;
       if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType())
           brushColor = Qt::white;
       else
           brushColor = Qt::black;

       brushColor.setAlpha(25);
       painter->setBrush(brushColor);
       painter->setPen(Qt::transparent);
       painter->drawRoundedRect(br, RECT_REDIUS / 2, RECT_REDIUS / 2);
    }

    // 绘制应用名称
    QTextOption appNameOption;
    appNameOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    appNameOption.setWrapMode(QTextOption::WordWrap);

    appNameRect.setY(br.y() + br.height() - TEXTTOLEFT + (fm.height() >= 28 ? 2 : 0) - fm.height() - fontPixelSize * TextSecond);

    if (drawBlueDot) {
        appNameRect.setX(appNameRect.x() + m_blueDotPixmap.width() / 2 + 5);
        appNameRect.setWidth(appNameRect.width() - m_blueDotPixmap.width());
    }

    if (m_calcUtil->fullscreen())
        drawAppDrawer(painter, index, iconRect);

    painter->setFont(appNamefont);
    painter->setBrush(QBrush(Qt::transparent));
    painter->setPen(Qt::white);

    if (!m_calcUtil->fullscreen()) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            painter->setPen(Qt::white);
        else
            painter->setPen(Qt::black);

        painter->setFont(QFont(painter->font().family(), DLauncher::DEFAULT_FONT_SIZE));
        painter->drawText(appNameRect, appNameResolved, appNameOption);
    } else {
        QRectF progressRect, buttonRect;
        progressRect.setTop(appNameRect.bottom() + 5);
        progressRect.setLeft(option.rect.left() + (option.rect.right() - option.rect.width() / 3) / 2);
        progressRect.setWidth(option.rect.width() / 3);
        progressRect.setHeight(option.rect.height() * 0.05);

        buttonRect.setTop(progressRect.bottom() + 5);
        buttonRect.setLeft(option.rect.left() + (option.rect.right() - option.rect.width() / 2) / 2);
        buttonRect.setWidth(option.rect.width() / 2);
        buttonRect.setHeight(option.rect.height() * 0.15);

        if (!itemIsDir && itemStatus == ItemInfo_v1::Busy) {
            painter->drawText(appNameRect, appNameResolved, appNameOption);
            QStyleOptionProgressBar progressBar;
            progressBar.text = QString::number(itemInfo.m_progressValue);
            progressBar.minimum = 0;
            progressBar.maximum = 100;
            progressBar.progress = itemInfo.m_progressValue;
            progressBar.invertedAppearance = true;
            progressBar.orientation = Qt::Horizontal;
            progressBar.textVisible = true;
            progressBar.textAlignment = Qt::AlignHCenter;
            progressBar.rect = progressRect.toRect();

            QStyleOptionButton button;
            button.rect = buttonRect.toRect();
            button.text = itemInfo.m_description;
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
            QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
        } else {
            const ItemInfo_v1 info = index.data(AppsListModel::DirNameRole).value<ItemInfo_v1>();
            painter->drawText(appNameRect, appNameResolved, appNameOption);
        }
    }

    if (!itemIsDir) {
        painter->drawPixmap(iconRect, iconPix, iconPix.rect());
        if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
            const QPoint autoStartIconPos = iconRect.bottomLeft()
                    - QPoint(m_autoStartPixmap.height(), m_autoStartPixmap.width()) / m_autoStartPixmap.devicePixelRatioF() / 2
                    + QPoint(iconRect.width() / 10, -iconRect.height() / 10);
            painter->drawPixmap(autoStartIconPos, m_autoStartPixmap);
        }
    }

    if (drawBlueDot) {
        const int marginRight = 2;
        const QRectF textRect = fm.boundingRect(appNameRect.toRect(), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, appNameResolved);
        const auto ratio = m_blueDotPixmap.devicePixelRatioF();
        const QPointF blueDotPos = textRect.topLeft() + QPoint(-m_blueDotPixmap.width() / ratio - marginRight,
                                                               (fm.height() - m_blueDotPixmap.height() / ratio) / 2);
        painter->drawPixmap(blueDotPos, m_blueDotPixmap);
    }
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}

const QRect AppItemDelegate::itemBoundingRect(const QRect &itemRect) const
{
    const int w = itemRect.width();
    const int h = itemRect.height();
    const int sub = qAbs((w - h) / 2);

    if (w == h)
        return itemRect;
    else if (w > h)
        return itemRect - QMargins(sub, 0, sub, 0);
    else
        return itemRect - QMargins(0, 0, 0, sub * 2);
}

const QRect AppItemDelegate::itemTextRect(const QRect &boundingRect, const QRect &iconRect, const bool extraWidthMargin) const
{
    QRect result = boundingRect;
    //名字宽度需要考虑到未打开应用的小蓝点标识
    if(extraWidthMargin) result.setWidth(result.width() + m_blueDotPixmap.width());
    result.setTop(iconRect.bottom());
    return result;
}

const QPair<QString, bool> AppItemDelegate::holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect) const
{
    const int textFlag = Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap;

    if (rect.contains(fm.boundingRect(rect, textFlag, text)))
        return QPair<QString, bool>(text, true);

    QFontMetrics ftm(fm);
    QString txt = ftm.elidedText(text, Qt::ElideRight, rect.width());
    return QPair<QString, bool>(txt, false);
}

/** 绘制应用文件夹
 * @brief AppItemDelegate::drawAppDrawer
 * @param painter 绘画师
 * @param index 应用模型索引
 * @param boundingRect 应用的边界矩形
 */
void AppItemDelegate::drawAppDrawer(QPainter *painter, const QModelIndex &index, QRect iconRect) const
{
    const bool itemIsDir = index.data(AppsListModel::ItemIsDirRole).toBool();
    const ItemInfoList_v1 itemList = index.data(AppsListModel::DirItemInfoRole).value<ItemInfoList_v1>();
    const QPixmap iconPix = index.data(AppsListModel::AppIconRole).value<QPixmap>();

    // 读数据配置应用文件夹效果
    QRect AppdrawerRect = QRect(iconRect.topLeft(), iconRect.size());
    if (itemIsDir && !itemList.isEmpty()) {
        QList<QPixmap> pixmapList = index.data(AppsListModel::DirAppIconsRole).value<QList<QPixmap>>();
        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(93, 92, 90, 100));
        painter->drawRoundedRect(AppdrawerRect, RECT_REDIUS, RECT_REDIUS);
        // 绘制文件夹内其他应用
        // designer: show max to 4 icons
        const int iconCount = qMin(4, itemList.size());
        for (int i = 0; i < iconCount; i++) {
            QPixmap itemPix = iconPix;
            if (i < pixmapList.size())
                itemPix = pixmapList.at(i);

            QRect sourceRect = appSourceRect(AppdrawerRect, i);
            painter->drawPixmap(sourceRect, itemPix, itemPix.rect());
        }
        return;
    }

//    Issue: https://github.com/linuxdeepin/developer-center/issues/3993
//    实现有问题，暂时屏蔽以免拖放后出现异常文件夹框框
//
//    // 应用文件夹特效, 在当前item上且仅给当前的item 绘制文件夹样式
//    if (m_dropIndex.isValid() && (index == m_dropIndex)) {
//        painter->setPen(Qt::transparent);
//        painter->setBrush(QColor(93, 92, 90, 100));
//        painter->drawRoundedRect(AppdrawerRect, RECT_REDIUS, RECT_REDIUS);
//    }
}

/**
 * @brief 计算抽屉图标中每个小图标的位置
 *
 * 抽屉图标中的图标，总行数与总列数总是相同。
 *
 * @param rect 应用抽屉的大小
 * @param index 抽屉内的第几个应用，首个为 0。
 * @return 矩形位置
 */
QRect AppItemDelegate::appSourceRect(QRect rect, int index) const
{
    constexpr int margin = 4;
    constexpr int gutter = 4;
    constexpr int iconPerRow = 2;
    constexpr int spaces = margin * 2 + (iconPerRow - 1) * gutter;
    int iconWidth = (rect.width() - spaces) / iconPerRow;
    int iconHeight = (rect.height() - spaces) / iconPerRow;
    int currentRow = index / iconPerRow;
    int currentColumn = index % iconPerRow;
    QPoint offset(margin + currentColumn * gutter + currentColumn * iconWidth,
                  margin + currentRow * gutter + currentRow * iconHeight);

    return QRect(rect.topLeft() + offset, QSize(iconWidth, iconHeight));
}
