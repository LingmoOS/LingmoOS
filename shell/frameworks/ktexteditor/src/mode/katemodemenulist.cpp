/*
    SPDX-FileCopyrightText: 2019-2020 Nibaldo González S. <nibgonz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "katemodemenulist.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katemodemanager.h"
#include "katepartdebug.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KLocalizedString>

namespace
{
/**
 * Detect words delimiters:
 *      ! " # $ % & ' ( ) * + , - . / : ;
 *      < = > ? [ \ ] ^ ` { | } ~ « »
 */
static bool isDelimiter(const ushort c)
{
    return (c <= 126 && c >= 33 && (c >= 123 || c <= 47 || (c <= 96 && c >= 58 && c != 95 && (c >= 91 || c <= 63)))) || c == 171 || c == 187;
}

/**
 * Overlay scroll bar on the list according to the operating system
 * and/or the desktop environment. In some desktop themes the scroll bar
 * isn't transparent, so it's better not to overlap it on the list.
 * NOTE: Currently, in the Breeze theme, the scroll bar does not overlap
 * the content. See: https://phabricator.kde.org/T9126
 */
inline static bool overlapScrollBar()
{
    return false;
}
}

void KateModeMenuList::init()
{
    connect(this, &QMenu::aboutToShow, this, &KateModeMenuList::onAboutToShowMenu);
}

