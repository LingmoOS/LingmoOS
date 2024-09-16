// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define private public
#include <QFont>
#undef private

#include "dfontmanager.h"
#include "dfontmanager_p.h"

#include <private/qfont_p.h>

#include <cmath>

DGUI_BEGIN_NAMESPACE

DFontManagerPrivate::DFontManagerPrivate(DFontManager *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
    baseFont.setPixelSize(fontPixelSize[baseFontSizeType]);
}

/*!
  \class Dtk::Gui::DFontManager
  \inmodule dtkgui
  \brief 字体大小设置的一个类,系统默认只设置T6.
 */

DFontManager::DFontManager(QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DFontManagerPrivate(this))
{
}

DFontManager::~DFontManager()
{
}

/*!
  \enum Dtk::Gui::DFontManager::SizeType
  DFontManager::SizeType 定义了 DFontManager 的系统字体的定义的大小; 而系统只会设置 T6 为系统默认的字体
  \value T1
  系统级别为 T1 的字体大小, 默认是40 px
  \value T2
  系统级别为 T2 的字体大小, 默认是30 px
  \value T3
  系统级别为 T3 的字体大小, 默认是24 px
  \value T4
  系统级别为 T4 的字体大小, 默认是20 px
  \value T5
  系统级别为 T5 的字体大小, 默认是17 px
  \value T6
  系统级别为 T6 的字体大小, 默认是14 px
  \value T7
  系统级别为 T7 的字体大小, 默认是13 px
  \value T8
  系统级别为 T8 的字体大小, 默认是12 px
  \value T9
  系统级别为 T9 的字体大小, 默认是11 px
  \value T10
  系统级别为 T10 的字体大小, 默认是10 px

  \omitvalue NSizeTypes
 */

/*!
  \brief 获取字体像素的大小.

  \a type 字体枚举类型
  \return 返回字体像素的大小
 */
int DFontManager::fontPixelSize(DFontManager::SizeType type) const
{
    D_DC(DFontManager);

    if (type >= NSizeTypes) {
        return 0;
    }

    return d->fontPixelSize[type] + d->fontPixelSizeDiff;
}

/*!
  \brief 设置字体像素大小.

  \a type 字体枚举类型
  \a size 字体大小
 */
void DFontManager::setFontPixelSize(DFontManager::SizeType type, int size)
{
    D_D(DFontManager);

    if (type >= NSizeTypes) {
        return;
    }

    if (d->fontPixelSize[type] == size) {
        return;
    }

    d->fontPixelSize[type] = size;
}

/*!
  \brief 设置字体的通用像素大小.

  \a font 预设计的字体像素的大小
 */
void DFontManager::setBaseFont(const QFont &font)
{
    D_D(DFontManager);

    if (d->baseFont == font)
        return;

    d->baseFont = font;
    d->fontPixelSizeDiff = fontPixelSize(font) - d->fontPixelSize[d->baseFontSizeType];

    Q_EMIT fontChanged();
}

QFont DFontManager::baseFont() const
{
    D_DC(DFontManager);

    return d->baseFont;
}

void DFontManager::resetBaseFont()
{
    D_DC(DFontManager);

    QFont font;
    font.setPixelSize(d->fontPixelSize[d->baseFontSizeType]);
    setBaseFont(font);
}

/*!
  \brief 获取字体.

  \a pixelSize 字体的像素大小
  \a base 要基于的字体
  \return 返回设置字体大小后的字体
 */
QFont DFontManager::get(int pixelSize, const QFont &base)
{
    QFont font = base;
    font.setPixelSize(pixelSize);
    return font;
}

int DFontManager::fontPixelSize(const QFont &font)
{
    int px = font.pixelSize();

    if (px == -1) {
        px = qRound(std::floor(((font.pointSizeF() * font.d->dpi) / 72) * 100 + 0.5) / 100);
    }

    return px;
}

DGUI_END_NAMESPACE
