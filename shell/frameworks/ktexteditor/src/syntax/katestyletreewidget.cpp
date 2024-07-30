/*
    SPDX-FileCopyrightText: 2001-2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katestyletreewidget.h"
#include "kateconfig.h"
#include "kateextendedattribute.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QAction>
#include <QColorDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>

// BEGIN KateStyleTreeDelegate
class KateStyleTreeDelegate : public QStyledItemDelegate
{
public:
    KateStyleTreeDelegate(KateStyleTreeWidget *widget);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    static QBrush getBrushForColorColumn(const QModelIndex &index, int column);
    KateStyleTreeWidget *m_widget;
};
// END

// BEGIN KateStyleTreeWidgetItem decl
/*
    QListViewItem subclass to display/edit a style, bold/italic is check boxes,
    normal and selected colors are boxes, which will display a color chooser when
    activated.
    The context name for the style will be drawn using the editor default font and
    the chosen colors.
    This widget id designed to handle the default as well as the individual hl style
    lists.
    This widget is designed to work with the KateStyleTreeWidget class exclusively.
    Added by anders, jan 23 2002.
*/
class KateStyleTreeWidgetItem : public QTreeWidgetItem
{
public:
    KateStyleTreeWidgetItem(QTreeWidgetItem *parent,
                            const QString &styleName,
                            KTextEditor::Attribute::Ptr defaultstyle,
                            KTextEditor::Attribute::Ptr data = KTextEditor::Attribute::Ptr());
    KateStyleTreeWidgetItem(QTreeWidget *parent,
                            const QString &styleName,
                            KTextEditor::Attribute::Ptr defaultstyle,
                            KTextEditor::Attribute::Ptr data = KTextEditor::Attribute::Ptr());
    ~KateStyleTreeWidgetItem() override
    {
    }

    enum columns {
        Context = 0,
        Bold,
        Italic,
        Underline,
        StrikeOut,
        Foreground,
        SelectedForeground,
        Background,
        SelectedBackground,
        UseDefaultStyle,
        NumColumns
    };

    enum {
        Type = ItemType::UserType + 1,
    };

    /* initializes the style from the default and the hldata */
    void initStyle();
    /* updates the hldata's style */
    void updateStyle();
    /* For bool fields, toggles them, for color fields, display a color chooser */
    void changeProperty(int p);
    /** unset a color.
     * c is 100 (BGColor) or 101 (SelectedBGColor) for now.
     */
    void unsetColor(int c);
    /* style context name */
    QString contextName() const
    {
        return text(0);
    }
    /* only true for a hl mode item using its default style */
    bool defStyle() const;
    /* true for default styles */
    bool isDefault() const;
    /* whichever style is active (currentStyle for hl mode styles not using
       the default style, defaultStyle otherwise) */
    KTextEditor::Attribute::Ptr style() const
    {
        return currentStyle;
    }

    QVariant data(int column, int role) const override;

    KateStyleTreeWidget *treeWidget() const;

private:
    /* private methods to change properties */
    void toggleDefStyle();
    void setColor(int);
    /* helper function to copy the default style into the KateExtendedAttribute,
       when a property is changed and we are using default style. */

    KTextEditor::Attribute::Ptr currentStyle, // the style currently in use (was "is")
        defaultStyle; // default style for hl mode contexts and default styles (was "ds")
    KTextEditor::Attribute::Ptr actualStyle; // itemdata for hl mode contexts (was "st")
};
// END

// BEGIN KateStyleTreeWidget
KateStyleTreeWidget::KateStyleTreeWidget(QWidget *parent, bool showUseDefaults)
    : QTreeWidget(parent)
{
    setItemDelegate(new KateStyleTreeDelegate(this));
    setRootIsDecorated(false);

    QStringList headers;
    headers << i18nc("@title:column Meaning of text in editor", "Context") << QString() << QString() << QString() << QString()
            << i18nc("@title:column Text style", "Normal") << i18nc("@title:column Text style", "Selected") << i18nc("@title:column Text style", "Background")
            << i18nc("@title:column Text style", "Background Selected");
    if (showUseDefaults) {
        headers << i18n("Use Default Style");
    }

    setHeaderLabels(headers);

    headerItem()->setIcon(1, QIcon::fromTheme(QStringLiteral("format-text-bold")));
    headerItem()->setIcon(2, QIcon::fromTheme(QStringLiteral("format-text-italic")));
    headerItem()->setIcon(3, QIcon::fromTheme(QStringLiteral("format-text-underline")));
    headerItem()->setIcon(4, QIcon::fromTheme(QStringLiteral("format-text-strikethrough")));

    // grab the background color and apply it to the palette
    QPalette pal = viewport()->palette();
    pal.setColor(QPalette::Window, KateRendererConfig::global()->backgroundColor());
    viewport()->setPalette(pal);
}

