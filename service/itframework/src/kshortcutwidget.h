/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSHORTCUTWIDGET_H
#define KSHORTCUTWIDGET_H

#include <kxmlgui_export.h>

#include <QKeySequence>
#include <QList>
#include <QWidget>
#include <memory>

class KActionCollection;
class KShortcutWidgetPrivate;

/**
 * @class KShortcutWidget kshortcutwidget.h KShortcutWidget
 *
 * \image html kshortcutwidget.png "KShortcutWidget"
 */
class KXMLGUI_EXPORT KShortcutWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool modifierlessAllowed READ isModifierlessAllowed WRITE setModifierlessAllowed)
public:
    KShortcutWidget(QWidget *parent = nullptr);
    ~KShortcutWidget() override;

    void setModifierlessAllowed(bool allow);
    bool isModifierlessAllowed();

    void setClearButtonsShown(bool show);

    QList<QKeySequence> shortcut() const;

    /**
     * Set a list of action collections to check against for conflictuous shortcut.
     *
     * If there is a conflictuous shortcut with a QAction, and that his shortcut can be configured
     * (KActionCollection::isShortcutConfigurable() returns true) the user will be prompted for eventually steal
     * the shortcut from this action
     *
     * Global shortcuts are automatically checked for conflicts
     *
     * Don't forget to call applyStealShortcut to actually steal the shortcut.
     *
     * @since 4.1
     */
    void setCheckActionCollections(const QList<KActionCollection *> &actionCollections);

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(4, 1)
    /**
     * @deprecated since 4.1
     * Use setCheckActionCollections so that KShortcutWidget knows
     * in which action collection to call the writeSettings method after stealing
     * a shortcut from an action.
     */
    KXMLGUI_DEPRECATED_VERSION(4, 1, "Use KShortcutWidget::setCheckActionCollections(const QList<KActionCollection *> &)")
    void setCheckActionList(const QList<QAction *> &checkList);
#endif

Q_SIGNALS:
    void shortcutChanged(const QList<QKeySequence> &cut);

public Q_SLOTS:
    void setShortcut(const QList<QKeySequence> &cut);
    void clearShortcut();

    /**
     * Actually remove the shortcut that the user wanted to steal, from the
     * action that was using it.
     *
     * To be called before you apply your changes.
     * No shortcuts are stolen until this function is called.
     */
    void applyStealShortcut();

private:
    friend class KShortcutWidgetPrivate;
    std::unique_ptr<KShortcutWidgetPrivate> const d;
};

#endif // KSHORTCUTWIDGET_H
