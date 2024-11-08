/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef LINGMO_QUICK_ITEMS_DIALOG_H
#define LINGMO_QUICK_ITEMS_DIALOG_H

#include <QQmlParserStatus>
#include <QQuickItem>
#include <QQuickWindow>
#include "window-helper.h"

namespace LingmoUIQuick {
class DialogPrivate;
class Dialog : public QQuickWindow, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)
    /**
     * Plasma Location of the dialog window. Useful if this dialog is a popup for a panel
     */
    Q_PROPERTY(LingmoUIQuick::Dialog::PopupLocation location READ location WRITE setLocation NOTIFY locationChanged)

    /**
     * Type of the window
     */
    Q_PROPERTY(LingmoUIQuick::WindowType::Type type READ type WRITE setType NOTIFY typeChanged)
    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)
    Q_PROPERTY(bool enableWindowBlur READ enableWindowBlur WRITE setEnableWindowBlur NOTIFY enableWindowBlurChanged)
    Q_PROPERTY(QRegion blurRegion READ blurRegion WRITE setBlurRegion NOTIFY blurRegionChanged)
    /**
     * Margin between dialog and it's parent item
     */
    Q_PROPERTY(int margin READ margin WRITE setMargin NOTIFY marginChanged)

    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChangedProxy)
    Q_PROPERTY(bool removeHeaderBar READ removeHeaderBar WRITE setRemoveHeaderBar NOTIFY removeHeaderBarChanged)

    Q_CLASSINFO("DefaultProperty", "mainItem")
public:
    /**
     * The Location enumeration describes where on screen an element, such as an
     * Applet or its managing container, is positioned on the screen.
     **/
    enum PopupLocation {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                     is described precisely by this value. */
        Desktop, /**< On the planar desktop layer, extending across
                the full screen from edge to edge */
        FullScreen, /**< Full screen */
        TopEdge, /**< Along the top of the screen*/
        BottomEdge, /**< Along the bottom of the screen*/
        LeftEdge, /**< Along the left side of the screen */
        RightEdge, /**< Along the right side of the screen */
    };
    Q_ENUM(PopupLocation)

    explicit Dialog(QQuickItem *parent = nullptr);
    ~Dialog();

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    LingmoUIQuick::Dialog::PopupLocation location() const;
    void setLocation(LingmoUIQuick::Dialog::PopupLocation location);

    void setType(LingmoUIQuick::WindowType::Type type);
    LingmoUIQuick::WindowType::Type type() const;

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    bool isVisible() const;
    void setVisible(bool visible);

    bool enableWindowBlur() const;
    void setEnableWindowBlur(bool enable);

    QRegion blurRegion() const;
    void setBlurRegion(const QRegion& region);

    int margin() const;
    void setMargin(int margin);

    bool removeHeaderBar() const;
    void setRemoveHeaderBar(bool remove);

    /**
     * @returns The suggested screen position for the popup
     * @param item the item the popup has to be positioned relatively to. if null, the popup will be positioned in the center of the window
     * @param size the size that the popup will have, which influences the final position
     */
    virtual QPoint popupPosition(QQuickItem *item, const QSize &size);
    Q_INVOKABLE void updatePosition();

protected:
    virtual void adjustGeometry(const QRect &geom);
    void classBegin() override;
    void componentComplete() override;
    void resizeEvent(QResizeEvent *re) override;
    void focusOutEvent(QFocusEvent *ev) override;
    bool event(QEvent *event) override;

Q_SIGNALS:
    void mainItemChanged();
    void visualParentChanged();
    void visibleChangedProxy(); // redeclaration of QQuickWindow::visibleChanged
    void typeChanged();
    void locationChanged();
    void hideOnWindowDeactivateChanged();
    /**
     * Emitted when the @see hideOnWindowDeactivate property is @c true and this dialog lost focus to a
     * window that is neither a parent dialog to nor a child dialog of this dialog.
     */
    void windowDeactivated();
    void enableWindowBlurChanged();
    void blurRegionChanged();
    void marginChanged();
    void removeHeaderBarChanged();

private:
    friend class DialogPrivate;
    const QScopedPointer<DialogPrivate> d;

    Q_PRIVATE_SLOT(d, void updateVisibility(bool visible))
    Q_PRIVATE_SLOT(d, void updateMinimumWidth())
    Q_PRIVATE_SLOT(d, void updateMinimumHeight())
    Q_PRIVATE_SLOT(d, void updateMaximumWidth())
    Q_PRIVATE_SLOT(d, void updateMaximumHeight())
    Q_PRIVATE_SLOT(d, void updateLayoutParameters())
    Q_PRIVATE_SLOT(d, void slotMainItemSizeChanged())
};



} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_DIALOG_H
