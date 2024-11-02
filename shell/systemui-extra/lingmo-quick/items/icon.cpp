/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "icon.h"
#include "icon-helper.h"
#include "icon-provider.h"
#include "theme-icon.h"

#include <QHash>
#include <QIcon>
#include <QImage>
#include <QBitmap>
#include <QPainter>
#include <QtMath>
#include <QSGTexture>
#include <QSGImageNode>
#include <QQuickWindow>

namespace LingmoUIQuick {

class IconPrivate
{
public:
    int radius {0};
    bool textureChanged {true};
    QIcon icon;
    Icon::Mode mode {Icon::Normal};
    QString          pointText {""};
    Types::Pos    pointPos {Types::TopRight};
    Icon::PointType  pointType {Icon::Null};
    Theme::ColorRole pointColor {Theme::Highlight};
    Theme::ColorRole highlightColor {Theme::HighlightedText};

    std::shared_ptr<QSGTexture> currentTexture;
    static QHash<QQuickWindow*, QHash<qint64, std::weak_ptr<QSGTexture> > > textureCache;
};

QHash<QQuickWindow*, QHash<qint64, std::weak_ptr<QSGTexture> > > IconPrivate::textureCache = {};

// ===== ICON ====== //
Icon::Icon(QQuickItem *parent) : QQuickItem(parent), d(new IconPrivate)
{
    setFlag(QQuickItem::ItemHasContents);

    connect(this, &QQuickItem::widthChanged, this, &Icon::onSizeChanged);
    connect(this, &QQuickItem::heightChanged, this, &Icon::onSizeChanged);
    connect(Theme::instance(), &Theme::iconThemeChanged, this, &Icon::markTextureChanged);
}

Icon::~Icon()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

// ====== DRAW ====== //
QSGNode *Icon::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    QSGImageNode *imageNode = nullptr;
    if (node) {
        imageNode = static_cast<QSGImageNode *>(node);
    } else {
        imageNode = window()->createImageNode();
        imageNode->setOwnsTexture(true);
        imageNode->setFiltering(QSGTexture::Linear);
        node = imageNode;
    }

    if (d->textureChanged) {
        imageNode->setTexture(createTexture());
        imageNode->setSourceRect({QPoint(0, 0), imageNode->texture()->textureSize()});
        d->textureChanged = false;
    }

    imageNode->setRect(boundingRect());
    return node;
}

QSGTexture *Icon::createTexture()
{
    QImage image;
    // 获取原始图像
    if (d->icon.isNull()) {
        QPixmap pixmap(size().toSize());
        pixmap.fill(Qt::transparent);
        image = pixmap.toImage();
    } else {
        image = d->icon.pixmap(size().toSize() * window()->devicePixelRatio()).toImage();
    }

//    // 缩放大图像？
//    QSize imageSize = image.size();
//    if (imageSize.width() > 512 || imageSize.height() > 512) {
//        image.scaled();
//        imageSize = image.size();
//    }

    // 修改图像
    //imageCorrection(image);

    if (d->mode & Icon::Disabled) {
        // use style
        image = QIcon(QPixmap::fromImage(std::move(image))).pixmap(size().toSize(), QIcon::Disabled).toImage();
    } else if (d->mode & Icon::Highlight) {
        bool isPureColor = true;
        if(!d->mode.testFlag(Icon::ForceHighlight)) {
            // TODO: 更新判断纯色算法
            isPureColor = ThemeIcon::isPixmapPureColor(QPixmap::fromImage(image));
        }
        if (isPureColor) {
            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(image.rect(), Theme::instance()->color(d->highlightColor));
        } else {
            QColor baseColor = Theme::instance()->color(d->highlightColor);
            QColor symbolicColor = ThemeIcon::getSymbolicColor();

            for (int i = 0; i < image.width(); i++) {
                for (int j = 0; j < image.height(); j++) {
                    auto color = image.pixelColor(i, j);
                    if (color.alpha() > 0.3) {
                        if (qAbs(color.red() - symbolicColor.red()) < 10 && qAbs(color.green() - symbolicColor.green()) < 10
                                && qAbs(color.blue() - symbolicColor.blue()) < 10) {
                            color.setRed(baseColor.red());
                            color.setGreen(baseColor.green());
                            color.setBlue(baseColor.blue());
                            image.setPixelColor(i, j, color);
                        }
                    }
                }
            }
        }
    }

    //std::shared_ptr<QSGTexture> texturePtr;
    //qint64 cacheKey = image.cacheKey();
    //auto hash = IconPrivate::textureCache[window()];
    //if (hash.contains(cacheKey)) {
    //    texturePtr = hash[cacheKey].lock();
    //    if (texturePtr) {
    //        d->currentTexture = texturePtr;
    //        return d->currentTexture.get();
    //    }
    //}

    QSGTexture *texture = window()->createTextureFromImage(image);
    //texturePtr = std::shared_ptr<QSGTexture>(texture, [this] (QSGTexture *texture) {
    //    //hash.remove(cacheKey);
    //    //if (hash.isEmpty()) {
    //    //    IconPrivate::textureCache.remove(window());
    //    //}
    //
    //    delete texture;
    //});

    //hash[cacheKey] = texturePtr;
    //d->currentTexture = texturePtr;

    return texture;
}

