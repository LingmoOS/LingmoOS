/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTOOLTIPWIDGET_H
#define KTOOLTIPWIDGET_H

#include <kwidgetsaddons_export.h>

#include <QWidget>
#include <memory>

/**
 * @class KToolTipWidget ktooltipwidget.h KToolTipWidget
 *
 * @brief A tooltip that contains a QWidget.
 *
 * This widget allows to show a tooltip that contains another widget.
 * If you only need to show text inside the tooltip, just use QWidget::setToolTip();
 *
 * To show the tooltip, you need to choose a position on the screen and also pass a
 * transient parent window (recommended on X11 and required on Wayland).
 * Use showAt() if you want to show the tooltip from a specific point.
 * Use showUnder() if you want to show it under a given rectangle.
 *
 * You can use a single instance of this class to show as many tooltips as you want.
 *
 * The tooltip does not take ownership of the content widget if the latter already
 * has a parent. While the tooltip is set as parent of the widget (by the layout),
 * the old parent is restored when the widget is replaced by another widget
 * and also when the tooltip is deleted.
 *
 * @since 5.30
 * @author Elvis Angelaccio <elvis.angelaccio@kde.org>
 */
class KWIDGETSADDONS_EXPORT KToolTipWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int hideDelay READ hideDelay WRITE setHideDelay)

public:
    explicit KToolTipWidget(QWidget *parent = nullptr);
    ~KToolTipWidget() override;

    /**
     * Show a tooltip containing @p content. The pos() of the tooltip will be @p pos.
     * You can call this method multiple times over the same KToolTipWidget instance
     * (previously shown widgets will be removed from the tooltip's layout).
     *
     * @param transientParent will be set as the transient parent of the tooltip.
     * @note The transient parent is required to show the tooltip on Wayland platforms.
     */
    void showAt(const QPoint &pos, QWidget *content, QWindow *transientParent);

    /**
     * Show a tooltip containing @p content centered below @p rect. If there is not
     * enough space in the screen below @p rect, the tooltip will be shown above
     * @p rect, if possible, or at the bottom of the screen otherwise.
     * You can call this method multiple times over the same KToolTipWidget instance
     * (previously shown widgets will be removed from the tooltip's layout).
     *
     * Typically @p rect is the visualRect() of a QAbstractItemView:
     * @snippet ktooltipwidget_test.cpp show_tooltip_widget
     *
     * @param transientParent will be set as the transient parent of the tooltip.
     * @note The transient parent is required to show the tooltip on Wayland platforms.
     */
    void showBelow(const QRect &rect, QWidget *content, QWindow *transientParent);

    /**
     * @return Delay (in ms) after which hideLater() will hide the tooltip. Default is 500.
     * @see hideLater(), setHideDelay()
     */
    int hideDelay() const;

public Q_SLOTS:

    /**
     * Hide the tooltip after a delay of hideDelay() ms (to allow interaction with the tooltip's widget).
     * If hideDelay() is 0, this is equivalent to hide().
     * @see hideDelay()
     */
    void hideLater();

    /**
     * Set after how many ms hideLater() will hide the tooltip.
     * @see hideLater(), hideDelay()
     */
    void setHideDelay(int delay);

Q_SIGNALS:
    /**
     * The tooltip has been hidden and the tooltip's widget is no longer visible.
     * This signal can be used to delete the tooltip's widget.
     */
    void hidden();

protected:
    void enterEvent(QEnterEvent *event) override;
    void hideEvent(QHideEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *event) override;

private:
    std::unique_ptr<class KToolTipWidgetPrivate> const d;

    Q_DISABLE_COPY(KToolTipWidget)
};

#endif
