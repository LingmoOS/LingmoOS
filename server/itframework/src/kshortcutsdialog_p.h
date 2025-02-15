/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHORTCUTSDIALOG_P_H
#define KSHORTCUTSDIALOG_P_H

#include "kkeysequencewidget.h"
#include "kshortcutseditor.h"

#include <KExtendableItemDelegate>

#include <QCollator>
#include <QGroupBox>
#include <QKeySequence>
#include <QList>
#include <QMetaType>
#include <QModelIndex>
#include <QTreeWidget>

class QLabel;
class QTreeWidgetItem;
class QRadioButton;
class QAction;
class KActionCollection;
class QPushButton;
class QComboBox;
class KShortcutsDialog;

enum ColumnDesignation {
    Name = 0,
    LocalPrimary,
    LocalAlternate,
    GlobalPrimary,
    GlobalAlternate,
    RockerGesture,
    ShapeGesture,
    Id,
};

enum MyRoles {
    ShortcutRole = Qt::UserRole,
    DefaultShortcutRole,
    ObjectRole,
};

/**
 * Type used for QTreeWidgetItems
 *
 * @internal
 */
enum ItemTypes {
    NonActionItem = 0,
    ActionItem = 1,
};

QKeySequence primarySequence(const QList<QKeySequence> &sequences);
QKeySequence alternateSequence(const QList<QKeySequence> &sequences);

/**
 * Mixes the KShortcutWidget into the treeview used by KShortcutsEditor. When selecting an shortcut
 * it changes the display from "CTRL-W" to the Widget.
 *
 * @bug That delegate uses KExtendableItemDelegate. That means a cell can be expanded. When selected
 * a cell is replaced by a KShortcutsEditor. When painting the widget KExtendableItemDelegate
 * reparents the widget to the viewport of the itemview it belongs to. The widget is destroyed when
 * the user selects another shortcut or explicitly issues a contractItem event. But when the user
 * clears the model the delegate misses that event and doesn't delete the KShortcutseditor. And
 * remains as a visible artefact in your treeview. Additionally when closing your application you get
 * an assertion failure from KExtendableItemDelegate.
 *
 * @internal
 */
class KShortcutsEditorDelegate : public KExtendableItemDelegate
{
    Q_OBJECT
public:
    KShortcutsEditorDelegate(QTreeWidget *parent, bool allowLetterShortcuts);
    // reimplemented to have some extra height
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * Set a list of action collections to check against for conflicting
     * shortcuts.
     *
     * @see KKeySequenceWidget::setCheckActionCollections
     */
    void setCheckActionCollections(const QList<KActionCollection *> &checkActionCollections);

Q_SIGNALS:
    void shortcutChanged(const QVariant &, const QModelIndex &);
public Q_SLOTS:
    void hiddenBySearchLine(QTreeWidgetItem *, bool);

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    mutable QPersistentModelIndex m_editingIndex;
    const bool m_allowLetterShortcuts;
    QWidget *m_editor = nullptr;

    //! List of actionCollections to check for conflicts.
    QList<KActionCollection *> m_checkActionCollections;

private Q_SLOTS:
    void itemActivated(const QModelIndex &index);

    /**
     * When the user collapses a hole subtree of shortcuts then remove eventually
     * extended items. Else we get that artefact bug. See above.
     */
    void itemCollapsed(const QModelIndex &index);

    /**
     * If the user allowed stealing a shortcut we want to be able to undo
     * that.
     */
    void stealShortcut(const QKeySequence &seq, QAction *action);

    void keySequenceChanged(const QKeySequence &);

#if 0
    void shapeGestureChanged(const KShapeGesture &);
    void rockerGestureChanged(const KRockerGesture &);
#endif
};

/**
 * That widget draws the decoration for KShortCutWidget. That widget is currently the only user.
 *
 * @internal
 */
class TabConnectedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabConnectedWidget(QWidget *parent)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *pe) override;
};

/**
 * Edit a shortcut. Let you select between using the default shortcut and configuring your own.
 *
 * @internal
 */
