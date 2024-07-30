/*
    SPDX-FileCopyrightText: 2019-2020 Nibaldo González S. <nibgonz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
 *  NOTE: The KateModeMenuListData::SearchLine class is based on
 *  KListWidgetSearchLine, by Scott Wheeler <wheeler@kde.org> and
 *  Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>.
 *  See: https://api.kde.org/frameworks/kitemviews/html/classKListWidgetSearchLine.html
 *
 *  TODO: Add keyboard shortcut to show the menu.
 *  See: KateModeMenuList::showEvent()
 */

#ifndef KATEMODEMENULIST_H
#define KATEMODEMENULIST_H

#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QString>

namespace KTextEditor
{
class DocumentPrivate;
class Document;
}

namespace KateModeMenuListData
{
class ListView;
class ListItem;
class SearchLine;
class Factory;
}

class KateFileType;
/**
 * Class of menu to select the
 * syntax highlighting language (mode menu).
 * Provides a menu with a scrollable list plus search bar.
 *
 * This is an alternative to the classic mode menu of the KateModeMenu class.
 *
 * @see KateModeManager, KateFileType, KateModeMenu
 */
class KateModeMenuList : public QMenu
{
public:
    /**
     * Horizontal Alignment with respect to the trigger button.
     * "AlignHDefault" is the normal alignment.
     * "AlignHInverse" uses right alignment in Left-to-right layouts and
     * left alignmentnin Right-to-left layouts (used in some languages).
     * "AlignLeft" and "AlignRight" forces the alignment, regardless of the layout direction.
     * @see setButton(), QWidget::layoutDirection(), Qt::LayoutDirection
     */
    enum AlignmentHButton { AlignHDefault, AlignHInverse, AlignLeft, AlignRight };
    /**
     * Vertical Alignment with respect to the trigger button.
     * "AlignVDefault" uses normal alignment (below the button) and "AlignTop"
     * forces the alignment above the trigger button.
     * @see setButton(), KateStatusBarOpenUpMenu::setVisible()
     */
    enum AlignmentVButton { AlignVDefault, AlignTop };
    /**
     * Define if the trigger button label must be updated when selecting an item.
     * @see setButton()
     */
    enum class AutoUpdateTextButton : bool;
    /**
     * Search bar position, above or below the list.
     */
    enum SearchBarPosition { Top, Bottom };
    /**
     * Defines where the list will scroll after clearing the search or changing the view.
     * @see setAutoScroll(), autoScroll()
     */
    enum AutoScroll { ScrollToSelectedItem, ScrollToTop };

    KateModeMenuList(const QString &title, QWidget *parent)
        : QMenu(title, parent)
    {
        init();
    }

    /**
     * Reload all items.
     * @see KateModeManager::update()
     */
    void reloadItems();

    /**
     * Update the selected item in the list widget, but without changing
     * the syntax highlighting in the document.
     * This is useful for updating this menu, when changing the syntax highlighting
     * from another menu, or from an external one. This doesn't hide or show the menu.
     * @param nameMode Raw name of the syntax highlight definition. If it's empty,
     *                 the "Normal" mode will be used.
     * @return True if @p nameMode exists and is selected.
     */
    bool selectHighlightingFromExternal(const QString &nameMode);
    /**
     * Update the selected item in the list widget, but without changing
     * the syntax highlighting in the document. This doesn't hide or show the menu.
     * The menu is kept updated according to the active syntax highlighting,
     * obtained from the KTextEditor::DocumentPrivate class.
     * @return True if the item is selected correctly.
     * @see KTextEditor::DocumentPrivate::fileType()
     */
    bool selectHighlightingFromExternal();

    /**
     * Set the button that shows this menu. It allows to update the label
     * of the button and define the alignment of the menu with respect to it.
     * This function doesn't call QPushButton::setMenu().
     * @param button Trigger button.
     * @param positionX Horizontal position of the menu with respect to the trigger button.
     * @param positionY Vertical position of the menu with respect to the trigger button.
     * @param autoUpdateTextButton Determines whether the text of the button should be
     *        changed when selecting an item from the menu.
     *
     * @see AlignmentHButton, AlignmentVButton, AutoUpdateTextButton
     */
    void setButton(QPushButton *button,
                   AlignmentHButton positionX = AlignHDefault,
                   AlignmentVButton positionY = AlignTop,
                   AutoUpdateTextButton autoUpdateTextButton = AutoUpdateTextButton(false));

    /**
     * Define the scroll when cleaning the search or changing the view.
     * The default value is AutoScroll::ScrollToSelectedItem.
     * @see AutoScroll
     */
    void setAutoScroll(AutoScroll scroll)
    {
        m_autoScroll = scroll;
    }

    /**
     * Set document to apply the syntax highlighting.
     * @see KTextEditor::DocumentPrivate
     */
    void updateMenu(KTextEditor::Document *doc);

protected:
    friend KateModeMenuListData::ListView;
    friend KateModeMenuListData::ListItem;
    friend KateModeMenuListData::SearchLine;

