// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistdelegate.h"
#include "appslistmodel.h"
#include "calculate_util.h"
#include "util.h"
#include "iteminfo.h"

#include <DGuiApplicationHelper>

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

AppListDelegate::AppListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),
      m_actived(false),
      m_calcUtil(CalculateUtil::instance())
{
    m_blueDotPixmap = renderSVG(":/skin/images/new_install_indicator.svg", QSize(10, 10));
    m_autoStartPixmap = renderSVG(":/skin/images/emblem-autostart.svg", QSize(16, 16));
}

void AppListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(AppsListModel::AppItemIsDraggingRole).value<bool>() &&
            !(option.features & QStyleOptionViewItem::HasDisplay)) {
        return;
    }

    if (!m_actived)
        return;

    const qreal ratio = qApp->devicePixelRatio();
    const QRect rect = option.rect;
    const bool isAlternate = option.features & QStyleOptionViewItem::Alternate;
    const QPixmap iconPixmap = index.data(AppsListModel::AppListIconRole).value<QPixmap>();
    const bool isTitle = index.data(AppsListModel::AppItemTitleRole).toBool();

    painter->setPen(Qt::NoPen);

    if (option.state.testFlag(QStyle::State_Selected) && !isTitle) {
        QColor brushColor;
        if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType())
            brushColor = Qt::white;
        else
            brushColor = Qt::black;

        brushColor.setAlpha(25);
        painter->setBrush(brushColor);
    } else if (isAlternate && !isTitle) {
        QColor brushColor(Qt::white);
        brushColor.setAlpha(static_cast<int>(255 * 0.4));
        painter->setBrush(brushColor);

        QColor penColor(Qt::black);
        penColor.setAlpha(static_cast<int>(255 * 0.05));
        painter->setPen(penColor);
    } else {
        painter->setBrush(Qt::NoBrush);
    }

    if (index.data(AppsListModel::DrawBackgroundRole).toBool())
        painter->drawRoundedRect(option.rect.marginsRemoved(QMargins(15, 0, 0, 0)), 8, 8);

    const int iconX = rect.x() + 20;
    const int iconY = qRound(rect.y() + (rect.height() - iconPixmap.height() / ratio) / 2);

    if (!isTitle)
        painter->drawPixmap(iconX, iconY, iconPixmap);

    // draw icon if app is auto startup
    if (index.data(AppsListModel::AppAutoStartRole).toBool()) {
        painter->drawPixmap(iconX, iconY + 16, m_autoStartPixmap);
    }

    QRect textRect = rect.marginsRemoved(QMargins(20, 1, 1, 1));
    if (!isTitle)
        textRect = rect.marginsRemoved(QMargins(60, 1, 1, 1)); // 图标 + 间隔,再显示文字

    textRect.setWidth(qRound(textRect.width() - m_blueDotPixmap.width() / ratio));
    painter->setPen(QPen(QPalette().brightText(), 1));

    const QString &appName = painter->fontMetrics().elidedText(index.data(AppsListModel::AppNameRole).toString(), Qt::ElideRight, textRect.width());

    if (isTitle) {
        painter->setOpacity(0.5);
        painter->setFont(QFont(painter->font().family(), DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8)));
    } else {
        painter->setOpacity(1);
        painter->setFont(QFont(painter->font().family(), DLauncher::DEFAULT_FONT_SIZE));
    }

    if (isAlternate) {
        QFont nameFont = painter->font();
        nameFont.setPixelSize(nameFont.pixelSize() + 2);
        painter->setFont(nameFont);
    }

    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, appName);
    painter->setOpacity(1);

    // draw blue dot if needed
    if (index.data(AppsListModel::AppNewInstallRole).toBool() && !isAlternate && !isTitle) {
        const QPointF blueDotPos(rect.width() - m_blueDotPixmap.width() / ratio - 6,
                                 rect.y() + (+ rect.height() - m_blueDotPixmap.height() / ratio) / 2);

        painter->drawPixmap(blueDotPos, m_blueDotPixmap);
    }
}

QSize AppListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);


    return QSize(0, 36);
}

void AppListDelegate::setActived(bool active)
{
    m_actived = active;
}

QPixmap AppListDelegate::dropShadow(QPixmap pixmap, int radius, const QColor &color, const QPoint &offset)
{
    if (pixmap.isNull()) {
        return QPixmap();
    }

    pixmap.setDevicePixelRatio(1);
    QImage temp(pixmap.size() + QSize(radius * 2, radius * 2),
                QImage::Format_ARGB32_Premultiplied);
    temp.fill(0);

    QPainter tempPainter(&temp);
    tempPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tempPainter.drawPixmap(QPoint(radius, radius), pixmap);
    tempPainter.end();

    QImage blurred(temp.size(),
                   QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);

    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, temp, radius, false, true);
    blurPainter.end();

    temp = blurred;

    tempPainter.begin(&temp);
    tempPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tempPainter.fillRect(temp.rect(), color);
    tempPainter.end();

    QPainter pa(&temp);
    pa.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pa.drawPixmap(radius - offset.x(), radius - offset.y(), pixmap);
    pa.end();

    return QPixmap::fromImage(temp);
}