void KateModeMenuList::onAboutToShowMenu()
{
    if (m_initialized) {
        return;
    }
    /*
     * Fix font size & font style: display the font correctly when changing it from the
     * KDE Plasma preferences. For example, the font type "Menu" is displayed, but "font()"
     * and "fontMetrics()" return the font type "General". Therefore, this overwrites the
     * "General" font. This makes it possible to correctly apply word wrapping on items,
     * when changing the font or its size.
     */
    QFont font = this->font();
    font.setFamily(font.family());
    font.setStyle(font.style());
    font.setStyleName(font.styleName());
    font.setBold(font.bold());
    font.setItalic(font.italic());
    font.setUnderline(font.underline());
    font.setStrikeOut(font.strikeOut());
    font.setPointSize(font.pointSize());
    setFont(font);

    /*
     * Calculate the size of the list and the checkbox icon (in pixels) according
     * to the font size. From font 12pt to 26pt increase the list size.
     */
    int menuWidth = 266;
    int menuHeight = 428;
    const int fontSize = font.pointSize();
    if (fontSize >= 12) {
        const int increaseSize = (fontSize - 11) * 10;
        if (increaseSize >= 150) { // Font size: 26pt
            menuWidth += 150;
            menuHeight += 150;
        } else {
            menuWidth += increaseSize;
            menuHeight += increaseSize;
        }

        if (fontSize >= 22) {
            m_iconSize = 32;
        } else if (fontSize >= 18) {
            m_iconSize = 24;
        } else if (fontSize >= 14) {
            m_iconSize = 22;
        } else if (fontSize >= 12) {
            m_iconSize = 18;
        }
    }

    // Create list and search bar
    m_list = KateModeMenuListData::Factory::createListView(this);
    m_searchBar = KateModeMenuListData::Factory::createSearchLine(this);

    // Empty icon for items.
    QPixmap emptyIconPixmap(m_iconSize, m_iconSize);
    emptyIconPixmap.fill(Qt::transparent);
    m_emptyIcon = QIcon(emptyIconPixmap);

    /*
     * Load list widget, scroll bar and items.
     */
    if (overlapScrollBar()) {
        // The vertical scroll bar will be added in another layout
        m_scroll = new QScrollBar(Qt::Vertical, this);
        m_list->setVerticalScrollBar(m_scroll);
        m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    m_list->setIconSize(QSize(m_iconSize, m_iconSize));
    m_list->setResizeMode(QListView::Adjust);
    // Size of the list widget and search bar.
    setSizeList(menuHeight, menuWidth);

    // Data model (items).
    // couple model to view to let it be deleted with the view
    m_model = new QStandardItemModel(0, 0, m_list);
    loadHighlightingModel();

    /*
     * Search bar widget.
     */
    m_searchBar->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    m_searchBar->setToolTip(i18nc("ToolTip of the search bar of modes of syntax highlighting",
                                  "Search for syntax highlighting modes by language name or file extension (for example, C++ or .cpp)"));
    m_searchBar->setMaxLength(200);

    m_list->setFocusProxy(m_searchBar);

    /*
     * Set layouts and widgets.
     * container (QWidget)
     * └── layoutContainer (QVBoxLayout)
     *      ├── m_layoutList (QGridLayout)
     *      │   ├── m_list (ListView)
     *      │   ├── layoutScrollBar (QHBoxLayout) --> m_scroll (QScrollBar)
     *      │   └── m_emptyListMsg (QLabel)
     *      └── layoutSearchBar (QHBoxLayout) --> m_searchBar (SearchLine)
     */
    QWidget *container = new QWidget(this);
    QVBoxLayout *layoutContainer = new QVBoxLayout(container);
    m_layoutList = new QGridLayout();
    QHBoxLayout *layoutSearchBar = new QHBoxLayout();

    m_layoutList->addWidget(m_list, 0, 0, Qt::AlignLeft);

    // Add scroll bar and set margin.
    // Overlap scroll bar above the list widget.
    if (overlapScrollBar()) {
        QHBoxLayout *layoutScrollBar = new QHBoxLayout();
        layoutScrollBar->addWidget(m_scroll);
        layoutScrollBar->setContentsMargins(1, 2, 2, 2); // ScrollBar Margin = 2, Also see: KateModeMenuListData::ListView::getContentWidth()
        m_layoutList->addLayout(layoutScrollBar, 0, 0, Qt::AlignRight);
    }

    layoutSearchBar->addWidget(m_searchBar);
    layoutContainer->addLayout(m_layoutList);
    layoutContainer->addLayout(layoutSearchBar);

    QWidgetAction *widAct = new QWidgetAction(this);
    widAct->setDefaultWidget(container);
    addAction(widAct);

    /*
     * Detect selected item with one click.
     * This also applies to double-clicks.
     */
    connect(m_list, &KateModeMenuListData::ListView::clicked, this, &KateModeMenuList::selectHighlighting);

    m_initialized = true;
}

void KateModeMenuList::reloadItems()
{
    // We aren't initialized, nothing to reload
    if (!m_initialized) {
        return;
    }

    const QString searchText = m_searchBar->text().trimmed();
    m_searchBar->m_bestResults.clear();
    if (!isHidden()) {
        hide();
    }
    /*
     * Clear model.
     * NOTE: This deletes the item objects and widgets indexed to items.
     * That is, the QLabel & QFrame objects of the section titles are also deleted.
     * See: QAbstractItemView::setIndexWidget(), QObject::deleteLater()
     */
    m_model->clear();
    m_list->selectionModel()->clear();
    m_selectedItem = nullptr;

    loadHighlightingModel();

    // Restore search text, if there is.
    m_searchBar->m_bSearchStateAutoScroll = false;
    if (!searchText.isEmpty()) {
        selectHighlightingFromExternal();
        m_searchBar->updateSearch(searchText);
        m_searchBar->setText(searchText);
    }
}

void KateModeMenuList::loadHighlightingModel()
{
    m_list->setModel(m_model);

    QString *prevHlSection = nullptr;
    /*
     * The width of the text container in the item, in pixels. This is used to make
     * a custom word wrap and prevent the item's text from passing under the scroll bar.
     * NOTE: 8 = Icon margin
     */
    const int maxWidthText = m_list->getContentWidth(1, 8) - m_iconSize - 8;

    // Transparent color used as background in the sections.
    QPixmap transparentPixmap = QPixmap(m_iconSize / 2, m_iconSize / 2);
    transparentPixmap.fill(Qt::transparent);
    QBrush transparentBrush(transparentPixmap);

    /*
     * The first item on the list is the "Best Search Matches" section,
     * which will remain hidden and will only be shown when necessary.
     */
    createSectionList(QString(), transparentBrush, false);
    m_defaultHeightItemSection = m_list->visualRect(m_model->index(0, 0)).height();
    m_list->setRowHidden(0, true);

    /*
     * Get list of modes from KateModeManager::list().
     * We assume that the modes are arranged according to sections, alphabetically;
     * and the attribute "translatedSection" isn't empty if "section" has a value.
     */
    for (auto *hl : KTextEditor::EditorPrivate::self()->modeManager()->list()) {
        if (hl->name.isEmpty()) {
            continue;
        }

        // Detects a new section.
        if (!hl->translatedSection.isEmpty() && (prevHlSection == nullptr || hl->translatedSection != *prevHlSection)) {
            createSectionList(hl->sectionTranslated(), transparentBrush);
        }
        prevHlSection = hl->translatedSection.isNull() ? nullptr : &hl->translatedSection;

        // Create item in the list with the language name.
        KateModeMenuListData::ListItem *item = KateModeMenuListData::Factory::createListItem();
        /*
         * NOTE:
         *  - (If the scroll bar is not overlapped) In QListView::setWordWrap(),
         *    when the scroll bar is hidden, the word wrap changes, but the size
         *    of the items is keeped, causing display problems in some items.
         *    KateModeMenuList::setWordWrap() applies a fixed word wrap.
         *  - Search names generated in: KateModeMenuListData::SearchLine::updateSearch()
         */
        item->setText(setWordWrap(hl->nameTranslated(), maxWidthText, m_list->fontMetrics()));
        item->setMode(hl);

        item->setIcon(m_emptyIcon);
        item->setEditable(false);
        // Add item
        m_model->appendRow(item);
    }
}

KateModeMenuListData::ListItem *KateModeMenuList::createSectionList(const QString &sectionName, const QBrush &background, bool bSeparator, int modelPosition)
{
    /*
     * Add a separator to the list.
     */
    if (bSeparator) {
        KateModeMenuListData::ListItem *separator = KateModeMenuListData::Factory::createListItem();
        separator->setFlags(Qt::NoItemFlags);
        separator->setEnabled(false);
        separator->setEditable(false);
        separator->setSelectable(false);

        separator->setSizeHint(QSize(separator->sizeHint().width() - 2, 4));
        separator->setBackground(background);

        QFrame *line = new QFrame(m_list);
        line->setFrameStyle(QFrame::HLine);

        if (modelPosition < 0) {
            m_model->appendRow(separator);
        } else {
            m_model->insertRow(modelPosition, separator);
        }
        m_list->setIndexWidget(m_model->index(separator->row(), 0), line);
        m_list->selectionModel()->select(separator->index(), QItemSelectionModel::Deselect);
    }

    /*
     * Add the section name to the list.
     */
    KateModeMenuListData::ListItem *section = KateModeMenuListData::Factory::createListItem();
    section->setFlags(Qt::NoItemFlags);
    section->setEnabled(false);
    section->setEditable(false);
    section->setSelectable(false);

    QLabel *label = new QLabel(sectionName, m_list);
    if (m_list->layoutDirection() == Qt::RightToLeft) {
        label->setAlignment(Qt::AlignRight);
    }
    label->setTextFormat(Qt::PlainText);
    label->setIndent(6);

    /*
     * NOTE: Names of sections in bold. The font color
     * should change according to Kate's color theme.
     */
    QFont font = label->font();
    font.setWeight(QFont::Bold);
    label->setFont(font);

    section->setBackground(background);

    if (modelPosition < 0) {
        m_model->appendRow(section);
    } else {
        m_model->insertRow(modelPosition + 1, section);
    }
    m_list->setIndexWidget(m_model->index(section->row(), 0), label);
    m_list->selectionModel()->select(section->index(), QItemSelectionModel::Deselect);

    // Apply word wrap in sections, for long labels.
    const int containerTextWidth = m_list->getContentWidth(2, 4);
    int heightSectionMargin = m_list->visualRect(m_model->index(section->row(), 0)).height() - label->sizeHint().height();

    if (label->sizeHint().width() > containerTextWidth) {
        label->setText(setWordWrap(label->text(), containerTextWidth - label->indent(), label->fontMetrics()));
        if (heightSectionMargin < 2) {
            heightSectionMargin = 2;
        }
        section->setSizeHint(QSize(section->sizeHint().width(), label->sizeHint().height() + heightSectionMargin));
    } else if (heightSectionMargin < 2) {
        section->setSizeHint(QSize(section->sizeHint().width(), label->sizeHint().height() + 2));
    }

    return section;
}

void KateModeMenuList::setButton(QPushButton *button, AlignmentHButton positionX, AlignmentVButton positionY, AutoUpdateTextButton autoUpdateTextButton)
{
    if (positionX == AlignHInverse) {
        if (layoutDirection() == Qt::RightToLeft) {
            m_positionX = KateModeMenuList::AlignLeft;
        } else {
            m_positionX = KateModeMenuList::AlignRight;
        }
    } else if (positionX == AlignLeft && layoutDirection() != Qt::RightToLeft) {
        m_positionX = KateModeMenuList::AlignHDefault;
    } else {
        m_positionX = positionX;
    }

    m_positionY = positionY;
    m_pushButton = button;
    m_autoUpdateTextButton = autoUpdateTextButton;
}

void KateModeMenuList::setSizeList(const int height, const int width)
{
    m_list->setSizeList(height, width);
    m_searchBar->setWidth(width);
}

void KateModeMenuList::autoScroll()
{
    if (m_selectedItem && m_autoScroll == ScrollToSelectedItem) {
        m_list->setCurrentItem(m_selectedItem->row());
        m_list->scrollToItem(m_selectedItem->row(), QAbstractItemView::PositionAtCenter);
    } else {
        m_list->scrollToFirstItem();
    }
}

void KateModeMenuList::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    /*
     * TODO: Put the menu on the bottom-edge of the window if the status bar is hidden,
     * to show the menu with keyboard shortcuts. To do this, it's preferable to add a new
     * function/slot to display the menu, correcting the position. If the trigger button
     * isn't set or is destroyed, there may be problems detecting Right-to-left layouts.
     */

    // Set the menu position
    if (m_pushButton && m_pushButton->isVisible()) {
        /*
         * Get vertical position.
         * NOTE: In KDE Plasma with Wayland, the reference point of the position
         * is the main window, not the desktop. Therefore, if the window is vertically
         * smaller than the menu, it will be positioned on the upper edge of the window.
         */
        int newMenu_y; // New vertical menu position
        if (m_positionY == AlignTop) {
            newMenu_y = m_pushButton->mapToGlobal(QPoint(0, 0)).y() - geometry().height();
            if (newMenu_y < 0) {
                newMenu_y = 0;
            }
        } else {
            newMenu_y = pos().y();
        }

        // Set horizontal position.
        if (m_positionX == AlignRight) {
            // New horizontal menu position
            int newMenu_x = pos().x() - geometry().width() + m_pushButton->geometry().width();
            // Get position of the right edge of the toggle button
            const int buttonPositionRight = m_pushButton->mapToGlobal(QPoint(0, 0)).x() + m_pushButton->geometry().width();
            if (newMenu_x < 0) {
                newMenu_x = 0;
            } else if (newMenu_x + geometry().width() < buttonPositionRight) {
                newMenu_x = buttonPositionRight - geometry().width();
            }
            move(newMenu_x, newMenu_y);
        } else if (m_positionX == AlignLeft) {
            move(m_pushButton->mapToGlobal(QPoint(0, 0)).x(), newMenu_y);
        } else if (m_positionY == AlignTop) {
            // Set vertical position, use the default horizontal position
            move(pos().x(), newMenu_y);
        }
    }

    // Select text from the search bar
    if (!m_searchBar->text().isEmpty()) {
        if (m_searchBar->text().trimmed().isEmpty()) {
            m_searchBar->clear();
        } else {
            m_searchBar->selectAll();
        }
    }

    // Set focus on the list. The list widget uses focus proxy to the search bar.
    m_list->setFocus(Qt::ActiveWindowFocusReason);

    KTextEditor::DocumentPrivate *doc = m_doc;
    if (!doc) {
        return;
    }

    // First show or if an external changed the current syntax highlighting.
    if (!m_selectedItem || (m_selectedItem->hasMode() && m_selectedItem->getMode()->name != doc->fileType())) {
        if (!selectHighlightingFromExternal(doc->fileType())) {
            // Strange case: if the current syntax highlighting does not exist in the list.
            if (m_selectedItem) {
                m_selectedItem->setIcon(m_emptyIcon);
            }
            if ((m_selectedItem || !m_list->currentItem()) && m_searchBar->text().isEmpty()) {
                m_list->scrollToFirstItem();
            }
            m_selectedItem = nullptr;
        }
    }
}

