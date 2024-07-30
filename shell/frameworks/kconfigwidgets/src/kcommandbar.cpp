/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kcommandbar.h"
#include "kcommandbarmodel_p.h"
#include "kconfigwidgets_debug.h"

#include <QAction>
#include <QCoreApplication>
#include <QGraphicsOpacityEffect>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QStyledItemDelegate>
#include <QTextLayout>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KFuzzyMatcher>
#include <KLocalizedString>
#include <KSharedConfig>

static QRect getCommandBarBoundingRect(KCommandBar *commandBar)
{
    QWidget *parentWidget = commandBar->parentWidget();
    Q_ASSERT(parentWidget);

    const QMainWindow *mainWindow = qobject_cast<const QMainWindow *>(parentWidget);
    if (!mainWindow) {
        return parentWidget->geometry();
    }

    QRect boundingRect = mainWindow->contentsRect();

    // exclude the menu bar from the bounding rect
    if (const QWidget *menuWidget = mainWindow->menuWidget()) {
        if (!menuWidget->isHidden()) {
            boundingRect.setTop(boundingRect.top() + menuWidget->height());
        }
    }

    // exclude the status bar from the bounding rect
    if (const QStatusBar *statusBar = mainWindow->findChild<QStatusBar *>()) {
        if (!statusBar->isHidden()) {
            boundingRect.setBottom(boundingRect.bottom() - statusBar->height());
        }
    }

    // exclude any undocked toolbar from the bounding rect
    const QList<QToolBar *> toolBars = mainWindow->findChildren<QToolBar *>();
    for (QToolBar *toolBar : toolBars) {
        if (toolBar->isHidden() || toolBar->isFloating()) {
            continue;
        }

        switch (mainWindow->toolBarArea(toolBar)) {
        case Qt::TopToolBarArea:
            boundingRect.setTop(std::max(boundingRect.top(), toolBar->geometry().bottom()));
            break;
        case Qt::RightToolBarArea:
            boundingRect.setRight(std::min(boundingRect.right(), toolBar->geometry().left()));
            break;
        case Qt::BottomToolBarArea:
            boundingRect.setBottom(std::min(boundingRect.bottom(), toolBar->geometry().top()));
            break;
        case Qt::LeftToolBarArea:
            boundingRect.setLeft(std::max(boundingRect.left(), toolBar->geometry().right()));
            break;
        default:
            break;
        }
    }

    return boundingRect;
}

// BEGIN CommandBarFilterModel
class CommandBarFilterModel final : public QSortFilterProxyModel
{
public:
    CommandBarFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        connect(this, &CommandBarFilterModel::modelAboutToBeReset, this, [this]() {
            m_hasActionsWithIcons = false;
        });
    }

    bool hasActionsWithIcons() const
    {
        return m_hasActionsWithIcons;
    }

    Q_SLOT void setFilterString(const QString &string)
    {
        // MUST reset the model here, we want to repopulate
        // invalidateFilter() will not work here
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        const int scoreLeft = sourceLeft.data(KCommandBarModel::Score).toInt();
        const int scoreRight = sourceRight.data(KCommandBarModel::Score).toInt();
        if (scoreLeft == scoreRight) {
            const QString textLeft = sourceLeft.data().toString();
            const QString textRight = sourceRight.data().toString();

            return textRight.localeAwareCompare(textLeft) < 0;
        }

        return scoreLeft < scoreRight;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

        bool accept = false;
        if (m_pattern.isEmpty()) {
            accept = true;
        } else {
            const QString row = index.data(Qt::DisplayRole).toString();
            KFuzzyMatcher::Result resAction = KFuzzyMatcher::match(m_pattern, row);
            sourceModel()->setData(index, resAction.score, KCommandBarModel::Score);
            accept = resAction.matched;
        }

        if (accept && !m_hasActionsWithIcons) {
            m_hasActionsWithIcons |= !index.data(Qt::DecorationRole).isNull();
        }

        return accept;
    }

private:
    QString m_pattern;
    mutable bool m_hasActionsWithIcons = false;
};
// END CommandBarFilterModel

class CommandBarStyleDelegate final : public QStyledItemDelegate
{
public:
    CommandBarStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    /**
     * Paints a single item's text
     */
    static void
    paintItemText(QPainter *p, const QString &textt, const QRect &rect, const QStyleOptionViewItem &options, QList<QTextLayout::FormatRange> formats)
    {
        QString text = options.fontMetrics.elidedText(textt, Qt::ElideRight, rect.width());

        // set formats and font
        QTextLayout textLayout(text, options.font);
        formats.append(textLayout.formats());
        textLayout.setFormats(formats);

        // set alignment, rtls etc
        QTextOption textOption;
        textOption.setTextDirection(options.direction);
        textOption.setAlignment(QStyle::visualAlignment(options.direction, options.displayAlignment));
        textLayout.setTextOption(textOption);

        // layout the text
        textLayout.beginLayout();

        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            return;
        }