QIcon brushIcon(const QColor &color)
{
    QPixmap pm(16, 16);
    QRect all(0, 0, 15, 15);
    {
        QPainter p(&pm);
        p.fillRect(all, color);
        p.setPen(Qt::black);
        p.drawRect(all);
    }
    return QIcon(pm);
}

bool KateStyleTreeWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    if (m_readOnly) {
        return false;
    }

    if (index.column() == KateStyleTreeWidgetItem::Context) {
        return false;
    }

    auto item = itemFromIndex(index);
    if (item->type() != KateStyleTreeWidgetItem::Type) {
        return QTreeWidget::edit(index, trigger, event);
    }
    KateStyleTreeWidgetItem *i = static_cast<KateStyleTreeWidgetItem *>(item);

    switch (trigger) {
    case QAbstractItemView::DoubleClicked:
    case QAbstractItemView::SelectedClicked:
    case QAbstractItemView::EditKeyPressed:
        i->changeProperty(index.column());
        update(index);
        update(index.sibling(index.row(), KateStyleTreeWidgetItem::Context));
        return false;
    default:
        return QTreeWidget::edit(index, trigger, event);
    }
}

void KateStyleTreeWidget::resizeColumns()
{
    for (int i = 0; i < columnCount(); ++i) {
        resizeColumnToContents(i);
    }
}

void KateStyleTreeWidget::showEvent(QShowEvent *event)
{
    QTreeWidget::showEvent(event);

    resizeColumns();
}

void KateStyleTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_readOnly) {
        return;
    }

    auto item = itemAt(event->pos());
    if (item->type() != KateStyleTreeWidgetItem::Type) {
        return;
    }
    KateStyleTreeWidgetItem *i = static_cast<KateStyleTreeWidgetItem *>(item);

    QMenu m(this);
    KTextEditor::Attribute::Ptr currentStyle = i->style();
    // the title is used, because the menu obscures the context name when
    // displayed on behalf of spacePressed().
    QPainter p;
    p.setPen(Qt::black);

    const QIcon emptyColorIcon = brushIcon(viewport()->palette().base().color());
    QIcon cl = brushIcon(i->style()->foreground().color());
    QIcon scl = brushIcon(i->style()->selectedForeground().color());
    QIcon bgcl = i->style()->hasProperty(QTextFormat::BackgroundBrush) ? brushIcon(i->style()->background().color()) : emptyColorIcon;
    QIcon sbgcl = i->style()->hasProperty(CustomProperties::SelectedBackground) ? brushIcon(i->style()->selectedBackground().color()) : emptyColorIcon;

    m.addSection(i->contextName());

    QAction *a = m.addAction(i18n("&Bold"), this, SLOT(changeProperty()));
    a->setCheckable(true);
    a->setChecked(currentStyle->fontBold());
    a->setData(KateStyleTreeWidgetItem::Bold);

    a = m.addAction(i18n("&Italic"), this, SLOT(changeProperty()));
    a->setCheckable(true);
    a->setChecked(currentStyle->fontItalic());
    a->setData(KateStyleTreeWidgetItem::Italic);

    a = m.addAction(i18n("&Underline"), this, SLOT(changeProperty()));
    a->setCheckable(true);
    a->setChecked(currentStyle->fontUnderline());
    a->setData(KateStyleTreeWidgetItem::Underline);

    a = m.addAction(i18n("S&trikeout"), this, SLOT(changeProperty()));
    a->setCheckable(true);
    a->setChecked(currentStyle->fontStrikeOut());
    a->setData(KateStyleTreeWidgetItem::StrikeOut);

    m.addSeparator();

    a = m.addAction(cl, i18nc("@action:inmenu", "Normal &Color…"), this, SLOT(changeProperty()));
    a->setData(KateStyleTreeWidgetItem::Foreground);

    a = m.addAction(scl, i18nc("@action:inmenu", "&Selected Color…"), this, SLOT(changeProperty()));
    a->setData(KateStyleTreeWidgetItem::SelectedForeground);

    a = m.addAction(bgcl, i18nc("@action:inmenu", "&Background Color…"), this, SLOT(changeProperty()));
    a->setData(KateStyleTreeWidgetItem::Background);

    a = m.addAction(sbgcl, i18nc("@action:inmenu", "S&elected Background Color…"), this, SLOT(changeProperty()));
    a->setData(KateStyleTreeWidgetItem::SelectedBackground);

    // defaulters
    m.addSeparator();

    a = m.addAction(emptyColorIcon, i18n("Unset Normal Color"), this, SLOT(unsetColor()));
    a->setData(1);

    a = m.addAction(emptyColorIcon, i18n("Unset Selected Color"), this, SLOT(unsetColor()));
    a->setData(2);

    // unsetters
    KTextEditor::Attribute::Ptr style = i->style();
    if (style->hasProperty(QTextFormat::BackgroundBrush)) {
        a = m.addAction(emptyColorIcon, i18n("Unset Background Color"), this, SLOT(unsetColor()));
        a->setData(3);
    }

    if (style->hasProperty(CustomProperties::SelectedBackground)) {
        a = m.addAction(emptyColorIcon, i18n("Unset Selected Background Color"), this, SLOT(unsetColor()));
        a->setData(4);
    }

    if (!i->isDefault() && !i->defStyle()) {
        m.addSeparator();
        a = m.addAction(i18n("Use &Default Style"), this, SLOT(changeProperty()));
        a->setCheckable(true);
        a->setChecked(i->defStyle());
        a->setData(KateStyleTreeWidgetItem::UseDefaultStyle);
    }
    m.exec(event->globalPos());
}

