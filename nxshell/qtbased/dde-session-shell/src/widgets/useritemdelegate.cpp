// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "useritemdelegate.h"

#include <DFontSizeManager>

#include <QFile>
#include <QPainter>
#include <QBitmap>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

const int ITEM_HEIGHT = 64;
const int ITEM_SPACE = 10;
const int LABEL_SPACE = 2;
const int RADIUS_VALUE = 8;
const QSize IMAGE_SIZE = QSize(48, 48);

UserItemDelegate::UserItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void UserItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QPen pen;
    QRect rect = option.rect;
    // item间要留10空间的空白
    rect.setHeight(rect.height() - ITEM_SPACE);

    if (option.state.testFlag(QStyle::State_Selected)) {
        // 鼠标悬停背景色
        QColor hoverColor(0, 129, 255, int(0.9 * 255));
        pen.setColor(hoverColor);
        painter->setPen(pen);
        painter->setBrush(hoverColor);
    } else {
        // 默认背景颜色
        QColor bgColor(Qt::black);
        bgColor.setAlpha(0.05 * 255);
        pen.setColor(bgColor);
        painter->setPen(pen);
        painter->setBrush(bgColor);
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    // 绘制背景颜色
    painter->drawRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);

    UserItemData userData = index.data(StaticUserDataRole).value<UserItemData>();

    // 绘制圆角图像
    drawRoundImage(painter, rect, userData.imagePath);

    // 已登录标志
    if (userData.isLogined)
        drawLoginedState(painter, rect);

    if (option.state.testFlag(QStyle::State_Selected)) {
        pen.setColor(Qt::white);
    } else {
        pen.setColor(Qt::black);
    }
    painter->setPen(pen);

    // 绘制userName，fullName，用户类型
    drawNameAndType(painter, userData, rect);

    bool isCurrentUser = index.data(IsCurrentUserDataRole).toBool();
    if (isCurrentUser)
        drawCheckedState(painter, rect);
}

QSize UserItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    UserItemData userData = index.data(StaticUserDataRole).value<UserItemData>();
    int height = ITEM_SPACE + displayNameHeight() + LABEL_SPACE + userTypeHeight() + ITEM_SPACE;
    if (!userData.name.isEmpty()) {
        height += nameHeight() + LABEL_SPACE;
    }

    height = height > ITEM_HEIGHT ? height : ITEM_HEIGHT;

    return QSize(option.rect.width(), height);
}

void UserItemDelegate::drawRoundImage(QPainter *thisPainter, const QRect &rect, const QString &path) const
{
    if (path.isEmpty() || !QFile::exists(path))
        return;

    // 设计图上常量
    int margTop = 0;
    const int marginLeft = 8;
    const int imageRadius = 10;

    if (rect.height() > IMAGE_SIZE.height()) {
        margTop = rect.height() / 2 - IMAGE_SIZE.height() / 2;
    }

    QRect drawRect = QRect(rect.left() + marginLeft, rect.top() + margTop,
                           IMAGE_SIZE.width(), IMAGE_SIZE.height());
    QPainterPath clipPath;
    clipPath.addRoundedRect(drawRect, imageRadius, imageRadius);
    thisPainter->save();
    thisPainter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    thisPainter->setClipPath(clipPath);
    QPixmap pixmap(path);
    pixmap = pixmap.scaled(IMAGE_SIZE);
    thisPainter->drawPixmap(drawRect, pixmap);
    thisPainter->restore();
}

void UserItemDelegate::drawLoginedState(QPainter *painter, const QRect &rect) const
{
    const int leftMargin = 50;
    const int topMargin = 50;
    const int radiusWidth = 8;

    QRect drawRect = QRect(rect.left() + leftMargin, rect.top() + topMargin,
                           radiusWidth, radiusWidth);
    QPen pen;
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->setBrush(QBrush(Qt::green));

    painter->drawEllipse(drawRect);
}

