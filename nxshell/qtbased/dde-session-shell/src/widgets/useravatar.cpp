// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "useravatar.h"
#include "dthememanager.h"
#include <QUrl>
#include <QFile>
#include <QPainterPath>

#define AVATAR_ROUND_RADIUS (18)

UserAvatar::UserAvatar(QWidget *parent)
    : QPushButton(parent)
    , m_iconLabel(new QLabel(this))
    , m_borderColor(Qt::white)
    , m_avatarSize(90)
    , m_borderWidth(0)
    , m_selected(false)
{
    setGeometry(0, 0, AvatarLargeSize, AvatarLargeSize);

    setCheckable(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_iconLabel->setObjectName("UserAvatar");
    m_iconLabel->setAccessibleName("UserAvatar");

    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
}

void UserAvatar::setIcon(const QString &iconPath)
{
    QUrl url(iconPath);
    if (url.isLocalFile()) {
        m_iconPath = url.path();
    } else {
        m_iconPath = iconPath;
    }
}

void UserAvatar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect roundedRect((width() - m_avatarSize) / 2, (height() - m_avatarSize) / 2, m_avatarSize, m_avatarSize);
    QPainterPath path;
    path.addRoundedRect(roundedRect, AVATAR_ROUND_RADIUS, AVATAR_ROUND_RADIUS);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setClipPath(path);

    const auto ratio = devicePixelRatioF();
    QString imgPath = m_iconPath;
    if (ratio > 1.0)
        imgPath.replace("icons/", "icons/bigger/");
    if (!QFile(imgPath).exists())
        imgPath = m_iconPath;

    QImage tmpImg(imgPath);

    painter.drawImage(roundedRect, this->isEnabled() ? tmpImg : imageToGray(tmpImg));

    QColor penColor = m_selected ? m_borderSelectedColor : m_borderColor;

    if (m_borderWidth) {
        QPen pen;
        pen.setColor(penColor);
        pen.setWidth(m_borderWidth);
        painter.setPen(pen);
        painter.drawPath(path);
        painter.end();
    }
}

QImage UserAvatar::imageToGray(const QImage &image)
{
    int height = image.height();
    int width = image.width();
    QImage targetImg(width, height, QImage::Format_Indexed8);
    targetImg.setColorCount(256);
    for(int i = 0; i < 256; i++)
        targetImg.setColor(i, qRgb(i, i, i));

    switch (image.format()) {
    case QImage::Format_Indexed8:
        for (int i = 0; i < height; i++) {
            const uchar *pSrc = static_cast<const uchar *>(image.constScanLine(i));
            uchar *pDest = static_cast<uchar *>(targetImg.scanLine(i));
            memcpy(pDest, pSrc, static_cast<size_t>(width));
        }
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        for (int i = 0; i < height; i++) {
            const QRgb *pSrc = reinterpret_cast<const QRgb *>(image.constScanLine(i));
            uchar *pDest = targetImg.scanLine(i);

            for (int j = 0; j < width; j++) {
                pDest[j] = static_cast<uchar>(qGray(pSrc[j]));
            }
        }
        break;
    default:
        break;
    }
    return targetImg;
}

void UserAvatar::setAvatarSize(const int size)
{
    if (size == m_avatarSize)
        return;

    m_avatarSize = size;
    setMinimumSize(size, size);
}

void UserAvatar::setDisabled(bool disable)
{
    setEnabled(!disable);

    update();
}

void UserAvatar::setSelected(bool selected)
{
    m_selected = selected;

    update();
}

int UserAvatar::borderWidth() const
{
    return m_borderWidth;
}

void UserAvatar::setBorderWidth(int borderWidth)
{
    m_borderWidth = borderWidth;
}

QColor UserAvatar::borderSelectedColor() const
{
    return m_borderSelectedColor;
}

void UserAvatar::setBorderSelectedColor(const QColor &borderSelectedColor)
{
    m_borderSelectedColor = borderSelectedColor;
}

QColor UserAvatar::borderColor() const
{
    return m_borderColor;
}

void UserAvatar::setBorderColor(const QColor &borderColor)
{
    m_borderColor = borderColor;
}
