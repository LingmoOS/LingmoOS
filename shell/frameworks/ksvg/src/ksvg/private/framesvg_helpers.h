/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSVG_FRAMESVG_HELPERS_H
#define KSVG_FRAMESVG_HELPERS_H

#include "framesvg.h"

namespace KSvg
{
namespace FrameSvgHelpers
{
/**
 * @returns the element id name for said @p borders
 */
QString borderToElementId(FrameSvg::EnabledBorders borders)
{
    if (borders == FrameSvg::NoBorder) {
        return QStringLiteral("center");
    } else if (borders == FrameSvg::TopBorder) {
        return QStringLiteral("top");
    } else if (borders == FrameSvg::BottomBorder) {
        return QStringLiteral("bottom");
    } else if (borders == FrameSvg::LeftBorder) {
        return QStringLiteral("left");
    } else if (borders == FrameSvg::RightBorder) {
        return QStringLiteral("right");
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::LeftBorder)) {
        return QStringLiteral("topleft");
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::RightBorder)) {
        return QStringLiteral("topright");
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::LeftBorder)) {
        return QStringLiteral("bottomleft");
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::RightBorder)) {
        return QStringLiteral("bottomright");
    } else {
        qWarning() << "unrecognized border" << borders;
    }
    return QString();
}

/**
 * @returns the suggested geometry for the @p borders given a @p fullSize frame size and a @p contentRect
 */
QRectF sectionRect(KSvg::FrameSvg::EnabledBorders borders, const QRectF &contentRect, const QSizeF &fullSize)
{
    // don't use QRect corner methods here, they have semantics that might come as unexpected.
    // prefer constructing the points explicitly. e.g. from QRect::topRight docs:
    // Note that for historical reasons this function returns QPoint(left() + width() -1, top()).

    if (borders == FrameSvg::NoBorder) {
        return contentRect;
    } else if (borders == FrameSvg::TopBorder) {
        return QRectF(QPointF(contentRect.left(), 0), QSizeF(contentRect.width(), contentRect.top()));
    } else if (borders == FrameSvg::BottomBorder) {
        return QRectF(QPointF(contentRect.left(), contentRect.bottom()), QSizeF(contentRect.width(), fullSize.height() - contentRect.bottom()));
    } else if (borders == FrameSvg::LeftBorder) {
        return QRectF(QPointF(0, contentRect.top()), QSizeF(contentRect.left(), contentRect.height()));
    } else if (borders == FrameSvg::RightBorder) {
        return QRectF(QPointF(contentRect.right(), contentRect.top()), QSizeF(fullSize.width() - contentRect.right(), contentRect.height()));
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::LeftBorder)) {
        return QRectF(QPointF(0, 0), QSizeF(contentRect.left(), contentRect.top()));
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::RightBorder)) {
        return QRectF(QPointF(contentRect.right(), 0), QSizeF(fullSize.width() - contentRect.right(), contentRect.top()));
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::LeftBorder)) {
        return QRectF(QPointF(0, contentRect.bottom()), QSizeF(contentRect.left(), fullSize.height() - contentRect.bottom()));
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::RightBorder)) {
        return QRectF(QPointF(contentRect.right(), contentRect.bottom()),
                      QSizeF(fullSize.width() - contentRect.right(), fullSize.height() - contentRect.bottom()));
    } else {
        qWarning() << "unrecognized border" << borders;
    }
    return QRectF();
}

}

}

#endif