void KateStyleTreeWidget::changeProperty()
{
    static_cast<KateStyleTreeWidgetItem *>(currentItem())->changeProperty(static_cast<QAction *>(sender())->data().toInt());
}

void KateStyleTreeWidget::unsetColor()
{
    static_cast<KateStyleTreeWidgetItem *>(currentItem())->unsetColor(static_cast<QAction *>(sender())->data().toInt());
}

void KateStyleTreeWidget::updateGroupHeadings()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        QTreeWidgetItem *currentTopLevelItem = topLevelItem(i);
        QTreeWidgetItem *firstChild = currentTopLevelItem->child(0);

        if (firstChild) {
            QColor foregroundColor = firstChild->data(KateStyleTreeWidgetItem::Foreground, Qt::DisplayRole).value<QColor>();
            QColor backgroundColor = firstChild->data(KateStyleTreeWidgetItem::Background, Qt::DisplayRole).value<QColor>();

            currentTopLevelItem->setForeground(KateStyleTreeWidgetItem::Context, foregroundColor);

            if (backgroundColor.isValid()) {
                currentTopLevelItem->setBackground(KateStyleTreeWidgetItem::Context, backgroundColor);
            }
        }
    }
}

void KateStyleTreeWidget::emitChanged()
{
    updateGroupHeadings();
    Q_EMIT changed();
}

void KateStyleTreeWidget::addItem(const QString &styleName, KTextEditor::Attribute::Ptr defaultstyle, KTextEditor::Attribute::Ptr data)
{
    new KateStyleTreeWidgetItem(this, styleName, std::move(defaultstyle), std::move(data));
}

void KateStyleTreeWidget::addItem(QTreeWidgetItem *parent, const QString &styleName, KTextEditor::Attribute::Ptr defaultstyle, KTextEditor::Attribute::Ptr data)
{
    new KateStyleTreeWidgetItem(parent, styleName, std::move(defaultstyle), std::move(data));
    updateGroupHeadings();
}
// END

// BEGIN KateStyleTreeWidgetItem
KateStyleTreeDelegate::KateStyleTreeDelegate(KateStyleTreeWidget *widget)
    : QStyledItemDelegate(widget)
    , m_widget(widget)
{
}