    /**
     * Action when displaying the menu.
     * Override from QWidget.
     */
    void showEvent(QShowEvent *event) override;

private:
    void init();

    void onAboutToShowMenu();

    /**
     * Define the size of the list widget, in pixels. The @p width is also
     * applied to the search bar. This does not recalculate the word wrap in items.
     */
    inline void setSizeList(const int height, const int width = 266);

    /**
     * Load the data model with the syntax highlighting definitions to show in the list.
     */
    void loadHighlightingModel();

    /**
     * Scroll the list, according to AutoScroll.
     * @see AutoScroll
     */
    void autoScroll();

    /**
     * Set a custom word wrap on a text line, according to a maximum width (in pixels).
     * @param text Line of text
     * @param maxWidth Width of the text container, in pixels.
     * @param fontMetrics Font metrics. See QWidget::fontMetrics()
     */
    QString setWordWrap(const QString &text, const int maxWidth, const QFontMetrics &fontMetrics) const;

    /**
     * Update the selected item in the list, with the active syntax highlighting.
     * This method only changes the selected item, with the checkbox icon, doesn't apply
     * syntax highlighting in the document or hides the menu.
     * @see selectHighlighting(), selectHighlightingFromExternal(), selectHighlightingSetVisibility()
     */
    void updateSelectedItem(KateModeMenuListData::ListItem *item);

    /**
     * Select an item from the list and apply the syntax highlighting in the document.
     * This is equivalent to the slot: KateModeMenuList::selectHighlighting().
     * @param bHideMenu If the menu should be hidden after applying the highlight.
     * @see selectHighlighting()
     */
    void selectHighlightingSetVisibility(QStandardItem *pItem, const bool bHideMenu);

    /**
     * Create a new section in the list of items and add it to the model.
     * It corresponds to a separator line and a title.
     * @param sectionName Section title.
     * @param background Background color is generally transparent.
     * @param bSeparator True if a separation line will also be created before the section title.
     * @param modelPosition Position in the model where to insert the new section. If the value is
     *                      less than zero, the section is added to the end of the list/model.
     * @return A pointer to the item created with the section title.
     */
    KateModeMenuListData::ListItem *createSectionList(const QString &sectionName, const QBrush &background, bool bSeparator = true, int modelPosition = -1);

    /**
     * Load message when the list is empty in the search.
     */
    void loadEmptyMsg();

    AutoScroll m_autoScroll = ScrollToSelectedItem;
    AlignmentHButton m_positionX;
    AlignmentVButton m_positionY;
    AutoUpdateTextButton m_autoUpdateTextButton;

    QPointer<QPushButton> m_pushButton = nullptr;
    QLabel *m_emptyListMsg = nullptr;
    QGridLayout *m_layoutList = nullptr;
    QScrollBar *m_scroll = nullptr;

    KateModeMenuListData::SearchLine *m_searchBar = nullptr;
    KateModeMenuListData::ListView *m_list = nullptr;
    QStandardItemModel *m_model = nullptr;

    /**
     * Item with active syntax highlighting.
     */
    KateModeMenuListData::ListItem *m_selectedItem = nullptr;

    /**
     * Icon for selected/active item (checkbox).
     * NOTE: Selected and inactive items show an icon with incorrect color,
     * however, this isn't a problem, since the list widget is never inactive.
     */
    const QIcon m_checkIcon = QIcon::fromTheme(QStringLiteral("checkbox"));
    QIcon m_emptyIcon;
    int m_iconSize = 16;

    int m_defaultHeightItemSection;

    QPointer<KTextEditor::DocumentPrivate> m_doc;

    bool m_initialized = false;

private:
    /**
     * Action when selecting a item in the list. This also applies
     * the syntax highlighting in the document and hides the menu.
     * This is equivalent to KateModeMenuList::selectHighlightingSetVisibility().
     * @see selectHighlightingSetVisibility(), updateSelectedItem()
     */
    void selectHighlighting(const QModelIndex &index);
};

namespace KateModeMenuListData
{
/**
 * Class of List Widget.
 */
class ListView : public QListView
{
private:
    ListView(KateModeMenuList *menu)
        : QListView(menu)
    {
        m_parentMenu = menu;
    }

public:
    ~ListView() override
    {
    }

    /**
     * Define the size of the widget list.
     * @p height and @p width are values in pixels.
     */
    void setSizeList(const int height, const int width = 266);

    /**
     * Get the width of the list, in pixels.
     * @see QAbstractScrollArea::sizeHint()
     */
    inline int getWidth() const;

    /**
     * Get the width of the contents of the list (in pixels), that is,
     * the list minus the scroll bar and margins.
     */
    int getContentWidth() const;