        const int lineWidth = rect.width();
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, 0));

        textLayout.endLayout();

        /**
         * get "Y" so that we can properly V-Center align the text in row
         */
        const int y = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignVCenter, textLayout.boundingRect().size().toSize(), rect).y();

        // draw the text
        const QPointF pos(rect.x(), y);
        textLayout.draw(p, pos);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const override
    {
        painter->save();

        /**
         * Draw everything, (widget, icon etc) except the text
         */
        QStyleOptionViewItem option = opt;
        initStyleOption(&option, index);
        option.text.clear(); // clear old text
        QStyle *style = option.widget->style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        const int hMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, option.widget);

        QRect textRect = option.rect;

        const CommandBarFilterModel *model = static_cast<const CommandBarFilterModel *>(index.model());
        if (model->hasActionsWithIcons()) {
            const int iconWidth = option.decorationSize.width() + (hMargin * 2);
            if (option.direction == Qt::RightToLeft) {
                textRect.adjust(0, 0, -iconWidth, 0);
            } else {
                textRect.adjust(iconWidth, 0, 0, 0);
            }
        }

        const QString original = index.data().toString();
        QStringView str = original;
        int componentIdx = original.indexOf(QLatin1Char(':'));
        int actionNameStart = 0;
        if (componentIdx > 0) {
            actionNameStart = componentIdx + 2;
            // + 2 because there is a space after colon
            str = str.mid(actionNameStart);
        }

        QList<QTextLayout::FormatRange> formats;
        if (componentIdx > 0) {
            QTextCharFormat gray;
            gray.setForeground(option.palette.placeholderText());
            formats.append({0, componentIdx, gray});
        }

        QTextCharFormat fmt;
        fmt.setForeground(option.palette.link());
        fmt.setFontWeight(QFont::Bold);

        /**
         * Highlight matches from fuzzy matcher
         */
        const auto fmtRanges = KFuzzyMatcher::matchedRanges(m_filterString, str);
        QTextCharFormat f;
        f.setForeground(option.palette.link());
        formats.reserve(formats.size() + fmtRanges.size());
        std::transform(fmtRanges.begin(), fmtRanges.end(), std::back_inserter(formats), [f, actionNameStart](const KFuzzyMatcher::Range &fr) {
            return QTextLayout::FormatRange{fr.start + actionNameStart, fr.length, f};
        });

        textRect.adjust(hMargin, 0, -hMargin, 0);
        paintItemText(painter, original, textRect, option, std::move(formats));

        painter->restore();
    }

public Q_SLOTS:
    void setFilterString(const QString &text)
    {
        m_filterString = text;
    }

private:
    QString m_filterString;
};

class ShortcutStyleDelegate final : public QStyledItemDelegate
{
public:
    ShortcutStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        // draw background
        option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

        const QString shortcutString = index.data().toString();
        if (shortcutString.isEmpty()) {
            return;
        }

        const ShortcutSegments shortcutSegments = splitShortcut(shortcutString);
        if (shortcutSegments.isEmpty()) {
            return;
        }

        struct Button {
            int textWidth;
            QString text;
        };

        // compute the width of each shortcut segment
        QList<Button> btns;
        btns.reserve(shortcutSegments.count());
        const int hMargin = horizontalMargin(option);
        for (const QString &text : shortcutSegments) {
            int textWidth = option.fontMetrics.horizontalAdvance(text);
            textWidth += 2 * hMargin;
            btns.append({textWidth, text});
        }

        int textHeight = option.fontMetrics.lineSpacing();
        // this happens on gnome so we manually decrease the height a bit
        if (textHeight == option.rect.height()) {
            textHeight -= 4;
        }

        const int y = option.rect.y() + (option.rect.height() - textHeight) / 2;
        int x;
        if (option.direction == Qt::RightToLeft) {
            x = option.rect.x() + hMargin;
        } else {
            x = option.rect.right() - shortcutDrawingWidth(option, shortcutSegments, hMargin) - hMargin;
        }