QBrush KateStyleTreeDelegate::getBrushForColorColumn(const QModelIndex &index, int column)
{
    QModelIndex colorIndex = index.sibling(index.row(), column);
    QVariant displayData = colorIndex.model()->data(colorIndex);
    return displayData.value<QBrush>();
}

void KateStyleTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    static QSet<int> columns;
    if (columns.isEmpty()) {
        columns << KateStyleTreeWidgetItem::Foreground << KateStyleTreeWidgetItem::SelectedForeground << KateStyleTreeWidgetItem::Background
                << KateStyleTreeWidgetItem::SelectedBackground;
    }

    if (index.column() == KateStyleTreeWidgetItem::Context) {
        QStyleOptionViewItem styleContextItem(option);

        QBrush brush = getBrushForColorColumn(index, KateStyleTreeWidgetItem::SelectedBackground);
        if (brush != QBrush()) {
            styleContextItem.palette.setBrush(QPalette::Highlight, brush);
        }

        brush = getBrushForColorColumn(index, KateStyleTreeWidgetItem::SelectedForeground);
        if (brush != QBrush()) {
            styleContextItem.palette.setBrush(QPalette::HighlightedText, brush);
        }

        return QStyledItemDelegate::paint(painter, styleContextItem, index);
    }

    QStyledItemDelegate::paint(painter, option, index);

    if (!columns.contains(index.column())) {
        return;
    }

    QVariant displayData = index.model()->data(index);
    if (displayData.userType() != QMetaType::QBrush) {
        return;
    }

    QBrush brush = displayData.value<QBrush>();

    QStyleOptionButton opt;
    opt.rect = option.rect;
    opt.palette = m_widget->palette();

    bool set = brush != QBrush();

    if (!set) {
        opt.text = i18nc("No text or background color set", "None set");
        brush = Qt::white;
    }

    m_widget->style()->drawControl(QStyle::CE_PushButton, &opt, painter, m_widget);

    if (set) {
        painter->fillRect(m_widget->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, m_widget), brush);
    }
}

KateStyleTreeWidgetItem::KateStyleTreeWidgetItem(QTreeWidgetItem *parent,
                                                 const QString &stylename,
                                                 KTextEditor::Attribute::Ptr defaultAttribute,
                                                 KTextEditor::Attribute::Ptr actualAttribute)
    : QTreeWidgetItem(parent, KateStyleTreeWidgetItem::Type)
    , currentStyle(nullptr)
    , defaultStyle(std::move(defaultAttribute))
    , actualStyle(std::move(actualAttribute))
{
    initStyle();
    setText(0, stylename);
}

KateStyleTreeWidgetItem::KateStyleTreeWidgetItem(QTreeWidget *parent,
                                                 const QString &stylename,
                                                 KTextEditor::Attribute::Ptr defaultAttribute,
                                                 KTextEditor::Attribute::Ptr actualAttribute)
    : QTreeWidgetItem(parent, KateStyleTreeWidgetItem::Type)
    , currentStyle(nullptr)
    , defaultStyle(std::move(defaultAttribute))
    , actualStyle(std::move(actualAttribute))
{
    initStyle();
    setText(0, stylename);
}

void KateStyleTreeWidgetItem::initStyle()
{
    if (!actualStyle) {
        currentStyle = defaultStyle;
    } else {
        currentStyle = new KTextEditor::Attribute(*defaultStyle);

        if (actualStyle->hasAnyProperty()) {
            *currentStyle += *actualStyle;
        }
    }

    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
}

static Qt::CheckState toCheckState(bool b)
{
    return b ? Qt::Checked : Qt::Unchecked;
}

