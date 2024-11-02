/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KPROGRESSBAR_H__
#define __KPROGRESSBAR_H__

#include <QWidget>
#include <QPaintEvent>
#include <QColor>

class KProgressBar : public QWidget
{
    Q_OBJECT
public:
    enum Corner {
        TopLeftCorner = 0x00001,
        TopRightCorner = 0x00002,
        BottomLeftCorner = 0x00004,
        BottomRightCorner = 0x00008
    };
    Q_DECLARE_FLAGS(Corners, Corner)
    typedef enum PROG_STATUS_e{
      STATUS_NORMAL = 0,
      STATUS_HIGH,
      STATUS_VHIGH
    }PROG_STATUS;
    explicit KProgressBar(QWidget *parent = nullptr, QString barType = "");

    void setMaxValue(qreal rMaxValue);
    void setCurValue(qreal rCurValue);
    void setThresholdValue(qreal rHigh, qreal rVHigh);
    void setColorAttr(QColor colorNormal, QColor colorLeft, QColor colorHigh, QColor colorVHigh);
    qreal getCurValue();
    qreal getMaxValue();
    PROG_STATUS getCurStatus() {
        return m_curStatus;
    }

protected:
    void paintEvent(QPaintEvent *event);
    void drawRoundedRect(QPainter *pa, const QRect &rect,
                       qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode = Qt::AbsoluteSize) const;

private:
    qreal m_rMaxValue;
    qreal m_rCurValue;
    qreal m_rThresholdHigh;
    qreal m_rThresholdVHigh;
    QColor m_colorNormal;
    QColor m_colorHigh;
    QColor m_colorVHigh;
    QColor m_colorLeft;
    PROG_STATUS m_curStatus;
    QString m_barType;
};

#endif // __KPROGRESSBAR_H__