class ShortcutEditWidget : public TabConnectedWidget
{
    Q_OBJECT
public:
    ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq, const QKeySequence &activeSeq, bool allowLetterShortcuts);

    //! @see KKeySequenceWidget::setCheckActionCollections()
    void setCheckActionCollections(const QList<KActionCollection *> &checkActionCollections);

    //@{
    //! @see KKeySequenceWidget::checkAgainstStandardShortcuts()
    KKeySequenceWidget::ShortcutTypes checkForConflictsAgainst() const;
    void setCheckForConflictsAgainst(KKeySequenceWidget::ShortcutTypes);
    //@}

    //@{
    //! @see KKeySequenceWidget::checkAgainstStandardShortcuts()
    bool multiKeyShortcutsAllowed() const;
    void setMultiKeyShortcutsAllowed(bool);
    //@}

    //! @see KKeySequenceWidget::setComponentName
    void setComponentName(const QString &componentName);

    void setAction(QObject *action);

public Q_SLOTS:

    //! Set the displayed sequences
    void setKeySequence(const QKeySequence &activeSeq);

Q_SIGNALS:

    //! Emitted when the key sequence is changed.
    void keySequenceChanged(const QKeySequence &);

    //! @see KKeySequenceWidget::stealShortcut()
    void stealShortcut(const QKeySequence &seq, QAction *action);

private Q_SLOTS:

    void defaultToggled(bool);
    void setCustom(const QKeySequence &);

private:
    QLabel *m_defaultLabel;
    QKeySequence m_defaultKeySequence;
    QRadioButton *m_defaultRadio;
    QRadioButton *m_customRadio;
    KKeySequenceWidget *m_customEditor;
    bool m_isUpdating;
    QObject *m_action;
    const QString m_noneText; // Translated "None" text for labels
};

#if 0
Q_DECLARE_METATYPE(KShapeGesture)
Q_DECLARE_METATYPE(KRockerGesture)
#endif

class KShortcutSchemesEditor : public QGroupBox
{
    Q_OBJECT
public:
    explicit KShortcutSchemesEditor(KShortcutsDialog *parent);

    /** @return the currently selected scheme in the editor (may differ from current app's scheme.*/
    QString currentScheme();
    void refreshSchemes();
    void addMoreMenuAction(QAction *action);

private Q_SLOTS:
    void newScheme();
    void deleteScheme();
    void exportShortcutsScheme();
    void importShortcutsScheme();
    void saveAsDefaultsForScheme();

Q_SIGNALS:
    void shortcutsSchemeChanged(const QString &);

protected:
    void updateDeleteButton();

private:
    QPushButton *m_newScheme;
    QPushButton *m_deleteScheme;
    QPushButton *m_exportScheme;
    QComboBox *m_schemesList;
    QMenu *m_moreActionsMenu;

    KShortcutsDialog *m_dialog;
};

class QAction;
#if 0
class KShapeGesture;
class KRockerGesture;
#endif

/**
 * A QTreeWidgetItem that can handle QActions.
 *
 * It provides undo, commit functionality for changes made. Changes are effective immediately. You
 * have to commit them or they will be undone when deleting the item.
 *
 * @internal
 */
class KShortcutsEditorItem : public QTreeWidgetItem
{
public:
    KShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action);

    /**
     * Destructor
     *
     * Will undo pending changes. If you don't want that. Call commitChanges before
     */
    ~KShortcutsEditorItem() override;

    //! Undo the changes since the last commit.
    void undo();

    //! Commit the changes.
    void commit();

    QVariant data(int column, int role = Qt::DisplayRole) const override;
    bool operator<(const QTreeWidgetItem &other) const override;

    QKeySequence keySequence(uint column) const;
    void setKeySequence(uint column, const QKeySequence &seq);
#if 0
    void setShapeGesture(const KShapeGesture &gst);
    void setRockerGesture(const KRockerGesture &gst);