QVariant KateStyleTreeWidgetItem::data(int column, int role) const
{
    if (column == Context) {
        switch (role) {
        case Qt::ForegroundRole:
            if (style()->hasProperty(QTextFormat::ForegroundBrush)) {
                return style()->foreground().color();
            }
            break;

        case Qt::BackgroundRole:
            if (style()->hasProperty(QTextFormat::BackgroundBrush)) {
                return style()->background().color();
            }
            break;

        case Qt::FontRole:
            return style()->font();
            break;
        }
    }

    if (role == Qt::CheckStateRole) {
        switch (column) {
        case Bold:
            return toCheckState(style()->fontBold());
        case Italic:
            return toCheckState(style()->fontItalic());
        case Underline:
            return toCheckState(style()->fontUnderline());
        case StrikeOut:
            return toCheckState(style()->fontStrikeOut());
        case UseDefaultStyle:
            /* can't compare all attributes, currentStyle has always more than defaultStyle (e.g. the item's name),
             * so we just compare the important ones:*/
            return toCheckState(currentStyle->foreground() == defaultStyle->foreground() && currentStyle->background() == defaultStyle->background()
                                && currentStyle->selectedForeground() == defaultStyle->selectedForeground()
                                && currentStyle->selectedBackground() == defaultStyle->selectedBackground()
                                && currentStyle->fontBold() == defaultStyle->fontBold() && currentStyle->fontItalic() == defaultStyle->fontItalic()
                                && currentStyle->fontUnderline() == defaultStyle->fontUnderline()
                                && currentStyle->fontStrikeOut() == defaultStyle->fontStrikeOut());
        }
    }

    if (role == Qt::DisplayRole) {
        switch (column) {
        case Foreground:
            return style()->foreground();
        case SelectedForeground:
            return style()->selectedForeground();
        case Background:
            return style()->background();
        case SelectedBackground:
            return style()->selectedBackground();
        }
    }

    return QTreeWidgetItem::data(column, role);
}

void KateStyleTreeWidgetItem::updateStyle()
{
    // nothing there, not update it, will crash
    if (!actualStyle) {
        return;
    }

    if (currentStyle->hasProperty(QTextFormat::FontWeight)) {
        if (currentStyle->fontWeight() != actualStyle->fontWeight()) {
            actualStyle->setFontWeight(currentStyle->fontWeight());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::FontWeight);
    }

    if (currentStyle->hasProperty(QTextFormat::FontItalic)) {
        if (currentStyle->fontItalic() != actualStyle->fontItalic()) {
            actualStyle->setFontItalic(currentStyle->fontItalic());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::FontItalic);
    }

    if (currentStyle->hasProperty(QTextFormat::FontStrikeOut)) {
        if (currentStyle->fontStrikeOut() != actualStyle->fontStrikeOut()) {
            actualStyle->setFontStrikeOut(currentStyle->fontStrikeOut());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::FontStrikeOut);
    }

    if (currentStyle->hasProperty(QTextFormat::TextUnderlineStyle)) {
        if (currentStyle->fontUnderline() != actualStyle->fontUnderline()) {
            actualStyle->setFontUnderline(currentStyle->fontUnderline());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::TextUnderlineStyle);
    }

    if (currentStyle->hasProperty(CustomProperties::Outline)) {
        if (currentStyle->outline() != actualStyle->outline()) {
            actualStyle->setOutline(currentStyle->outline());
        }
    } else {
        actualStyle->clearProperty(CustomProperties::Outline);
    }

    if (currentStyle->hasProperty(QTextFormat::ForegroundBrush)) {
        if (currentStyle->foreground() != actualStyle->foreground()) {
            actualStyle->setForeground(currentStyle->foreground());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::ForegroundBrush);
    }

    if (currentStyle->hasProperty(CustomProperties::SelectedForeground)) {
        if (currentStyle->selectedForeground() != actualStyle->selectedForeground()) {
            actualStyle->setSelectedForeground(currentStyle->selectedForeground());
        }
    } else {
        actualStyle->clearProperty(CustomProperties::SelectedForeground);
    }

    if (currentStyle->hasProperty(QTextFormat::BackgroundBrush)) {
        if (currentStyle->background() != actualStyle->background()) {
            actualStyle->setBackground(currentStyle->background());
        }
    } else {
        actualStyle->clearProperty(QTextFormat::BackgroundBrush);
    }

    if (currentStyle->hasProperty(CustomProperties::SelectedBackground)) {
        if (currentStyle->selectedBackground() != actualStyle->selectedBackground()) {
            actualStyle->setSelectedBackground(currentStyle->selectedBackground());
        }
    } else {
        actualStyle->clearProperty(CustomProperties::SelectedBackground);
    }
}

/* only true for a hl mode item using its default style */
bool KateStyleTreeWidgetItem::defStyle() const
{
    return actualStyle && actualStyle->properties() != defaultStyle->properties();
}

