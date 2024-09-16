// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INPUTSELECTIONHANDLE_H
#define INPUTSELECTIONHANDLE_H

#include "global.h"

#include <QRasterWindow>
#include <QImage>

QT_BEGIN_NAMESPACE
class QWindow;
class QMouseEvent;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DDesktopInputSelectionControl;
class DInputSelectionHandle : public QRasterWindow
{
    Q_OBJECT

public:
    enum HandlePosition {
        Up = 0, //handle位于选中区域的上方
        Down = 1 // handle位于选中区域的下方
    };

    DInputSelectionHandle(HandlePosition position, DDesktopInputSelectionControl *pControl);

    HandlePosition handlePosition() const;
    void setHandlePosition(HandlePosition position);
    QSize handleImageSize() const;

protected:
    void paintEvent(QPaintEvent *pe) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateImage(HandlePosition position);

private:
    HandlePosition m_position;
    DDesktopInputSelectionControl *m_pInputSelectionControl;
    QImage m_image;
};

DPP_END_NAMESPACE

#endif // INPUTSELECTIONHANDLE_H