void KateModeMenuList::updateSelectedItem(KateModeMenuListData::ListItem *item)
{
    // Change the previously selected item to empty icon
    if (m_selectedItem) {
        m_selectedItem->setIcon(m_emptyIcon);
    }

    // Update the selected item
    item->setIcon(m_checkIcon);
    m_selectedItem = item;
    m_list->setCurrentItem(item->row());

    // Change text of the trigger button
    if (bool(m_autoUpdateTextButton) && m_pushButton && item->hasMode()) {
        m_pushButton->setText(item->getMode()->nameTranslated());
    }
}

void KateModeMenuList::selectHighlightingSetVisibility(QStandardItem *pItem, const bool bHideMenu)
{
    if (!pItem || !pItem->isSelectable() || !pItem->isEnabled()) {
        return;
    }

    KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>(pItem);

    if (!item->text().isEmpty()) {
        updateSelectedItem(item);
    }
    if (bHideMenu) {
        hide();
    }

    // Apply syntax highlighting
    KTextEditor::DocumentPrivate *doc = m_doc;
    if (doc && item->hasMode()) {
        doc->updateFileType(item->getMode()->name, true);
    }
}

void KateModeMenuList::selectHighlighting(const QModelIndex &index)
{
    selectHighlightingSetVisibility(m_model->item(index.row(), 0), true);
}

