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

#ifndef __KHEADER_VIEW_H__
#define __KHEADER_VIEW_H__

#include <QHeaderView>

class QModelIndex;

/**
 * @brief Customized HeaderView
 */
class KHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    /**
     * @brief KHeaderView constructor
     * @param orientation Header orientation
     * @param parent Parent Widget
     */
    explicit KHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief sizeHint Hint size of this widget
     * @return Hint size
     */
    QSize sizeHint() const override;
    /**
     * @brief sectionSizeHint Hint size of section
     * @param logicalIndex Section logical index
     * @return Hint size in pixels
     */
    int sectionSizeHint(int logicalIndex) const;

    /**
     * @brief getSpacing Get section spacing
     * @return Spacing in pixels
     */
    inline int getSpacing() const { return m_spacing; }
    /**
     * @brief setSpacing Set section spacing
     * @param spacing Spacing in pixels
     */
    inline void setSpacing(int spacing) { m_spacing = spacing; }

    QPoint getMousePressPoint();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *e);
    void mouseReleaseEvent(QMouseEvent * e);
private:
    // spacing between sections
    int m_spacing {1};
    QMap<int, QString>tooltipTextMap;
    QPoint pressPoint{0,0};
};

#endif  // __KHEADER_VIEW_H__