#endif

    bool isModified(uint column) const;
    bool isModified() const;

    void setNameBold(bool flag)
    {
        m_isNameBold = flag;
    }

private:
    friend class KShortcutsEditorPrivate;

    //! Recheck modified status - could have changed back to initial value
    void updateModified();

    //! The action this item is responsible for
    QAction *m_action;

    //! Should the Name column be painted in bold?
    bool m_isNameBold;

    //@{
    //! The original shortcuts before user changes. 0 means no change.
    QList<QKeySequence> *m_oldLocalShortcut = nullptr;
    QList<QKeySequence> *m_oldGlobalShortcut = nullptr;
#if 0
    KShapeGesture *m_oldShapeGesture;
    KRockerGesture *m_oldRockerGesture;
#endif
    //@}

    //! The localized action name
    QString m_actionNameInTable;

    //! The action id. Needed for exporting and importing
    QString m_id;

    //! The collator, for sorting
    QCollator m_collator;
};

// NEEDED FOR KShortcutsEditorPrivate
#include "ui_kshortcutsdialog.h"

// Hack to make two protected methods public.
// Used by both KShortcutsEditorPrivate and KShortcutsEditorDelegate
class QTreeWidgetHack : public QTreeWidget
{
public:
    QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const
    {
        return QTreeWidget::itemFromIndex(index);
    }
    QModelIndex indexFromItem(QTreeWidgetItem *item, int column) const
    {
        return QTreeWidget::indexFromItem(item, column);
    }
};

/**
 * This class should belong into kshortcutseditor.cpp. But kshortcutseditordelegate uses a static
 * function of this class. So for now it's here. But i will remove it later.
 *
 * @internal
 */
class KShortcutsEditorPrivate
{
public:
    explicit KShortcutsEditorPrivate(KShortcutsEditor *qq);

    void initGUI(KShortcutsEditor::ActionTypes actionTypes, KShortcutsEditor::LetterShortcuts allowLetterShortcuts);
    void appendToView(uint nList, const QString &title = QString());
    // used in appendToView
    QTreeWidgetItem *findOrMakeItem(QTreeWidgetItem *parent, const QString &name);

    static KShortcutsEditorItem *itemFromIndex(QTreeWidget *const w, const QModelIndex &index);

    // Set all shortcuts to their default values (bindings).
    void allDefault();

    // clear all shortcuts
    void clearConfiguration();

    // Import shortcuts from file
    void importConfiguration(KConfigBase *config);

#if 0
    //helper functions for conflict resolution
    bool stealShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &gest);
    bool stealRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &gest);
#endif

    // conflict resolution functions
    void changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture);
#if 0
    void changeShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &capture);
    void changeRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &capture);
#endif

    // private slots
    // this invokes the appropriate conflict resolution function
    void capturedShortcut(const QVariant &, const QModelIndex &);

    //! Represents the three hierarchies the dialog handles.
    struct HierarchyInfo {
        QTreeWidgetItem *root = nullptr;
        QTreeWidgetItem *program = nullptr;
        QTreeWidgetItem *action = nullptr;
    };

    /**
     * Add @p action at @p level. Checks for QActions and unnamed actions
     * before adding.
     *
     * @return @c true if the action was really added, @c false if not
     */
    bool addAction(QAction *action, QTreeWidgetItem *parent);

    void printShortcuts() const;

    void setActionTypes(KShortcutsEditor::ActionTypes actionTypes);

    void setGlobalColumnsHidden(bool hide);
    void setLocalColumnsHidden(bool hide);

    // members
    QList<KActionCollection *> actionCollections;
    KShortcutsEditor *q;

    Ui::KShortcutsDialog ui;

    KShortcutsEditor::ActionTypes actionTypes;
    KShortcutsEditorDelegate *delegate;

    // Tracks if there are any Global shortcuts in any of the action collections shown in the dialog
    bool m_hasAnyGlobalShortcuts = false;
};

Q_DECLARE_METATYPE(KShortcutsEditorItem *)

#endif /* KSHORTCUTSDIALOG_P_H */