bool KateModeMenuList::selectHighlightingFromExternal(const QString &nameMode)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>(m_model->item(i, 0));

        if (!item->hasMode() || m_model->item(i, 0)->text().isEmpty()) {
            continue;
        }
        if (item->getMode()->name == nameMode || (nameMode.isEmpty() && item->getMode()->name == QLatin1String("Normal"))) {
            updateSelectedItem(item);

            // Clear search
            if (!m_searchBar->text().isEmpty()) {
                // Prevent the empty list message from being seen over the items for a short time
                if (m_emptyListMsg) {
                    m_emptyListMsg->hide();
                }

                // NOTE: This calls updateSearch(), it's scrolled to the selected item or the first item.
                m_searchBar->clear();
            } else if (m_autoScroll == ScrollToSelectedItem) {
                m_list->scrollToItem(i);
            } else {
                // autoScroll()
                m_list->scrollToFirstItem();
            }
            return true;
        }
    }
    return false;
}

bool KateModeMenuList::selectHighlightingFromExternal()
{
    KTextEditor::DocumentPrivate *doc = m_doc;
    if (doc) {
        return selectHighlightingFromExternal(doc->fileType());
    }
    return false;
}

void KateModeMenuList::loadEmptyMsg()
{
    m_emptyListMsg = new QLabel(i18nc("A search yielded no results", "No items matching your search"), this);
    m_emptyListMsg->setMargin(15);
    m_emptyListMsg->setWordWrap(true);

    const int fontSize = font().pointSize() > 10 ? font().pointSize() + 4 : 14;

    QColor color = m_emptyListMsg->palette().color(QPalette::Text);
    m_emptyListMsg->setStyleSheet(QLatin1String("font-size: ") + QString::number(fontSize) + QLatin1String("pt; color: rgba(") + QString::number(color.red())
                                  + QLatin1Char(',') + QString::number(color.green()) + QLatin1Char(',') + QString::number(color.blue())
                                  + QLatin1String(", 0.3);"));

    m_emptyListMsg->setAlignment(Qt::AlignCenter);
    m_layoutList->addWidget(m_emptyListMsg, 0, 0, Qt::AlignCenter);
}