    /**
     * Get the width of the contents of the list (in pixels), that is, the list minus
     * the scroll bar and margins. The parameter allows you to specify additional margins
     * according to the scroll bar, which can be superimposed or fixed depending to the
     * desktop environment or operating system.
     * @param overlayScrollbarMargin Additional margin for the scroll bar, if it is
     *                               superimposed on the list.
     * @param classicScrollbarMargin Additional margin for the scroll bar, if fixed in the list.
     */
    inline int getContentWidth(const int overlayScrollbarMargin, const int classicScrollbarMargin) const;

    inline void setCurrentItem(const int rowItem)
    {
        selectionModel()->setCurrentIndex(m_parentMenu->m_model->index(rowItem, 0), QItemSelectionModel::ClearAndSelect);
    }

    inline QStandardItem *currentItem() const
    {
        return m_parentMenu->m_model->item(currentIndex().row(), 0);
    }

    inline void scrollToItem(const int rowItem, QAbstractItemView::ScrollHint hint = QAbstractItemView::PositionAtCenter)
    {
        scrollTo(m_parentMenu->m_model->index(rowItem, 0), hint);
    }

    inline void scrollToFirstItem()
    {
        setCurrentItem(1);
        scrollToTop();
    }

protected:
    /**
     * Override from QListView.
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    KateModeMenuList *m_parentMenu = nullptr;
    friend Factory;
};

/**
 * Class of an Item of the Data Model of the List.
 * @see KateModeMenuListData::ListView, KateFileType, QStandardItemModel
 */
class ListItem : public QStandardItem
{
private:
    ListItem()
        : QStandardItem()
    {
    }

    const KateFileType *m_type = nullptr;
    QString m_searchName;

    friend Factory;

public:
    ~ListItem() override
    {
    }

    /**
     * Associate this item with a KateFileType object.
     */
    inline void setMode(KateFileType *type)
    {
        m_type = type;
    }
    const KateFileType *getMode() const
    {
        return m_type;
    }
    bool hasMode() const
    {
        return m_type;
    }

    /**
     * Generate name of the item used for the search.
     * @param itemName The item name.
     * @return True if a new name is generated for the search.
     */
    bool generateSearchName(const QString &itemName);

    /**
     * Find matches in the extensions of the item mode, with a @p text.
     * @param text Text to match, without dots or asterisks. For example, in
     *             a common extension, it corresponds to the text after "*."
     * @return True if a match is found, false if not.
     */
    bool matchExtension(const QString &text) const;

    const QString &getSearchName() const
    {
        return m_searchName;
    }
};

/**
 * Class of Search Bar.
 * Based on the KListWidgetSearchLine class.
 */
class SearchLine : public QLineEdit
{
public:
    ~SearchLine() override
    {
        m_bestResults.clear();
    }

    /**
     * Define the width of the search bar, in pixels.
     */
    void setWidth(const int width);

private:
    SearchLine(KateModeMenuList *menu)
        : QLineEdit(menu)
    {
        m_parentMenu = menu;
        init();
    }

    void init();

    /**
     * Select result of the items search.
     * Used only by KateModeMenuListData::SearchLine::updateSearch().
     */
    void setSearchResult(const int rowItem, bool &bEmptySection, int &lastSection, int &firstSection, int &lastItem);

    /**
     * Delay in search results after typing, in milliseconds.
     * Default value: 200
     */
    static const int m_searchDelay = 170;

    /**
     * This prevents auto-scrolling when the search is kept clean.
     */
    bool m_bSearchStateAutoScroll = false;

    QString m_search = QString();
    int m_queuedSearches = 0;
    Qt::CaseSensitivity m_caseSensitivity = Qt::CaseInsensitive;

    /**
     * List of items to display in the "Best Search Matches" section. The integer value
     * corresponds to the original position of the item in the model. The purpose of this
     * is to restore the position of the items when starting or cleaning a search.
     */
    QList<QPair<ListItem *, int>> m_bestResults;

    KateModeMenuList *m_parentMenu = nullptr;
    friend Factory;
    friend void KateModeMenuList::reloadItems();

protected:
    /**
     * Override from QLineEdit. This allows you to navigate through
     * the menu and write in the search bar simultaneously with the keyboard.
     */
    void keyPressEvent(QKeyEvent *event) override;

public:
    virtual void clear();
    virtual void updateSearch(const QString &s = QString());

private:
    void _k_queueSearch(const QString &s);
    void _k_activateSearch();
};

class Factory
{
private:
    friend KateModeMenuList;
    Factory(){};
    static ListView *createListView(KateModeMenuList *parentMenu)
    {
        return new ListView(parentMenu);
    }
    static ListItem *createListItem()
    {
        return new ListItem();
    }
    static SearchLine *createSearchLine(KateModeMenuList *parentMenu)
    {
        return new SearchLine(parentMenu);
    }
};
}

#endif // KATEMODEMENULIST_H