        painter->save();
        painter->setPen(option.palette.buttonText().color());
        painter->setRenderHint(QPainter::Antialiasing);
        for (int i = 0, n = btns.count(); i < n; ++i) {
            const Button &button = btns.at(i);

            QRect outputRect(x, y, button.textWidth, textHeight);

            // an even element indicates that it is a key
            if (i % 2 == 0) {
                painter->save();
                painter->setPen(Qt::NoPen);

                // draw rounded rect shadow
                auto shadowRect = outputRect.translated(0, 1);
                painter->setBrush(option.palette.shadow());
                painter->drawRoundedRect(shadowRect, 3.0, 3.0);

                // draw rounded rect itself
                painter->setBrush(option.palette.window());
                painter->drawRoundedRect(outputRect, 3.0, 3.0);

                painter->restore();
            }

            // draw shortcut segment
            painter->drawText(outputRect, Qt::AlignCenter, button.text);

            x += outputRect.width();
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.isValid() && index.column() == KCommandBarModel::Column_Shortcut) {
            const QString shortcut = index.data().toString();
            if (!shortcut.isEmpty()) {
                const ShortcutSegments shortcutSegments = splitShortcut(shortcut);
                if (!shortcutSegments.isEmpty()) {
                    const int hMargin = horizontalMargin(option);
                    int width = shortcutDrawingWidth(option, shortcutSegments, hMargin);

                    // add left and right margins
                    width += 2 * hMargin;

                    return QSize(width, 0);
                }
            }
        }

        return QStyledItemDelegate::sizeHint(option, index);
    }

private:
    using ShortcutSegments = QStringList;

    // split shortcut into segments i.e. will return
    // ["Ctrl", "+", "A", ", ", "Ctrl", "+", "K"] for "Ctrl+A, Ctrl+K"
    // twice as fast as using regular expressions
    static ShortcutSegments splitShortcut(const QString &shortcut)
    {
        ShortcutSegments segments;
        if (!shortcut.isEmpty()) {
            const int shortcutLength = shortcut.length();
            int start = 0;
            for (int i = 0; i < shortcutLength; ++i) {
                const QChar c = shortcut.at(i);
                if (c == QLatin1Char('+')) {
                    if (i > start) {
                        segments << shortcut.mid(start, i - start);
                    }
                    segments << shortcut.at(i);
                    start = i + 1;
                } else if (c == QLatin1Char(',')) {
                    if (i > start) {
                        segments << shortcut.mid(start, i - start);
                        start = i;
                    }
                    const int j = i + 1;
                    if (j < shortcutLength && shortcut.at(j) == QLatin1Char(' ')) {
                        segments << shortcut.mid(start, j - start + 1);
                        i = j;
                    } else {
                        segments << shortcut.at(i);
                    }
                    start = i + 1;
                }
            }
            if (start < shortcutLength) {
                segments << shortcut.mid(start);
            }

            // check we have successfully parsed the shortcut
            if (segments.isEmpty()) {
                qCWarning(KCONFIG_WIDGETS_LOG) << "Splitting shortcut failed" << shortcut;
            }
        }

        return segments;
    }

    // returns the width needed to draw the shortcut
    static int shortcutDrawingWidth(const QStyleOptionViewItem &option, const ShortcutSegments &shortcutSegments, int hMargin)
    {
        int width = 0;
        if (!shortcutSegments.isEmpty()) {
            width = option.fontMetrics.horizontalAdvance(shortcutSegments.join(QString()));

            // add left and right margins for each segment
            width += shortcutSegments.count() * 2 * hMargin;
        }

        return width;
    }

    int horizontalMargin(const QStyleOptionViewItem &option) const
    {
        return option.widget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin, &option) + 2;
    }
};

// BEGIN KCommandBarPrivate
class KCommandBarPrivate
{
public:
    QTreeView m_treeView;
    QLineEdit m_lineEdit;
    KCommandBarModel m_model;
    CommandBarFilterModel m_proxyModel;

    /**
     * selects first item in treeview
     */
    void reselectFirst()
    {
        const QModelIndex index = m_proxyModel.index(0, 0);
        m_treeView.setCurrentIndex(index);
    }

    /**
     * blocks signals before clearing line edit to ensure
     * we don't trigger filtering / sorting
     */
    void clearLineEdit()
    {
        const QSignalBlocker blocker(m_lineEdit);
        m_lineEdit.clear();
    }

    void slotReturnPressed(KCommandBar *q);

    void setLastUsedActions();

    QStringList lastUsedActions() const;
};

