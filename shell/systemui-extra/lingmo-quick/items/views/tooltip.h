/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINGMO_QUICK_ITEMS_TOOLTIP_H
#define LINGMO_QUICK_ITEMS_TOOLTIP_H

class QQuickItem;
#include "tooltip-dialog.h"

namespace LingmoUIQuick {

class Tooltip : public QQuickItem
{
    Q_OBJECT
    /**
     * The item shown inside the tooltip.
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

    /**
     * The main text of this tooltip
     */
    Q_PROPERTY(QString mainText READ mainText WRITE setMainText NOTIFY mainTextChanged)

    /**
     * Returns whether the mouse is inside the item
     */
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)

    /**
     * Plasma Location of the dialog window. Useful if this dialog is a popup for a panel
     */
    Q_PROPERTY(LingmoUIQuick::Dialog::PopupLocation location READ location WRITE setLocation NOTIFY locationChanged)

    /**
     * Property that controls if a tooltips will show on mouse over.
     * The default is true.
     */
    Q_PROPERTY(bool active MEMBER m_active WRITE setActive NOTIFY activeChanged)

    /**
     * Timeout in milliseconds after which the tooltip will hide itself.
     * Set this value to -1 to never hide the tooltip automatically.
     */
    Q_PROPERTY(int timeout MEMBER m_timeout WRITE setTimeout)
    Q_PROPERTY(bool posFollowCursor READ posFollowCursor WRITE setPosFollowCursor NOTIFY activeChanged)
    /**
     * Margin between tooltip and it's parent item, only work for posFollowCursor is false and location is set to
     * TopEdge, BottomEdge, LeftEdge or RightEdge.
     */
    Q_PROPERTY(int margin READ margin WRITE setMargin NOTIFY marginChanged)
    /**
     * If interactive is false (default), the tooltip will automatically hide
     * itself as soon as the mouse leaves the tooltiparea, if is true, if the
     * mouse leaves tooltiparea and goes over the tooltip itself, the tooltip
     * won't hide, so it will be possible to interact with tooltip contents.
     */
    Q_PROPERTY(bool interactive MEMBER m_interactive WRITE setInteractive NOTIFY interactiveChanged)

    /**
     * Returns true if the main text of this tooltip is likely to be rich text
     */
    Q_PROPERTY(bool isRichText MEMBER m_isRichText NOTIFY isRichTextChanged)

public:
    explicit Tooltip(QQuickItem *parent = nullptr);
    ~Tooltip() override;

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QString mainText() const;
    void setMainText(const QString &mainText);

    LingmoUIQuick::Dialog::PopupLocation location() const;
    void setLocation(LingmoUIQuick::Dialog::PopupLocation location);

    bool containsMouse() const;
    void setContainsMouse(bool contains);

    bool posFollowCursor();
    void setPosFollowCursor(bool follow);

    void setActive(bool active);

    void setTimeout(int timeout);

    int margin() const;
    void setMargin(int margin);

    void setInteractive(bool interactive);
public Q_SLOTS:

    /**
     * Shows the tooltip.
     */
    void showTooltip();

    /**
     * Hides the tooltip after a grace period if shown. Does not affect whether the tooltip area is active.
     */
    void hideTooltip();

    /**
     * Hides the tooltip immediately, in comparison to hideTooltip.
     */
    void hideImmediately();

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    TooltipDialog *tooltipDialogInstance();

Q_SIGNALS:
    void mainItemChanged();
    void mainTextChanged();
    void containsMouseChanged();
    void locationChanged();
    void activeChanged();
    /**
     * Emitted just before the tooltip dialog is shown.
     *
     */
    void aboutToShow();
    /**
     * Emitted when the tooltip's visibility changes.
     *
     */
    void toolTipVisibleChanged(bool toolTipVisible);
    void posFollowCursorChanged();
    void marginChanged();
    void interactiveChanged();
    void isRichTextChanged();

private:

    bool isValid() const;
    bool m_tooltipsEnabledGlobally;
    bool m_containsMouse;
    LingmoUIQuick::Dialog::PopupLocation m_location;
    QPointer<QQuickItem> m_mainItem;
    QTimer *m_showTimer;
    QString m_mainText;
    QVariant m_icon;
    bool m_active;
    int m_interval;
    int m_timeout;
    bool m_posFollowCursor;
    int m_margin;
    bool m_interactive;
    bool m_isRichText;

    // TooltipDialog is not a Q_GLOBAL_STATIC because QQuickwindows as global static
    // are deleted too later after some stuff in the qml runtime has already been deleted,
    // causing a crash on exit
    bool m_usingDialog : 1;
    static TooltipDialog *s_dialog;
    static int s_dialogUsers;
    bool m_deactivatedByClick;

};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_TOOLTIP_H