QString KateModeMenuList::setWordWrap(const QString &text, const int maxWidth, const QFontMetrics &fontMetrics) const
{
    // Get the length of the text, in pixels, and compare it with the container
    if (fontMetrics.horizontalAdvance(text) <= maxWidth) {
        return text;
    }

    // Add line breaks in the text to fit in the container
    QStringList words = text.split(QLatin1Char(' '));
    if (words.count() < 1) {
        return text;
    }
    QString newText = QString();
    QString tmpLineText = QString();

    for (int i = 0; i < words.count() - 1; ++i) {
        // Elide mode in long words
        if (fontMetrics.horizontalAdvance(words[i]) > maxWidth) {
            if (!tmpLineText.isEmpty()) {
                newText += tmpLineText + QLatin1Char('\n');
                tmpLineText.clear();
            }
            newText +=
                fontMetrics.elidedText(words[i], m_list->layoutDirection() == Qt::RightToLeft ? Qt::ElideLeft : Qt::ElideRight, maxWidth) + QLatin1Char('\n');
            continue;
        } else {
            tmpLineText += words[i];
        }

        // This prevents the last line of text from having only one word with 1 or 2 chars
        if (i == words.count() - 3 && words[i + 2].length() <= 2
            && fontMetrics.horizontalAdvance(tmpLineText + QLatin1Char(' ') + words[i + 1] + QLatin1Char(' ') + words[i + 2]) > maxWidth) {
            newText += tmpLineText + QLatin1Char('\n');
            tmpLineText.clear();
        }
        // Add line break if the maxWidth is exceeded with the next word
        else if (fontMetrics.horizontalAdvance(tmpLineText + QLatin1Char(' ') + words[i + 1]) > maxWidth) {
            newText += tmpLineText + QLatin1Char('\n');
            tmpLineText.clear();
        } else {
            tmpLineText.append(QLatin1Char(' '));
        }
    }

    // Add line breaks in delimiters, if the last word is greater than the container
    bool bElidedLastWord = false;
    if (fontMetrics.horizontalAdvance(words[words.count() - 1]) > maxWidth) {
        bElidedLastWord = true;
        const int lastw = words.count() - 1;
        for (int c = words[lastw].length() - 1; c >= 0; --c) {
            if (isDelimiter(words[lastw][c].unicode()) && fontMetrics.horizontalAdvance(words[lastw].mid(0, c + 1)) <= maxWidth) {
                bElidedLastWord = false;
                if (fontMetrics.horizontalAdvance(words[lastw].mid(c + 1)) > maxWidth) {
                    words[lastw] = words[lastw].mid(0, c + 1) + QLatin1Char('\n')
                        + fontMetrics.elidedText(words[lastw].mid(c + 1),
                                                 m_list->layoutDirection() == Qt::RightToLeft ? Qt::ElideLeft : Qt::ElideRight,
                                                 maxWidth);
                } else {
                    words[lastw].insert(c + 1, QLatin1Char('\n'));
                }
                break;
            }
        }
    }

    if (!tmpLineText.isEmpty()) {
        newText += tmpLineText;
    }
    if (bElidedLastWord) {
        newText += fontMetrics.elidedText(words[words.count() - 1], m_list->layoutDirection() == Qt::RightToLeft ? Qt::ElideLeft : Qt::ElideRight, maxWidth);
    } else {
        newText += words[words.count() - 1];
    }
    return newText;
}

void KateModeMenuListData::SearchLine::setWidth(const int width)
{
    setMinimumWidth(width);
    setMaximumWidth(width);
}

void KateModeMenuListData::ListView::setSizeList(const int height, const int width)
{
    setMinimumWidth(width);
    setMaximumWidth(width);
    setMinimumHeight(height);
    setMaximumHeight(height);
}

int KateModeMenuListData::ListView::getWidth() const
{
    // Equivalent to: sizeHint().width()
    // But "sizeHint().width()" returns an incorrect value when the menu is large.
    return size().width() - 4;
}

int KateModeMenuListData::ListView::getContentWidth(const int overlayScrollbarMargin, const int classicScrollbarMargin) const
{
    if (overlapScrollBar()) {
        return getWidth() - m_parentMenu->m_scroll->sizeHint().width() - 2 - overlayScrollbarMargin; // ScrollBar Margin = 2
    }
    return getWidth() - verticalScrollBar()->sizeHint().width() - classicScrollbarMargin;
}

int KateModeMenuListData::ListView::getContentWidth() const
{
    return getContentWidth(0, 0);
}

bool KateModeMenuListData::ListItem::generateSearchName(const QString &itemName)
{
    QString searchName = QString(itemName);
    bool bNewName = false;

    // Replace word delimiters with spaces
    for (int i = searchName.length() - 1; i >= 0; --i) {
        if (isDelimiter(searchName[i].unicode())) {
            searchName.replace(i, 1, QLatin1Char(' '));
            if (!bNewName) {
                bNewName = true;
            }
        }
        // Avoid duplicate delimiters/spaces
        if (bNewName && i < searchName.length() - 1 && searchName[i].isSpace() && searchName[i + 1].isSpace()) {
            searchName.remove(i + 1, 1);
        }
    }

    if (bNewName) {
        if (searchName[searchName.length() - 1].isSpace()) {
            searchName.remove(searchName.length() - 1, 1);
        }
        if (searchName[0].isSpace()) {
            searchName.remove(0, 1);
        }
        m_searchName = searchName;
        return true;
    } else {
        m_searchName = itemName;
    }
    return false;
}