void Icon::imageCorrection(QImage &image)
{
    // 画点
    drawPoint(image);
    // 修改状态
}

QPoint Icon::posPoint(const QSize &imageSize, const QSize &pointSize) const
{
    QPoint point;
    switch (d->pointPos) {
        default:
        case Types::TopRight:
            point = {imageSize.width() - pointSize.width(), 0};
            break;
        case Types::TopLeft:
            point = {0, 0};
            break;
        case Types::BottomRight:
            point = {imageSize.width() - pointSize.width(), imageSize.height() - pointSize.height()};
            break;
        case Types::BottomLeft:
            point = {0, imageSize.height() - pointSize.height()};
            break;
        case Types::Center:
            point = {(imageSize.width() - pointSize.width()) / 2, (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::LeftCenter:
            point = {0, (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::TopCenter:
            point = {(imageSize.width() - pointSize.width()) / 2, 0};
            break;
        case Types::RightCenter:
            point = {imageSize.width() - pointSize.width(), (imageSize.height() - pointSize.height()) / 2};
            break;
        case Types::BottomCenter:
            point = {(imageSize.width() - pointSize.width()) / 2, imageSize.height() - pointSize.height()};
            break;
    }

    if (point.x() < 0) {
        point.setX(0);
    } else if ((point.x() + pointSize.width()) > imageSize.width()) {
        point.setX(imageSize.width() - pointSize.width());
    }

    if (point.y() < 0) {
        point.setY(0);
    } else if ((point.y() + pointSize.height()) > imageSize.height()) {
        point.setY(imageSize.height() - pointSize.height());
    }

    return point;
}

void Icon::drawPoint(QImage &image)
{
    if (d->pointType == Icon::Null) {
        return;
    }

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setBrush(Theme::instance()->color(d->pointColor));
    painter.setPen(Theme::instance()->color(d->pointColor));

    if (d->pointType == Icon::Point) {
        QSize pointSize(qFloor(image.width() * 0.35), qFloor(image.height() * 0.35));
        QRect rect(posPoint(image.size(), pointSize), pointSize);

        painter.drawEllipse(rect);

    } else if (d->pointType == Icon::Text) {
        QRect rect = {0, 0, qFloor(image.width() * 0.4), qFloor(image.height() * 0.4)};

        QFont font = painter.font();
        // 为了将两位数正常显示在框框中
        font.setPointSize(qFloor(6.0 * rect.width() / 12.0));
        font.setBold(true);

        QString text = d->pointText;
        QFontMetrics fontMetrics(font);
        QRect textRect = fontMetrics.boundingRect(rect, Qt::AlignVCenter | Qt::AlignHCenter, text);
        if (textRect.width() > rect.width()) {
            if (textRect.width() > image.width()) {
                rect.setWidth(image.width());
                text = fontMetrics.elidedText(text, Qt::ElideRight, rect.width() - 4);
            } else {
                rect.setWidth(textRect.width() + 8);
            }
        }

        rect.moveTopLeft(posPoint(image.size(), rect.size()));

        // 底部圆
        painter.save();
        painter.setPen(Theme::instance()->colorWithCustomTransparency(Theme::BrightText, Theme::Active, 0.15));
        painter.drawRoundedRect(rect, qFloor(rect.height() * 0.5), qFloor(rect.height() * 0.5));
        painter.restore();

        // draw text.
        painter.save();
        // TODO: 优化字体显示
        painter.setFont(font);
        painter.setPen(Theme::instance()->buttonText());
        painter.drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, text);
        painter.restore();
    }
}

void Icon::onSizeChanged()
{
    markTextureChanged();
}

// ====== METHODS ====== //
QVariant Icon::source() const
{
    return d->icon;
}

void Icon::setSource(const QVariant &source)
{
    switch (source.userType()) {
        case QMetaType::QIcon:
            d->icon = source.value<QIcon>();
            break;
        case QMetaType::QImage:
            d->icon.addPixmap(QPixmap::fromImage(source.value<QImage>()));
            break;
        case QMetaType::QPixmap:
            d->icon.addPixmap(source.value<QPixmap>());
            break;
        case QMetaType::QBitmap:
            d->icon.addPixmap(source.value<QBitmap>());
            break;
        case QMetaType::QString: {
            d->icon = IconHelper::loadIcon(source.value<QString>());
            break;
        }
        default:
            break;
    }

    if (!d->icon.isNull()) {
        markTextureChanged();
        Q_EMIT sourceChanged();
    }
}

Icon::Mode Icon::mode() const
{
    return d->mode;
}

void Icon::setMode(const Icon::Mode &mode)
{
    if (d->mode == mode) {
        return;
    }

    d->mode = mode;
    if (d->mode & Icon::AutoHighlight) {
        updateMode();
        connect(Theme::instance(), &Theme::themeNameChanged, this, &Icon::updateMode);
    } else {
        disconnect(Theme::instance(), nullptr, this, nullptr);
        markTextureChanged();
    }

    Q_EMIT modeChanged();
}

Theme::ColorRole Icon::highlightColor() const
{
    return d->highlightColor;
}

void Icon::setHighlightColor(const Theme::ColorRole &color)
{
    if (d->highlightColor == color) {
        return;
    }

    d->highlightColor = color;
    markTextureChanged();
    Q_EMIT highlightColorChanged();
}

int Icon::radius() const
{
    return d->radius;
}

void Icon::setRadius(int radius)
{
    if (d->radius == radius) {
        return;
    }

    d->radius = radius;
    markTextureChanged();
    Q_EMIT radiusChanged();
}

Icon::PointType Icon::pointType() const
{
    return d->pointType;
}

void Icon::setPointType(Icon::PointType type)
{
    if (d->pointType == type) {
        return;
    }

    d->pointType = type;
    markTextureChanged();
    Q_EMIT pointTypeChanged();
}

Theme::ColorRole Icon::pointColor() const
{
    return d->pointColor;
}

void Icon::setPointColor(const Theme::ColorRole &color)
{
    if (d->pointColor == color) {
        return;
    }

    d->pointColor = color;
    markTextureChanged();
    Q_EMIT pointColorChanged();
}

QString Icon::pointText() const
{
    return d->pointText;
}

void Icon::setPointText(const QString &text)
{
    if (d->pointText == text) {
        return;
    }

    d->pointText = text;
    markTextureChanged();
    Q_EMIT pointTextChanged();
}

Types::Pos Icon::pointPos() const
{
    return d->pointPos;
}

void Icon::setPointPos(Types::Pos pos)
{
    if (d->pointPos == pos) {
        return;
    }

    d->pointPos = pos;
    markTextureChanged();
    Q_EMIT pointPosChanged();
}

inline void Icon::markTextureChanged()
{
    d->textureChanged = true;
    update();
}

void Icon::updateMode()
{
    if (Theme::instance()->isDarkTheme()) {
        d->mode |= Icon::Highlight;
    } else {
        d->mode &= ~Icon::Highlight;
    }

    markTextureChanged();
}

} // LingmoUIQuick