/* true for default styles */
bool KateStyleTreeWidgetItem::isDefault() const
{
    return actualStyle ? false : true;
}

void KateStyleTreeWidgetItem::changeProperty(int p)
{
    if (p == Bold) {
        currentStyle->setFontBold(!currentStyle->fontBold());
    } else if (p == Italic) {
        currentStyle->setFontItalic(!currentStyle->fontItalic());
    } else if (p == Underline) {
        currentStyle->setFontUnderline(!currentStyle->fontUnderline());
    } else if (p == StrikeOut) {
        currentStyle->setFontStrikeOut(!currentStyle->fontStrikeOut());
    } else if (p == UseDefaultStyle) {
        toggleDefStyle();
    } else {
        setColor(p);
    }

    updateStyle();

    treeWidget()->emitChanged();
}

void KateStyleTreeWidgetItem::toggleDefStyle()
{
    if (*currentStyle == *defaultStyle) {
        KMessageBox::information(treeWidget(),
                                 i18n("\"Use Default Style\" will be automatically unset when you change any style properties."),
                                 i18n("Kate Styles"),
                                 QStringLiteral("Kate hl config use defaults"));
    } else {
        currentStyle = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute(*defaultStyle));
        updateStyle();

        QModelIndex currentIndex = treeWidget()->currentIndex();
        while (currentIndex.isValid()) {
            treeWidget()->update(currentIndex);
            currentIndex = currentIndex.sibling(currentIndex.row(), currentIndex.column() - 1);
        }
    }
}

void KateStyleTreeWidgetItem::setColor(int column)
{
    QColor c; // use this
    QColor d; // default color
    if (column == Foreground) {
        c = currentStyle->foreground().color();
        d = defaultStyle->foreground().color();
    } else if (column == SelectedForeground) {
        c = currentStyle->selectedForeground().color();
        d = defaultStyle->selectedForeground().color();
    } else if (column == Background) {
        c = currentStyle->background().color();
        d = defaultStyle->background().color();
    } else if (column == SelectedBackground) {
        c = currentStyle->selectedBackground().color();
        d = defaultStyle->selectedBackground().color();
    }

    if (!c.isValid()) {
        c = d;
    }

    const QColor selectedColor = QColorDialog::getColor(c, treeWidget());

    if (!selectedColor.isValid()) {
        return;
    }

    // if set default, and the attrib is set in the default style use it
    // else if set default, unset it
    // else set the selected color
    switch (column) {
    case Foreground:
        currentStyle->setForeground(selectedColor);
        break;
    case SelectedForeground:
        currentStyle->setSelectedForeground(selectedColor);
        break;
    case Background:
        currentStyle->setBackground(selectedColor);
        break;
    case SelectedBackground:
        currentStyle->setSelectedBackground(selectedColor);
        break;
    }

    // FIXME
    // repaint();
}

void KateStyleTreeWidgetItem::unsetColor(int colorId)
{
    switch (colorId) {
    case 1:
        if (defaultStyle->hasProperty(QTextFormat::ForegroundBrush)) {
            currentStyle->setForeground(defaultStyle->foreground());
        } else {
            currentStyle->clearProperty(QTextFormat::ForegroundBrush);
        }
        break;
    case 2:
        if (defaultStyle->hasProperty(CustomProperties::SelectedForeground)) {
            currentStyle->setSelectedForeground(defaultStyle->selectedForeground());
        } else {
            currentStyle->clearProperty(CustomProperties::SelectedForeground);
        }
        break;
    case 3:
        if (currentStyle->hasProperty(QTextFormat::BackgroundBrush)) {
            currentStyle->clearProperty(QTextFormat::BackgroundBrush);
        }
        break;
    case 4:
        if (currentStyle->hasProperty(CustomProperties::SelectedBackground)) {
            currentStyle->clearProperty(CustomProperties::SelectedBackground);
        }
        break;
    }

    updateStyle();

    treeWidget()->emitChanged();
}

KateStyleTreeWidget *KateStyleTreeWidgetItem::treeWidget() const
{
    return static_cast<KateStyleTreeWidget *>(QTreeWidgetItem::treeWidget());
}

bool KateStyleTreeWidget::readOnly() const
{
    return m_readOnly;
}

void KateStyleTreeWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}
// END

#include "moc_katestyletreewidget.cpp"
