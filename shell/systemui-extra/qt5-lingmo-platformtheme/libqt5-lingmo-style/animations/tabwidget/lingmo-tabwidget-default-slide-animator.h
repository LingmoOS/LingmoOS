/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef LINGMOTABWIDGETDEFAULTSLIDEANIMATOR_H
#define LINGMOTABWIDGETDEFAULTSLIDEANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QStackedWidget>
#include "lingmo-tabwidget-animator-iface.h"

#include <QPixmap>

namespace LINGMO {

namespace TabWidget {

/*!
 * \brief The DefaultSlideAnimator class
 * \details
 * This class is an implement of LINGMOTabWidgetAnimatorIface.
 */
class DefaultSlideAnimator : public QVariantAnimation, public LINGMOTabWidgetAnimatorIface
{
    Q_OBJECT
public:
    explicit DefaultSlideAnimator(QObject *parent = nullptr);
    ~DefaultSlideAnimator();

    bool bindTabWidget(QTabWidget *w);
    bool unboundTabWidget();
    QWidget *boundedWidget() {return m_bound_widget;}

    bool eventFilter(QObject *obj, QEvent *e);

protected:
    void watchSubPage(QWidget *w);

    bool filterTabWidget(QObject *obj, QEvent *e);
    bool filterStackedWidget(QObject *obj, QEvent *e);
    bool filterSubPage(QObject *obj, QEvent *e);
    bool filterTmpPage(QObject *obj, QEvent *e);

    void clearPixmap();

private:
    QTabWidget *m_bound_widget = nullptr;
    QStackedWidget *m_stack = nullptr;
    QList<QWidget *> m_children;

    QPixmap m_previous_pixmap;
    QPixmap m_next_pixmap;

    /*!
     * \brief m_tmp_page
     * \note
     * insert a tmp tab page into tab widget directly is dangerous,
     * because a custom tab widget's page may be desgined different
     * with normal tab page, such as explor-qt's directory view.
     * In that case, it might lead program crashed when
     * application call a custom page but get a tmp page.
     *
     * for those reasons, i use a temporary widgets bound to the
     * stacked widget with qt's parent&child mechanism.
     * It can float on the top layer or hide on the lower layer of stack,
     * but it does not belong to the elements in the stack (no index),
     * which can avoid the above problems.
     *
     * However, this way might be incompatible with other animations.
     * Because it uses a new widget for painting, not relate with orignal
     * page. Another conflict is the oxygen's fade animation might force
     * raise current tab page when it finished. That might cause a incompleted
     * slide animation if slide duration is longer than fade's.
     */
    QWidget *m_tmp_page = nullptr;

    /*!
     * \brief m_tab_resizing
     * \details
     * If a went to a resize event, the animation should handle
     * widget's relayout after resized.
     * This bool varient is used to help judege if animator's pixmaps
     * and template widget' states should be updated.
     */
    bool m_tab_resizing = false;
    int pervIndex = -1;
    bool left_right = true;
    bool horizontal  = false;
    QWidget *previous_widget = nullptr;
};

}

}

#endif // LINGMOTABWIDGETDEFAULTSLIDEANIMATOR_H