bool KateModeMenuListData::ListItem::matchExtension(const QString &text) const
{
    if (!hasMode() || m_type->wildcards.count() == 0) {
        return false;
    }

    /*
     * Only file extensions and full names are matched. Files like "Kconfig*"
     * aren't considered. It's also assumed that "text" doesn't contain '*'.
     */
    for (const auto &ext : m_type->wildcards) {
        // File extension
        if (ext.startsWith(QLatin1String("*."))) {
            if (text.length() == ext.length() - 2 && text.compare(QStringView(ext).mid(2), Qt::CaseInsensitive) == 0) {
                return true;
            }
        } else if (text.length() != ext.length() || ext.endsWith(QLatin1Char('*'))) {
            continue;
            // Full name
        } else if (text.compare(ext, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

void KateModeMenuListData::ListView::keyPressEvent(QKeyEvent *event)
{
    // Ctrl/Alt/Shift/Meta + Return/Enter selects an item, but without hiding the menu
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        && (event->modifiers().testFlag(Qt::ControlModifier) || event->modifiers().testFlag(Qt::AltModifier) || event->modifiers().testFlag(Qt::ShiftModifier)
            || event->modifiers().testFlag(Qt::MetaModifier))) {
        m_parentMenu->selectHighlightingSetVisibility(m_parentMenu->m_list->currentItem(), false);
    }
    // Return/Enter selects an item and hide the menu
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_parentMenu->selectHighlightingSetVisibility(m_parentMenu->m_list->currentItem(), true);
    } else {
        QListView::keyPressEvent(event);
    }
}

void KateModeMenuListData::SearchLine::keyPressEvent(QKeyEvent *event)
{
    if (m_parentMenu->m_list
        && (event->matches(QKeySequence::MoveToNextLine) || event->matches(QKeySequence::SelectNextLine) || event->matches(QKeySequence::MoveToPreviousLine)
            || event->matches(QKeySequence::SelectPreviousLine) || event->matches(QKeySequence::MoveToNextPage) || event->matches(QKeySequence::SelectNextPage)
            || event->matches(QKeySequence::MoveToPreviousPage) || event->matches(QKeySequence::SelectPreviousPage) || event->key() == Qt::Key_Return
            || event->key() == Qt::Key_Enter)) {
        QApplication::sendEvent(m_parentMenu->m_list, event);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

void KateModeMenuListData::SearchLine::init()
{
    connect(this, &KateModeMenuListData::SearchLine::textChanged, this, &KateModeMenuListData::SearchLine::_k_queueSearch);

    setEnabled(true);
    setClearButtonEnabled(true);
}

void KateModeMenuListData::SearchLine::clear()
{
    m_queuedSearches = 0;
    m_bSearchStateAutoScroll = (text().trimmed().isEmpty()) ? false : true;
    /*
     * NOTE: This calls "SearchLine::_k_queueSearch()" with an empty string.
     * The search clearing should be done without delays.
     */
    QLineEdit::clear();
}

void KateModeMenuListData::SearchLine::_k_queueSearch(const QString &s)
{
    m_queuedSearches++;
    m_search = s;

    if (m_search.isEmpty()) {
        _k_activateSearch(); // Clear search without delay
    } else {
        QTimer::singleShot(m_searchDelay, this, &KateModeMenuListData::SearchLine::_k_activateSearch);
    }
}

void KateModeMenuListData::SearchLine::_k_activateSearch()
{
    m_queuedSearches--;

    if (m_queuedSearches <= 0) {
        updateSearch(m_search);
        m_queuedSearches = 0;
    }
}

void KateModeMenuListData::SearchLine::updateSearch(const QString &s)
{
    if (m_parentMenu->m_emptyListMsg) {
        m_parentMenu->m_emptyListMsg->hide();
    }
    if (m_parentMenu->m_scroll && m_parentMenu->m_scroll->isHidden()) {
        m_parentMenu->m_scroll->show();
    }

    KateModeMenuListData::ListView *listView = m_parentMenu->m_list;
    QStandardItemModel *listModel = m_parentMenu->m_model;

    const QString searchText = (s.isNull() ? text() : s).simplified();

    /*
     * Clean "Best Search Matches" section, move items to their original places.
     */
    if (!listView->isRowHidden(0)) {
        listView->setRowHidden(0, true);
    }
    if (!m_bestResults.isEmpty()) {
        const int sizeBestResults = m_bestResults.size();
        for (int i = 0; i < sizeBestResults; ++i) {
            listModel->takeRow(m_bestResults.at(i).first->index().row());
            listModel->insertRow(m_bestResults.at(i).second + sizeBestResults - i - 1, m_bestResults.at(i).first);
        }
        m_bestResults.clear();
    }

    /*
     * Empty search bar.
     * Show all items and scroll to the selected item or to the first item.
     */
    if (searchText.isEmpty() || (searchText.size() == 1 && searchText[0].isSpace())) {
        for (int i = 1; i < listModel->rowCount(); ++i) {
            if (listView->isRowHidden(i)) {
                listView->setRowHidden(i, false);
            }
        }

        // Don't auto-scroll if the search is already clear
        if (m_bSearchStateAutoScroll) {
            m_parentMenu->autoScroll();
        }
        m_bSearchStateAutoScroll = false;
        return;
    }

    /*
     * Prepare item filter.
     */
    int lastItem = -1;
    int lastSection = -1;
    int firstSection = -1;
    bool bEmptySection = true;
    bool bSectionSeparator = false;
    bool bSectionName = false;
    bool bNotShowBestResults = false;
    bool bSearchExtensions = true;
    bool bExactMatch = false; // If the search name will not be used
    /*
     * It's used for two purposes, it's true if searchText is a
     * single alphanumeric character or if it starts with a point.
     * Both cases don't conflict, so a single bool is used.
     */
    bool bIsAlphaOrPointExt = false;

    /*
     * Don't search for extensions if the search text has only one character,
     * to avoid unwanted results. In this case, the items that start with
     * that character are displayed.
     */
    if (searchText.length() < 2) {
        bSearchExtensions = false;
        if (searchText[0].isLetterOrNumber()) {
            bIsAlphaOrPointExt = true;
        }
    }
    // If the search text has a point at the beginning, match extensions
    else if (searchText.length() > 1 && searchText[0].toLatin1() == 46) {
        bIsAlphaOrPointExt = true;
        bSearchExtensions = true;
        bExactMatch = true;
    }
    // Two characters: search using the normal name of the items
    else if (searchText.length() == 2) {
        bExactMatch = true;
        // if it contains the '*' character, don't match extensions
        if (searchText[1].toLatin1() == 42 || searchText[0].toLatin1() == 42) {
            bSearchExtensions = false;
        }
    }
    /*
     * Don't use the search name if the search text has delimiters.
     * Don't search in extensions if it contains the '*' character.
     */
    else {
        QString::const_iterator srcText = searchText.constBegin();
        QString::const_iterator endText = searchText.constEnd();

        for (int it = 0; it < searchText.length() / 2 + searchText.length() % 2; ++it) {
            --endText;
            const ushort ucsrc = srcText->unicode();
            const ushort ucend = endText->unicode();

            // If searchText contains "*"
            if (ucsrc == 42 || ucend == 42) {
                bSearchExtensions = false;
                bExactMatch = true;
                break;
            }
            if (!bExactMatch && (isDelimiter(ucsrc) || (ucsrc != ucend && isDelimiter(ucend)))) {
                bExactMatch = true;
            }
            ++srcText;
        }
    }

    /*
     * Filter items.
     */
    for (int i = 1; i < listModel->rowCount(); ++i) {
        QString itemName = listModel->item(i, 0)->text();

        /*
         * Hide/show the name of the section. If the text of the item
         * is empty, then it corresponds to the name of the section.
         */
        if (itemName.isEmpty()) {
            listView->setRowHidden(i, false);

            if (bSectionSeparator) {
                bSectionName = true;
            } else {
                bSectionSeparator = true;
            }

            /*
             * This hides the name of the previous section
             * (and the separator) if this section has no items.
             */
            if (bSectionName && bEmptySection && lastSection > 0) {
                listView->setRowHidden(lastSection, true);
                listView->setRowHidden(lastSection - 1, true);
            }

            // Find the section name
            if (bSectionName) {
                bSectionName = false;
                bSectionSeparator = false;
                bEmptySection = true;
                lastSection = i;
            }
            continue;
        }

        /*
         * Start filtering items.
         */
        KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>(listModel->item(i, 0));

        if (!item->hasMode()) {
            listView->setRowHidden(i, true);
            continue;
        }
        if (item->getSearchName().isEmpty()) {
            item->generateSearchName(item->getMode()->translatedName.isEmpty() ? item->getMode()->name : item->getMode()->translatedName);
        }

        /*
         * Add item to the "Best Search Matches" section if there is an exact match in the search.
         * However, if the "exact match" is already the first search result, that section will not
         * be displayed, as it isn't necessary.
         */
        if (!bNotShowBestResults
            && (item->getSearchName().compare(searchText, m_caseSensitivity) == 0
                || (bExactMatch && item->getMode()->nameTranslated().compare(searchText, m_caseSensitivity) == 0))) {
            if (lastItem == -1) {
                bNotShowBestResults = true;
            } else {
                m_bestResults.append(qMakePair(item, i));
                continue;
            }
        }

        // Only a character is written in the search bar
        if (searchText.length() == 1) {
            if (bIsAlphaOrPointExt) {
                /*
                 * Add item to the "Best Search Matches" section, if there is a single letter.
                 * Also look for coincidence in the raw name, some translations use delimiters
                 * instead of spaces and this can lead to inaccurate results.
                 */
                bool bMatchCharDel = true;
                if (item->getMode()->name.startsWith(searchText + QLatin1Char(' '), m_caseSensitivity)) {
                    if (QString(QLatin1Char(' ') + item->getSearchName() + QLatin1Char(' '))
                            .contains(QLatin1Char(' ') + searchText + QLatin1Char(' '), m_caseSensitivity)) {
                        m_bestResults.append(qMakePair(item, i));
                        continue;
                    } else {
                        bMatchCharDel = false;
                    }
                }

                // CASE 1: All the items that start with that character will be displayed.
                if (item->getSearchName().startsWith(searchText, m_caseSensitivity)) {
                    setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
                    continue;
                }

                // CASE 2: Matches considering delimiters. For example, when writing "c",
                //         "Objective-C" will be displayed in the results, but not "Yacc/Bison".
                if (bMatchCharDel
                    && QString(QLatin1Char(' ') + item->getSearchName() + QLatin1Char(' '))
                           .contains(QLatin1Char(' ') + searchText + QLatin1Char(' '), m_caseSensitivity)) {
                    setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
                    continue;
                }
            }
            // CASE 3: The character isn't a letter or number, do an exact search.
            else if (item->getMode()->nameTranslated().contains(searchText[0], m_caseSensitivity)) {
                setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
                continue;
            }
        }
        // CASE 4: Search text, using the search name or the normal name.
        else if (!bExactMatch && item->getSearchName().contains(searchText, m_caseSensitivity)) {
            setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
            continue;
        } else if (bExactMatch && item->getMode()->nameTranslated().contains(searchText, m_caseSensitivity)) {
            setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
            continue;
        }

        // CASE 5: Exact matches in extensions.
        if (bSearchExtensions) {
            if (bIsAlphaOrPointExt && item->matchExtension(searchText.mid(1))) {
                setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
                continue;
            } else if (item->matchExtension(searchText)) {
                setSearchResult(i, bEmptySection, lastSection, firstSection, lastItem);
                continue;
            }
        }

        // Item not found, hide
        listView->setRowHidden(i, true);
    }

    // Remove last section name, if it's empty.
    if (bEmptySection && lastSection > 0 && !listModel->item(listModel->rowCount() - 1, 0)->text().isEmpty()) {
        listView->setRowHidden(lastSection, true);
        listView->setRowHidden(lastSection - 1, true);
    }

    // Hide the separator line in the name of the first section.
    if (m_bestResults.isEmpty()) {
        listView->setRowHidden(0, true);
        if (firstSection > 0) {
            listView->setRowHidden(firstSection - 1, true);
        }
    } else {
        /*
         * Show "Best Search Matches" section, if there are items.
         */

        // Show title in singular or plural, depending on the number of items.
        QLabel *labelSection = static_cast<QLabel *>(listView->indexWidget(listModel->index(0, 0)));
        if (m_bestResults.size() == 1) {
            labelSection->setText(
                i18nc("Title (in singular) of the best result in an item search. Please, that the translation doesn't have more than 34 characters, since the "
                      "menu where it's displayed is small and fixed.",
                      "Best Search Match"));
        } else {
            labelSection->setText(
                i18nc("Title (in plural) of the best results in an item search. Please, that the translation doesn't have more than 34 characters, since the "
                      "menu where it's displayed is small and fixed.",
                      "Best Search Matches"));
        }

        int heightSectionMargin = m_parentMenu->m_defaultHeightItemSection - labelSection->sizeHint().height();
        if (heightSectionMargin < 2) {
            heightSectionMargin = 2;
        }
        int maxWidthText = listView->getContentWidth(1, 3);
        // NOTE: labelSection->sizeHint().width() == labelSection->indent() + labelSection->fontMetrics().horizontalAdvance(labelSection->text())
        const bool bSectionMultiline = labelSection->sizeHint().width() > maxWidthText;
        maxWidthText -= labelSection->indent();
        if (!bSectionMultiline) {
            listModel->item(0, 0)->setSizeHint(QSize(listModel->item(0, 0)->sizeHint().width(), labelSection->sizeHint().height() + heightSectionMargin));
            listView->setRowHidden(0, false);
        }

        /*
         * Show items in "Best Search Matches" section.
         */
        int rowModelBestResults = 0; // New position in the model

        // Special Case: always show the "R Script" mode first by typing "r" in the search box
        if (searchText.length() == 1 && searchText.compare(QLatin1String("r"), m_caseSensitivity) == 0) {
            for (const QPair<ListItem *, int> &itemBestResults : std::as_const(m_bestResults)) {
                listModel->takeRow(itemBestResults.second);
                ++rowModelBestResults;
                if (itemBestResults.first->getMode()->name == QLatin1String("R Script")) {
                    listModel->insertRow(1, itemBestResults.first);
                    listView->setRowHidden(1, false);
                } else {
                    listModel->insertRow(rowModelBestResults, itemBestResults.first);
                    listView->setRowHidden(rowModelBestResults, false);
                }
            }
        } else {
            // Move items to the "Best Search Matches" section
            for (const QPair<ListItem *, int> &itemBestResults : std::as_const(m_bestResults)) {
                listModel->takeRow(itemBestResults.second);
                listModel->insertRow(++rowModelBestResults, itemBestResults.first);
                listView->setRowHidden(rowModelBestResults, false);
            }
        }
        if (lastItem == -1) {
            lastItem = rowModelBestResults;
        }

        // Add word wrap in long section titles.
        if (bSectionMultiline) {
            if (listView->visualRect(listModel->index(lastItem, 0)).bottom() + labelSection->sizeHint().height() + heightSectionMargin
                    > listView->geometry().height()
                || labelSection->sizeHint().width() > listView->getWidth() - 1) {
                labelSection->setText(m_parentMenu->setWordWrap(labelSection->text(), maxWidthText, labelSection->fontMetrics()));
            }
            listModel->item(0, 0)->setSizeHint(QSize(listModel->item(0, 0)->sizeHint().width(), labelSection->sizeHint().height() + heightSectionMargin));
            listView->setRowHidden(0, false);
        }

        m_parentMenu->m_list->setCurrentItem(1);
    }

    listView->scrollToTop();

    // Show message of empty list
    if (lastItem == -1) {
        if (m_parentMenu->m_emptyListMsg == nullptr) {
            m_parentMenu->loadEmptyMsg();
        }
        if (m_parentMenu->m_scroll) {
            m_parentMenu->m_scroll->hide();
        }
        m_parentMenu->m_emptyListMsg->show();
    }
    // Hide scroll bar if it isn't necessary
    else if (m_parentMenu->m_scroll && listView->visualRect(listModel->index(lastItem, 0)).bottom() <= listView->geometry().height()) {
        m_parentMenu->m_scroll->hide();
    }

    m_bSearchStateAutoScroll = true;
}

void KateModeMenuListData::SearchLine::setSearchResult(const int rowItem, bool &bEmptySection, int &lastSection, int &firstSection, int &lastItem)
{
    if (lastItem == -1) {
        /*
         * Detect the first result of the search and "select" it.
         * This allows you to scroll through the list using
         * the Up/Down keys after entering a search.
         */
        m_parentMenu->m_list->setCurrentItem(rowItem);

        // Position of the first section visible.
        if (lastSection > 0) {
            firstSection = lastSection;
        }
    }
    if (bEmptySection) {
        bEmptySection = false;
    }

    lastItem = rowItem;
    if (m_parentMenu->m_list->isRowHidden(rowItem)) {
        m_parentMenu->m_list->setRowHidden(rowItem, false);
    }
}

void KateModeMenuList::updateMenu(KTextEditor::Document *doc)
{
    m_doc = static_cast<KTextEditor::DocumentPrivate *>(doc);
}
