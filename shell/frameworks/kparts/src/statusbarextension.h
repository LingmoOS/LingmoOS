/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPARTS_STATUSBAREXTENSION_H
#define KPARTS_STATUSBAREXTENSION_H

#include <kparts/kparts_export.h>

#include <QWidget>
#include <memory>

class QStatusBar;
class KMainWindow;
class QEvent;

namespace KParts
{
class Part;
class ReadOnlyPart;

// Defined in impl
class StatusBarItem;
class StatusBarExtensionPrivate;

/**
 * @class StatusBarExtension statusbarextension.h <KParts/StatusBarExtension>
 *
 * @short An extension for KParts that allows more sophisticated statusbar handling
 *
 * Every part can use this class to customize the statusbar as long as it is active.
 * Add items via addStatusBarItem and remove an item with removeStatusBarItem.
 *
 * IMPORTANT: do NOT add any items immediately after constructing the extension.
 * Give the application time to set the statusbar in the extension if necessary.
 */
class KPARTS_EXPORT StatusBarExtension : public QObject
{
    Q_OBJECT

public:
    explicit StatusBarExtension(KParts::Part *parent);
    explicit StatusBarExtension(KParts::ReadOnlyPart *parent); // KF6: REMOVE
    ~StatusBarExtension() override;

    /**
     * This adds a widget to the statusbar for this part.
     * If you use this method instead of using statusBar() directly,
     * this extension will take care of removing the items when the parts GUI
     * is deactivated and will re-add them when it is reactivated.
     * The parameters are the same as QStatusBar::addWidget().
     *
     * Note that you can't use KStatusBar methods (inserting text items by id)
     * but you can create a KStatusBarLabel with a dummy id instead, and use
     * it directly in order to get the same look and feel.
     *
     * @param widget the widget to add
     * @param stretch the stretch factor. 0 for a minimum size.
     * @param permanent passed to QStatusBar::addWidget as the "permanent" bool.
     * Note that the item isn't really permanent though, it goes away when
     * the part is unactivated. This simply controls whether temporary messages
     * hide the @p widget, and whether it's added to the left or to the right side.
     *
     * @Note that the widget does not technically become a child of the
     *       StatusBarExtension in a QObject sense. However, it @em will be deleted
     *       when the StatusBarExtension is deleted.
     *
     * IMPORTANT: do NOT add any items immediately after constructing the extension.
     * Give the application time to set the statusbar in the extension if necessary.
     */
    void addStatusBarItem(QWidget *widget, int stretch, bool permanent);

    /**
     * Remove a widget from the statusbar for this part.
     */
    void removeStatusBarItem(QWidget *widget);

    /**
     * @return the statusbar of the KMainWindow in which this part is currently embedded.
     * WARNING: this could return 0L
     */
    QStatusBar *statusBar() const;

    /**
     * This allows the hosting application to set a particular QStatusBar
     * for this part. If it doesn't do this, the statusbar used will be
     * the one of the KMainWindow in which the part is embedded.
     * Konqueror uses this to assign a view-statusbar to the part.
     * The part should never call this method!
     */
    void setStatusBar(QStatusBar *status);

    /**
     * Queries @p obj for a child object which inherits from this
     * StatusBarExtension class. Convenience method.
     */
    static StatusBarExtension *childObject(QObject *obj);

    /** @internal */
    bool eventFilter(QObject *watched, QEvent *ev) override;

private:
    // for future extensions
    friend class StatusBarExtensionPrivate;
    std::unique_ptr<StatusBarExtensionPrivate> const d;
};

}
#endif // KPARTS_STATUSBAREXTENSION_H