void KCommandBarPrivate::slotReturnPressed(KCommandBar *q)
{
    auto act = m_proxyModel.data(m_treeView.currentIndex(), Qt::UserRole).value<QAction *>();
    if (act) {
        // if the action is a menu, we take all its actions
        // and reload our dialog with these instead.
        if (auto menu = act->menu()) {
            auto menuActions = menu->actions();
            KCommandBar::ActionGroup ag;

            // if there are no actions, trigger load actions
            // this happens with some menus that are loaded on demand
            if (menuActions.size() == 0) {
                Q_EMIT menu->aboutToShow();
                ag.actions = menu->actions();
            }

            QString groupName = KLocalizedString::removeAcceleratorMarker(act->text());
            ag.name = groupName;

            m_model.refresh({ag});
            reselectFirst();
            /**
             *  We want the "textChanged" signal here
             *  so that proxy model triggers filtering again
             *  so don't use d->clearLineEdit()
             */
            m_lineEdit.clear();
            return;
        } else {
            m_model.actionTriggered(act->text());
            q->hide();
            act->trigger();
        }
    }

    clearLineEdit();
    q->hide();
    q->deleteLater();
}

void KCommandBarPrivate::setLastUsedActions()
{
    auto cfg = KSharedConfig::openStateConfig();
    KConfigGroup cg(cfg, QStringLiteral("General"));

    QStringList actionNames = cg.readEntry(QStringLiteral("CommandBarLastUsedActions"), QStringList());

    return m_model.setLastUsedActions(actionNames);
}

QStringList KCommandBarPrivate::lastUsedActions() const
{
    return m_model.lastUsedActions();
}
// END KCommandBarPrivate

// BEGIN KCommandBar
KCommandBar::KCommandBar(QWidget *parent)
    : QFrame(parent)
    , d(new KCommandBarPrivate)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setProperty("_breeze_force_frame", true);

    QGraphicsDropShadowEffect *e = new QGraphicsDropShadowEffect(this);
    e->setColor(palette().color(QPalette::Dark));
    e->setOffset(0, 4);
    e->setBlurRadius(48);
    setGraphicsEffect(e);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    setLayout(layout);

    layout->addWidget(&d->m_lineEdit);
    d->m_lineEdit.setClearButtonEnabled(true);
    d->m_lineEdit.addAction(QIcon::fromTheme(QStringLiteral("search")), QLineEdit::LeadingPosition);
    d->m_lineEdit.setFrame(false);
    d->m_lineEdit.setTextMargins(QMargins() + style()->pixelMetric(QStyle::PM_ButtonMargin));
    setFocusProxy(&d->m_lineEdit);

    layout->addWidget(&d->m_treeView);
    d->m_treeView.setTextElideMode(Qt::ElideLeft);
    d->m_treeView.setUniformRowHeights(true);
    d->m_treeView.setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags(Qt::TopEdge)));

    CommandBarStyleDelegate *delegate = new CommandBarStyleDelegate(this);
    ShortcutStyleDelegate *del = new ShortcutStyleDelegate(this);
    d->m_treeView.setItemDelegateForColumn(KCommandBarModel::Column_Command, delegate);
    d->m_treeView.setItemDelegateForColumn(KCommandBarModel::Column_Shortcut, del);

    connect(&d->m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        d->slotReturnPressed(this);
    });
    connect(&d->m_lineEdit, &QLineEdit::textChanged, &d->m_proxyModel, &CommandBarFilterModel::setFilterString);
    connect(&d->m_lineEdit, &QLineEdit::textChanged, delegate, &CommandBarStyleDelegate::setFilterString);
    connect(&d->m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        d->m_treeView.viewport()->update();
        d->reselectFirst();
    });
    connect(&d->m_treeView, &QTreeView::clicked, this, [this]() {
        d->slotReturnPressed(this);
    });

    d->m_proxyModel.setSourceModel(&d->m_model);
    d->m_treeView.setSortingEnabled(true);
    d->m_treeView.setModel(&d->m_proxyModel);

    d->m_treeView.header()->setMinimumSectionSize(0);
    d->m_treeView.header()->setStretchLastSection(false);
    d->m_treeView.header()->setSectionResizeMode(KCommandBarModel::Column_Command, QHeaderView::Stretch);
    d->m_treeView.header()->setSectionResizeMode(KCommandBarModel::Column_Shortcut, QHeaderView::ResizeToContents);

    parent->installEventFilter(this);
    d->m_treeView.installEventFilter(this);
    d->m_lineEdit.installEventFilter(this);

    d->m_treeView.setHeaderHidden(true);
    d->m_treeView.setRootIsDecorated(false);
    d->m_treeView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_treeView.setSelectionMode(QTreeView::SingleSelection);

    QLabel *placeholderLabel = new QLabel;
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    placeholderLabel->setWordWrap(true);
    // To match the size of a level 2 Heading/KTitleWidget
    QFont placeholderLabelFont = placeholderLabel->font();
    placeholderLabelFont.setPointSize(qRound(placeholderLabelFont.pointSize() * 1.3));
    placeholderLabel->setFont(placeholderLabelFont);
    // Match opacity of QML placeholder label component
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(placeholderLabel);
    opacityEffect->setOpacity(0.5);
    placeholderLabel->setGraphicsEffect(opacityEffect);

    QHBoxLayout *placeholderLayout = new QHBoxLayout;
    placeholderLayout->addWidget(placeholderLabel);
    d->m_treeView.setLayout(placeholderLayout);

    connect(&d->m_proxyModel, &CommandBarFilterModel::modelReset, this, [this, placeholderLabel]() {
        if (d->m_proxyModel.rowCount() > 0) {
            placeholderLabel->hide();
        } else {
            if (d->m_model.rowCount() == 0) {
                placeholderLabel->setText(i18n("No commands to display"));
            } else {
                placeholderLabel->setText(i18n("No commands matching the filter"));
            }
            placeholderLabel->show();
        }
    });

    setHidden(true);

    // Migrate last used action config to new location
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("General"));
    if (cg.hasKey("CommandBarLastUsedActions")) {
        const QStringList actionNames = cg.readEntry("CommandBarLastUsedActions", QStringList());

        KConfigGroup stateCg(KSharedConfig::openStateConfig(), QStringLiteral("General"));
        stateCg.writeEntry(QStringLiteral("CommandBarLastUsedActions"), actionNames);

        cg.deleteEntry(QStringLiteral("CommandBarLastUsedActions"));
    }
}