void UserItemDelegate::drawNameAndType(QPainter *painter, const UserItemData &userData, const QRect &rect) const
{
    // 文字区域设计图常量
    const int leftMargin = 65;  // 开始位置距离item最左边
    const int rightMargin = 48; // 结束位置距离item最右边
    const int topMargin = 0;

    const int itemSpacing = 10;

    int textAreaWidth = rect.width() - leftMargin - rightMargin;
    QRect displayNameRect = QRect(rect.left() + leftMargin, rect.top() + topMargin,
                                  textAreaWidth, displayNameHeight());

    QFont font = DFontSizeManager::instance()->t4();
    font.setBold(true);
    painter->setFont(font);

    // 绘制displayName, +1个像素是为了避免刚好width和文本宽度一致，最后文本还有省略号
    QString displayNameText = elidedText(userData.displayName, textAreaWidth + 1, font.pixelSize(), true);

    QTextOption qTextOption;
    qTextOption.setAlignment(Qt::AlignmentFlag::AlignVCenter);
    painter->drawText(displayNameRect, displayNameText, qTextOption);

    QFont typeFont = DFontSizeManager::instance()->t8();
    int userTypeAreaWidth = stringWidth(userData.userStrType, typeFont.pixelSize());

    // 绘制name
    int nameWidth = 0;
    if (!userData.name.isEmpty()) {
        font = DFontSizeManager::instance()->t6();
        font.setBold(false);
        painter->setFont(font);

        QString nameText = elidedText(userData.name, textAreaWidth - itemSpacing - userTypeAreaWidth, font.pixelSize());
        nameWidth = stringWidth(nameText, font.pixelSize());
        QRect nameRect = QRect(displayNameRect.left(), displayNameRect.bottom() + LABEL_SPACE,
                                   textAreaWidth - itemSpacing - userTypeAreaWidth, nameHeight());

        painter->drawText(nameRect, nameText);
    }

    typeFont.setBold(false);
    painter->setFont(typeFont);

    // 绘制userType, +1个像素为了避免字体大小不一样，绘制中心位置不一致
    int userTypeLeft = userData.name.isEmpty() ? displayNameRect.left() : displayNameRect.left() + nameWidth + itemSpacing;
    QRect userTypeRect = QRect(userTypeLeft, displayNameRect.bottom() + LABEL_SPACE + 1, userTypeAreaWidth + 1, userTypeHeight());
    qreal currentOpacity = painter->opacity();
    painter->setOpacity(0.7);
    painter->drawText(userTypeRect, userData.userStrType, qTextOption);
    painter->setOpacity(currentOpacity);
}

void UserItemDelegate::drawCheckedState(QPainter *painter, const QRect &rect) const
{
    // 绘制对勾,14x12
    const int width = 14;
    const int height = 12;
    const int marginRight = 11;

    QPen pen = painter->pen();
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(QBrush());

    int left = rect.width() - marginRight - width;
    int top = rect.top() + rect.height() / 2 - height / 2;

    QPainterPath path;
    path.moveTo(left, top + 6);
    path.lineTo(left + 4, top + 11);
    path.lineTo(left + 12, top + 1);

    painter->drawPath(path);
}

int UserItemDelegate::stringWidth(const QString &str, int fontSize, bool isBold) const
{
    if (str.isEmpty())
        return 0;

    QFont font;
    font.setBold(isBold);
    font.setPixelSize(fontSize);

    QFontMetrics fm(font);
    return fm.boundingRect(str).width();
}

QString UserItemDelegate::elidedText(const QString &originString, int width, int fontSize, bool isBold) const
{
    if (originString.isEmpty())
        return QString();

    QFont font;
    font.setBold(isBold);
    font.setPixelSize(fontSize);

    QFontMetrics fm(font);
    return fm.elidedText(originString, Qt::ElideRight, width);
}

int UserItemDelegate::displayNameHeight()
{
    QFont font = DFontSizeManager::instance()->t4();
    QFontMetrics fm(font);
    return fm.height();
}

int UserItemDelegate::nameHeight()
{
    QFont font = DFontSizeManager::instance()->t6();
    QFontMetrics fm(font);
    return fm.height();
}

int UserItemDelegate::userTypeHeight()
{
    QFont font = DFontSizeManager::instance()->t8();
    QFontMetrics fm(font);
    return fm.height();
}