/**
 * Destructor defined here to make unique_ptr work
 */
KCommandBar::~KCommandBar()
{
    auto lastUsedActions = d->lastUsedActions();
    auto cfg = KSharedConfig::openStateConfig();
    KConfigGroup cg(cfg, QStringLiteral("General"));
    cg.writeEntry("CommandBarLastUsedActions", lastUsedActions);

    // Explicitly remove installed event filters of children of d-pointer
    // class, otherwise while KCommandBar is being torn down, an event could
    // fire and the eventFilter() accesses d, which would cause a crash
    // bug 452527
    d->m_treeView.removeEventFilter(this);
    d->m_lineEdit.removeEventFilter(this);
}

void KCommandBar::setActions(const QList<ActionGroup> &actions)
{
    // First set last used actions in the model
    d->setLastUsedActions();

    d->m_model.refresh(actions);
    d->reselectFirst();

    show();
    setFocus();
}

void KCommandBar::show()
{
    const QRect boundingRect = getCommandBarBoundingRect(this);

    static constexpr int minWidth = 500;
    const int maxWidth = boundingRect.width();
    const int preferredWidth = maxWidth / 2.4;

    static constexpr int minHeight = 250;
    const int maxHeight = boundingRect.height();
    const int preferredHeight = maxHeight / 2;

    const QSize size{std::min(maxWidth, std::max(preferredWidth, minWidth)), std::min(maxHeight, std::max(preferredHeight, minHeight))};

    setFixedSize(size);

    // set the position to the top-center of the parent
    // just below the menubar/toolbar (if any)
    const QPoint position{boundingRect.center().x() - size.width() / 2, boundingRect.y() + 6};
    move(position);

    QWidget::show();
}

bool KCommandBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == &d->m_lineEdit) {
            const int key = keyEvent->key();
            const bool forward2list = (key == Qt::Key_Up) || (key == Qt::Key_Down) || (key == Qt::Key_PageUp) || (key == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(&d->m_treeView, event);
                return true;
            }
        } else if (obj == &d->m_treeView) {
            const int key = keyEvent->key();
            const bool forward2input = (key != Qt::Key_Up) && (key != Qt::Key_Down) && (key != Qt::Key_PageUp) && (key != Qt::Key_PageDown)
                && (key != Qt::Key_Tab) && (key != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(&d->m_lineEdit, event);
                return true;
            }
        }

        if (keyEvent->key() == Qt::Key_Escape) {
            hide();
            deleteLater();
            return true;
        }
    }

    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && isVisible() && !(d->m_lineEdit.hasFocus() || d->m_treeView.hasFocus())) {
        d->clearLineEdit();
        deleteLater();
        hide();
        return true;
    }

    // handle resizing
    if (parent() == obj && event->type() == QEvent::Resize) {
        show();
    }

    return QWidget::eventFilter(obj, event);
}
// END KCommandBar

#include "moc_kcommandbar.cpp"
