/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcharselect.h"
#include "kcharselect_p.h"

#include "loggingcategory.h"

#include <QAction>
#include <QActionEvent>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextBrowser>
#include <QTimer>
#include <QToolButton>

Q_GLOBAL_STATIC(KCharSelectData, s_data)

class KCharSelectTablePrivate
{
public:
    KCharSelectTablePrivate(KCharSelectTable *qq)
        : q(qq)
    {
    }

    KCharSelectTable *const q;

    QFont font;
    KCharSelectItemModel *model = nullptr;
    QList<uint> chars;
    uint chr = 0;

    void resizeCells();
    void doubleClicked(const QModelIndex &index);
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

class KCharSelectPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KCharSelect)

public:
    struct HistoryItem {
        uint c;
        bool fromSearch;
        QString searchString;
    };

    enum { MaxHistoryItems = 100 };

    KCharSelectPrivate(KCharSelect *qq)
        : q(qq)
    {
    }

    KCharSelect *const q;

    QToolButton *backButton = nullptr;
    QToolButton *forwardButton = nullptr;
    QLineEdit *searchLine = nullptr;
    QFontComboBox *fontCombo = nullptr;
    QSpinBox *fontSizeSpinBox = nullptr;
    QComboBox *sectionCombo = nullptr;
    QComboBox *blockCombo = nullptr;
    KCharSelectTable *charTable = nullptr;
    QTextBrowser *detailBrowser = nullptr;

    bool searchMode = false; // a search is active
    bool historyEnabled = false;
    bool allPlanesEnabled = false;
    int inHistory = 0; // index of current char in history
    QList<HistoryItem> history;
    QObject *actionParent = nullptr;

    QString createLinks(QString s);
    void historyAdd(uint c, bool fromSearch, const QString &searchString);
    void showFromHistory(int index);
    void updateBackForwardButtons();
    void activateSearchLine();
    void back();
    void forward();
    void fontSelected();
    void charSelected(uint c);
    void updateCurrentChar(uint c);
    void slotUpdateUnicode(uint c);
    void sectionSelected(int index);
    void blockSelected(int index);
    void searchEditChanged();
    void search();
    void linkClicked(QUrl url);
};

Q_DECLARE_TYPEINFO(KCharSelectPrivate::HistoryItem, Q_RELOCATABLE_TYPE);

/******************************************************************/
/* Class: KCharSelectTable                                        */
/******************************************************************/

KCharSelectTable::KCharSelectTable(QWidget *parent, const QFont &_font)
    : QTableView(parent)
    , d(new KCharSelectTablePrivate(this))
{
    d->font = _font;

    setTabKeyNavigation(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    QPalette _palette;
    _palette.setColor(backgroundRole(), palette().color(QPalette::Base));
    setPalette(_palette);
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Custom);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Custom);

    setFocusPolicy(Qt::StrongFocus);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setDragDropMode(QAbstractItemView::DragDrop);
    setTextElideMode(Qt::ElideNone);

    connect(this, &KCharSelectTable::doubleClicked, this, [this](const QModelIndex &index) {
        d->doubleClicked(index);
    });

    d->resizeCells();
}

KCharSelectTable::~KCharSelectTable() = default;

void KCharSelectTable::setFont(const QFont &_font)
{
    QTableView::setFont(_font);
    d->font = _font;
    if (d->model) {
        d->model->setFont(_font);
    }
    d->resizeCells();
}

uint KCharSelectTable::chr()
{
    return d->chr;
}

QFont KCharSelectTable::font() const
{
    return d->font;
}

QList<uint> KCharSelectTable::displayedChars() const
{
    return d->chars;
}

void KCharSelectTable::setChar(uint c)
{
    int pos = d->chars.indexOf(c);
    if (pos != -1) {
        setCurrentIndex(model()->index(pos / model()->columnCount(), pos % model()->columnCount()));
    }
}

void KCharSelectTable::setContents(const QList<uint> &chars)
{
    d->chars = chars;

    auto oldModel = d->model;
    d->model = new KCharSelectItemModel(chars, d->font, this);
    setModel(d->model);
    d->resizeCells();

    // Setting a model changes the selectionModel. Make sure to always reconnect.
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        d->slotSelectionChanged(selected, deselected);
    });

    connect(d->model, &KCharSelectItemModel::showCharRequested, this, &KCharSelectTable::showCharRequested);

    delete oldModel; // The selection model is thrown away when the model gets destroyed().
}

void KCharSelectTable::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    // this prevents horizontal scrolling when selecting a character in the last column
    if (index.isValid() && index.column() != 0) {
        QTableView::scrollTo(d->model->index(index.row(), 0), hint);
    } else {
        QTableView::scrollTo(index, hint);
    }
}

void KCharSelectTablePrivate::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if (!model || selected.indexes().isEmpty()) {
        return;
    }
    QVariant temp = model->data(selected.indexes().at(0), KCharSelectItemModel::CharacterRole);
    if (temp.userType() != QMetaType::UInt) {
        return;
    }
    uint c = temp.toUInt();
    chr = c;
    Q_EMIT q->focusItemChanged(c);
}

void KCharSelectTable::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e);
    if (e->size().width() != e->oldSize().width()) {
        // Resize our cells. But do so asynchronously through the event loop.
        // Otherwise we can end up with an infinite loop as resizing the cells in turn results in
        // a layout change which results in a resize event. More importantly doing this blockingly
        // crashes QAccessible as the resize we potentially cause will discard objects which are
        // still being used in the call chain leading to this event.
        // https://bugs.kde.org/show_bug.cgi?id=374933
        // https://bugreports.qt.io/browse/QTBUG-58153
        // This can be removed once a fixed Qt version is the lowest requirement for Frameworks.
        auto timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, [&, timer]() {
            d->resizeCells();
            timer->deleteLater();
        });
        timer->start(0);
    }
}

void KCharSelectTablePrivate::resizeCells()
{
    KCharSelectItemModel *model = static_cast<KCharSelectItemModel *>(q->model());
    if (!model) {
        return;
    }

    const int viewportWidth = q->viewport()->size().width();

    QFontMetrics fontMetrics(font);

    // Determine the max width of the displayed characters
    // fontMetrics.maxWidth() doesn't help because of font fallbacks
    // (testcase: Malayalam characters)
    int maxCharWidth = 0;
    const QList<uint> chars = model->chars();
    for (int i = 0; i < chars.size(); ++i) {
        char32_t thisChar = chars.at(i);
        if (s_data()->isPrint(thisChar)) {
            maxCharWidth = qMax(maxCharWidth, fontMetrics.boundingRect(QString::fromUcs4(&thisChar, 1)).width());
        }
    }
    // Avoid too narrow cells
    maxCharWidth = qMax(maxCharWidth, 2 * fontMetrics.xHeight());
    maxCharWidth = qMax(maxCharWidth, fontMetrics.height());
    // Add the necessary padding, trying to match the delegate
    const int textMargin = q->style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, q) + 1;
    maxCharWidth += 2 * textMargin;

    const int columns = qMax(1, viewportWidth / maxCharWidth);
    model->setColumnCount(columns);

    const uint oldChar = q->chr();

    const int new_w = viewportWidth / columns;
    const int rows = model->rowCount();
    q->setUpdatesEnabled(false);
    QHeaderView *hHeader = q->horizontalHeader();
    hHeader->setMinimumSectionSize(new_w);
    const int spaceLeft = viewportWidth - new_w * columns;
    for (int i = 0; i <= columns; ++i) {
        if (i < spaceLeft) {
            hHeader->resizeSection(i, new_w + 1);
        } else {
            hHeader->resizeSection(i, new_w);
        }
    }

    QHeaderView *vHeader = q->verticalHeader();
#ifdef Q_OS_WIN
    int new_h = fontMetrics.lineSpacing() + 1;
#else
    int new_h = fontMetrics.xHeight() * 3;
#endif
    const int fontHeight = fontMetrics.height();
    if (new_h < 5 || new_h < 4 + fontHeight) {
        new_h = qMax(5, 4 + fontHeight);
    }
    vHeader->setMinimumSectionSize(new_h);
    for (int i = 0; i < rows; ++i) {
        vHeader->resizeSection(i, new_h);
    }

    q->setUpdatesEnabled(true);
    q->setChar(oldChar);
}

void KCharSelectTablePrivate::doubleClicked(const QModelIndex &index)
{
    uint c = model->data(index, KCharSelectItemModel::CharacterRole).toUInt();
    if (s_data()->isPrint(c)) {
        Q_EMIT q->activated(c);
    }
}

void KCharSelectTable::keyPressEvent(QKeyEvent *e)
{
    if (d->model) {
        switch (e->key()) {
        case Qt::Key_Space:
            Q_EMIT activated(QChar::Space);
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            if (!currentIndex().isValid()) {
                return;
            }
            uint c = d->model->data(currentIndex(), KCharSelectItemModel::CharacterRole).toUInt();
            if (s_data()->isPrint(c)) {
                Q_EMIT activated(c);
            }
            return;
        }
        default:
            break;
        }
    }
    QTableView::keyPressEvent(e);
}

/******************************************************************/
/* Class: KCharSelect                                             */
/******************************************************************/

KCharSelect::KCharSelect(QWidget *parent, const Controls controls)
    : QWidget(parent)
    , d(new KCharSelectPrivate(this))
{
    initWidget(controls, nullptr);
}

KCharSelect::KCharSelect(QWidget *parent, QObject *actionParent, const Controls controls)
    : QWidget(parent)
    , d(new KCharSelectPrivate(this))
{
    initWidget(controls, actionParent);
}

void attachToActionParent(QAction *action, QObject *actionParent, const QList<QKeySequence> &shortcuts)
{
    if (!action || !actionParent) {
        return;
    }

    action->setParent(actionParent);

    if (actionParent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(actionParent, "addAction", Q_ARG(QString, action->objectName()), Q_ARG(QAction *, action));
        QMetaObject::invokeMethod(actionParent, "setDefaultShortcuts", Q_ARG(QAction *, action), Q_ARG(QList<QKeySequence>, shortcuts));
    } else {
        action->setShortcuts(shortcuts);
    }
}

void KCharSelect::initWidget(const Controls controls, QObject *actionParent)
{
    d->actionParent = actionParent;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    if (SearchLine & controls) {
        QHBoxLayout *searchLayout = new QHBoxLayout();
        mainLayout->addLayout(searchLayout);
        d->searchLine = new QLineEdit(this);
        searchLayout->addWidget(d->searchLine);
        d->searchLine->setPlaceholderText(tr("Enter a search term or character…", "@info:placeholder"));
        d->searchLine->setClearButtonEnabled(true);
        d->searchLine->setToolTip(tr("Enter a search term or character here", "@info:tooltip"));

        QAction *findAction = new QAction(this);
        connect(findAction, &QAction::triggered, this, [this]() {
            d->activateSearchLine();
        });
        findAction->setObjectName(QStringLiteral("edit_find"));
        findAction->setText(tr("&Find…", "@action"));
        findAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
        attachToActionParent(findAction, actionParent, QKeySequence::keyBindings(QKeySequence::Find));

        connect(d->searchLine, &QLineEdit::textChanged, this, [this]() {
            d->searchEditChanged();
        });
        connect(d->searchLine, &QLineEdit::returnPressed, this, [this]() {
            d->search();
        });
    }

    if ((SearchLine & controls) && ((FontCombo & controls) || (FontSize & controls) || (BlockCombos & controls))) {
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line);
    }

    QHBoxLayout *comboLayout = new QHBoxLayout();

    d->backButton = new QToolButton(this);
    comboLayout->addWidget(d->backButton);
    d->backButton->setEnabled(false);
    d->backButton->setText(tr("Previous in History", "@action:button Goes to previous character"));
    d->backButton->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    d->backButton->setToolTip(tr("Go to previous character in history", "@info:tooltip"));

    d->forwardButton = new QToolButton(this);
    comboLayout->addWidget(d->forwardButton);
    d->forwardButton->setEnabled(false);
    d->forwardButton->setText(tr("Next in History", "@action:button Goes to next character"));
    d->forwardButton->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    d->forwardButton->setToolTip(tr("Go to next character in history", "info:tooltip"));

    QAction *backAction = new QAction(this);
    connect(backAction, &QAction::triggered, d->backButton, &QAbstractButton::animateClick);
    backAction->setObjectName(QStringLiteral("go_back"));
    backAction->setText(tr("&Back", "@action go back"));
    backAction->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    attachToActionParent(backAction, actionParent, QKeySequence::keyBindings(QKeySequence::Back));

    QAction *forwardAction = new QAction(this);
    connect(forwardAction, &QAction::triggered, d->forwardButton, &QAbstractButton::animateClick);
    forwardAction->setObjectName(QStringLiteral("go_forward"));
    forwardAction->setText(tr("&Forward", "@action go forward"));
    forwardAction->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    attachToActionParent(forwardAction, actionParent, QKeySequence::keyBindings(QKeySequence::Forward));

    if (QApplication::isRightToLeft()) { // swap the back/forward icons
        QIcon tmp = backAction->icon();
        backAction->setIcon(forwardAction->icon());
        forwardAction->setIcon(tmp);
    }

    connect(d->backButton, &QToolButton::clicked, this, [this]() {
        d->back();
    });
    connect(d->forwardButton, &QToolButton::clicked, this, [this]() {
        d->forward();
    });

    d->sectionCombo = new QComboBox(this);
    d->sectionCombo->setObjectName(QStringLiteral("sectionCombo"));
    d->sectionCombo->setToolTip(tr("Select a category", "@info:tooltip"));
    comboLayout->addWidget(d->sectionCombo);
    d->blockCombo = new QComboBox(this);
    d->blockCombo->setObjectName(QStringLiteral("blockCombo"));
    d->blockCombo->setToolTip(tr("Select a block to be displayed", "@info:tooltip"));
    d->blockCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    comboLayout->addWidget(d->blockCombo, 1);
    QStringList sectionList = s_data()->sectionList();
    d->sectionCombo->addItems(sectionList);
    d->blockCombo->setMinimumWidth(QFontMetrics(QWidget::font()).averageCharWidth() * 25);

    connect(d->sectionCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        d->sectionSelected(index);
    });

    connect(d->blockCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        d->blockSelected(index);
    });

    d->fontCombo = new QFontComboBox(this);
    comboLayout->addWidget(d->fontCombo);
    d->fontCombo->setEditable(true);
    d->fontCombo->resize(d->fontCombo->sizeHint());
    d->fontCombo->setToolTip(tr("Set font", "@info:tooltip"));

    d->fontSizeSpinBox = new QSpinBox(this);
    comboLayout->addWidget(d->fontSizeSpinBox);
    d->fontSizeSpinBox->setValue(QWidget::font().pointSize());
    d->fontSizeSpinBox->setRange(1, 400);
    d->fontSizeSpinBox->setSingleStep(1);
    d->fontSizeSpinBox->setToolTip(tr("Set font size", "@info:tooltip"));

    connect(d->fontCombo, &QFontComboBox::currentFontChanged, this, [this]() {
        d->fontSelected();
    });
    connect(d->fontSizeSpinBox, &QSpinBox::valueChanged, this, [this]() {
        d->fontSelected();
    });

    if ((HistoryButtons & controls) || (FontCombo & controls) || (FontSize & controls) || (BlockCombos & controls)) {
        mainLayout->addLayout(comboLayout);
    }
    if (!(HistoryButtons & controls)) {
        d->backButton->hide();
        d->forwardButton->hide();
    }
    if (!(FontCombo & controls)) {
        d->fontCombo->hide();
    }
    if (!(FontSize & controls)) {
        d->fontSizeSpinBox->hide();
    }
    if (!(BlockCombos & controls)) {
        d->sectionCombo->hide();
        d->blockCombo->hide();
    }

    QSplitter *splitter = new QSplitter(this);
    if ((CharacterTable & controls) || (DetailBrowser & controls)) {
        mainLayout->addWidget(splitter);
    } else {
        splitter->hide();
    }
    d->charTable = new KCharSelectTable(this, QFont());
    if (CharacterTable & controls) {
        splitter->addWidget(d->charTable);
    } else {
        d->charTable->hide();
    }

    const QSize sz(200, 200);
    d->charTable->resize(sz);
    d->charTable->setMinimumSize(sz);

    d->charTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCurrentFont(QFont());

    connect(d->charTable, &KCharSelectTable::focusItemChanged, this, [this](uint c) {
        d->updateCurrentChar(c);
    });
    connect(d->charTable, &KCharSelectTable::activated, this, [this](uint c) {
        d->charSelected(c);
    });
    connect(d->charTable, &KCharSelectTable::showCharRequested, this, &KCharSelect::setCurrentCodePoint);

    d->detailBrowser = new QTextBrowser(this);
    if (DetailBrowser & controls) {
        splitter->addWidget(d->detailBrowser);
    } else {
        d->detailBrowser->hide();
    }
    d->detailBrowser->setOpenLinks(false);
    connect(d->detailBrowser, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
        d->linkClicked(url);
    });

    setFocusPolicy(Qt::StrongFocus);
    if (SearchLine & controls) {
        setFocusProxy(d->searchLine);
    } else {
        setFocusProxy(d->charTable);
    }

    d->sectionSelected(1); // this will also call blockSelected(0)
    setCurrentCodePoint(QChar::Null);

    d->historyEnabled = true;
}

KCharSelect::~KCharSelect() = default;

QSize KCharSelect::sizeHint() const
{
    return QWidget::sizeHint();
}

void KCharSelect::setCurrentFont(const QFont &_font)
{
    d->fontCombo->setCurrentFont(_font);
    d->fontSizeSpinBox->setValue(_font.pointSize());
    d->fontSelected();
}

void KCharSelect::setAllPlanesEnabled(bool all)
{
    d->allPlanesEnabled = all;
}

bool KCharSelect::allPlanesEnabled() const
{
    return d->allPlanesEnabled;
}

QChar KCharSelect::currentChar() const
{
    if (d->allPlanesEnabled) {
        qFatal("You must use KCharSelect::currentCodePoint instead of KCharSelect::currentChar");
    }
    return QChar(d->charTable->chr());
}

uint KCharSelect::currentCodePoint() const
{
    return d->charTable->chr();
}

QFont KCharSelect::currentFont() const
{
    return d->charTable->font();
}

QList<QChar> KCharSelect::displayedChars() const
{
    if (d->allPlanesEnabled) {
        qFatal("You must use KCharSelect::displayedCodePoints instead of KCharSelect::displayedChars");
    }
    QList<QChar> result;
    const auto displayedChars = d->charTable->displayedChars();
    result.reserve(displayedChars.size());
    for (uint c : displayedChars) {
        result.append(QChar(c));
    }
    return result;
}

QList<uint> KCharSelect::displayedCodePoints() const
{
    return d->charTable->displayedChars();
}

void KCharSelect::setCurrentChar(const QChar &c)
{
    if (d->allPlanesEnabled) {
        qCritical("You should use KCharSelect::setCurrentCodePoint instead of KCharSelect::setCurrentChar");
    }
    setCurrentCodePoint(c.unicode());
}

void KCharSelect::setCurrentCodePoint(uint c)
{
    if (!d->allPlanesEnabled && QChar::requiresSurrogates(c)) {
        qCritical("You must setAllPlanesEnabled(true) to use non-BMP characters");
        c = QChar::ReplacementCharacter;
    }
    if (c > QChar::LastValidCodePoint) {
        qCWarning(KWidgetsAddonsLog, "Code point outside Unicode range");
        c = QChar::LastValidCodePoint;
    }
    bool oldHistoryEnabled = d->historyEnabled;
    d->historyEnabled = false;
    int block = s_data()->blockIndex(c);
    int section = s_data()->sectionIndex(block);
    d->sectionCombo->setCurrentIndex(section);
    int index = d->blockCombo->findData(block);
    if (index != -1) {
        d->blockCombo->setCurrentIndex(index);
    }
    d->historyEnabled = oldHistoryEnabled;
    d->charTable->setChar(c);
}

void KCharSelectPrivate::historyAdd(uint c, bool fromSearch, const QString &searchString)
{
    // qCDebug(KWidgetsAddonsLog) << "about to add char" << c << "fromSearch" << fromSearch << "searchString" << searchString;

    if (!historyEnabled) {
        return;
    }

    if (!history.isEmpty() && c == history.last().c) {
        // avoid duplicates
        return;
    }

    // behave like a web browser, i.e. if user goes back from B to A then clicks C, B is forgotten
    while (!history.isEmpty() && inHistory != history.count() - 1) {
        history.removeLast();
    }

    while (history.size() >= MaxHistoryItems) {
        history.removeFirst();
    }

    HistoryItem item;
    item.c = c;
    item.fromSearch = fromSearch;
    item.searchString = searchString;
    history.append(item);

    inHistory = history.count() - 1;
    updateBackForwardButtons();
}

void KCharSelectPrivate::showFromHistory(int index)
{
    Q_ASSERT(index >= 0 && index < history.count());
    Q_ASSERT(index != inHistory);

    inHistory = index;
    updateBackForwardButtons();

    const HistoryItem &item = history[index];
    // qCDebug(KWidgetsAddonsLog) << "index" << index << "char" << item.c << "fromSearch" << item.fromSearch
    //    << "searchString" << item.searchString;

    // avoid adding an item from history into history again
    bool oldHistoryEnabled = historyEnabled;
    historyEnabled = false;
    if (item.fromSearch) {
        if (searchLine->text() != item.searchString) {
            searchLine->setText(item.searchString);
            search();
        }
        charTable->setChar(item.c);
    } else {
        searchLine->clear();
        q->setCurrentCodePoint(item.c);
    }
    historyEnabled = oldHistoryEnabled;
}

void KCharSelectPrivate::updateBackForwardButtons()
{
    backButton->setEnabled(inHistory > 0);
    forwardButton->setEnabled(inHistory < history.count() - 1);
}

void KCharSelectPrivate::activateSearchLine()
{
    searchLine->setFocus();
    searchLine->selectAll();
}

void KCharSelectPrivate::back()
{
    Q_ASSERT(inHistory > 0);
    showFromHistory(inHistory - 1);
}

void KCharSelectPrivate::forward()
{
    Q_ASSERT(inHistory + 1 < history.count());
    showFromHistory(inHistory + 1);
}

void KCharSelectPrivate::fontSelected()
{
    QFont font = fontCombo->currentFont();
    font.setPointSize(fontSizeSpinBox->value());
    charTable->setFont(font);
    Q_EMIT q->currentFontChanged(font);
}

void KCharSelectPrivate::charSelected(uint c)
{
    if (!allPlanesEnabled) {
        Q_EMIT q->charSelected(QChar(c));
    }
    Q_EMIT q->codePointSelected(c);
}

void KCharSelectPrivate::updateCurrentChar(uint c)
{
    if (!allPlanesEnabled) {
        Q_EMIT q->currentCharChanged(QChar(c));
    }
    Q_EMIT q->currentCodePointChanged(c);
    if (searchMode || sectionCombo->currentIndex() == 0) {
        // we are in search mode or all characters are shown. make the two comboboxes show the section & block for this character (only the blockCombo for the
        // all characters mode).
        //(when we are not in search mode nor in the all characters mode the current character always belongs to the current section & block.)
        int block = s_data()->blockIndex(c);
        if (searchMode) {
            int section = s_data()->sectionIndex(block);
            sectionCombo->setCurrentIndex(section);
        }
        int index = blockCombo->findData(block);
        if (index != -1) {
            blockCombo->setCurrentIndex(index);
        }
    }

    if (searchLine) {
        historyAdd(c, searchMode, searchLine->text());
    }

    slotUpdateUnicode(c);
}

void KCharSelectPrivate::slotUpdateUnicode(uint c)
{
    QString html = QLatin1String("<p>") + tr("Character:") + QLatin1Char(' ') + s_data()->display(c, charTable->font()) + QLatin1Char(' ')
        + s_data()->formatCode(c) + QLatin1String("<br />");

    QString name = s_data()->name(c);
    if (!name.isEmpty()) {
        // is name ever empty? </p> should always be there...
        html += tr("Name: ") + name.toHtmlEscaped() + QLatin1String("</p>");
    }
    const QStringList aliases = s_data()->aliases(c);
    const QStringList notes = s_data()->notes(c);
    const QList<uint> seeAlso = s_data()->seeAlso(c);
    const QStringList equivalents = s_data()->equivalents(c);
    const QStringList approxEquivalents = s_data()->approximateEquivalents(c);
    const QList<uint> decomposition = s_data()->decomposition(c);
    if (!(aliases.isEmpty() && notes.isEmpty() && seeAlso.isEmpty() && equivalents.isEmpty() && approxEquivalents.isEmpty() && decomposition.isEmpty())) {
        html += QLatin1String("<p><b>") + tr("Annotations and Cross References") + QLatin1String("</b></p>");
    }

    if (!aliases.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("Alias names:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (const QString &alias : aliases) {
            html += QLatin1String("<li>") + alias.toHtmlEscaped() + QLatin1String("</li>");
        }
        html += QLatin1String("</ul>");
    }

    if (!notes.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("Notes:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (const QString &note : notes) {
            html += QLatin1String("<li>") + createLinks(note.toHtmlEscaped()) + QLatin1String("</li>");
        }
        html += QLatin1String("</ul>");
    }

    if (!seeAlso.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("See also:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (uint c2 : seeAlso) {
            if (!allPlanesEnabled && QChar::requiresSurrogates(c2)) {
                continue;
            }
            html += QLatin1String("<li><a href=\"") + QString::number(c2, 16) + QLatin1String("\">");
            if (s_data()->isPrint(c2)) {
                html += QLatin1String("&#8206;&#") + QString::number(c2) + QLatin1String("; ");
            }
            html += s_data()->formatCode(c2) + QLatin1Char(' ') + s_data()->name(c2).toHtmlEscaped() + QLatin1String("</a></li>");
        }
        html += QLatin1String("</ul>");
    }

    if (!equivalents.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("Equivalents:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (const QString &equivalent : equivalents) {
            html += QLatin1String("<li>") + createLinks(equivalent.toHtmlEscaped()) + QLatin1String("</li>");
        }
        html += QLatin1String("</ul>");
    }

    if (!approxEquivalents.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("Approximate equivalents:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (const QString &approxEquivalent : approxEquivalents) {
            html += QLatin1String("<li>") + createLinks(approxEquivalent.toHtmlEscaped()) + QLatin1String("</li>");
        }
        html += QLatin1String("</ul>");
    }

    if (!decomposition.isEmpty()) {
        html += QLatin1String("<p style=\"margin-bottom: 0px;\">") + tr("Decomposition:") + QLatin1String("</p><ul style=\"margin-top: 0px;\">");
        for (uint c2 : decomposition) {
            if (!allPlanesEnabled && QChar::requiresSurrogates(c2)) {
                continue;
            }
            html += QLatin1String("<li>") + createLinks(s_data()->formatCode(c2, 4, QString())) + QLatin1String("</li>");
        }
        html += QLatin1String("</ul>");
    }

    QStringList unihan = s_data()->unihanInfo(c);
    if (unihan.count() == 7) {
        html += QLatin1String("<p><b>") + tr("CJK Ideograph Information") + QLatin1String("</b></p><p>");
        bool newline = true;
        if (!unihan[0].isEmpty()) {
            html += tr("Definition in English: ") + unihan[0];
            newline = false;
        }
        if (!unihan[2].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Mandarin Pronunciation: ") + unihan[2];
            newline = false;
        }
        if (!unihan[1].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Cantonese Pronunciation: ") + unihan[1];
            newline = false;
        }
        if (!unihan[6].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Japanese On Pronunciation: ") + unihan[6];
            newline = false;
        }
        if (!unihan[5].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Japanese Kun Pronunciation: ") + unihan[5];
            newline = false;
        }
        if (!unihan[3].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Tang Pronunciation: ") + unihan[3];
            newline = false;
        }
        if (!unihan[4].isEmpty()) {
            if (!newline) {
                html += QLatin1String("<br>");
            }
            html += tr("Korean Pronunciation: ") + unihan[4];
            newline = false;
        }
        html += QLatin1String("</p>");
    }

    html += QLatin1String("<p><b>") + tr("General Character Properties") + QLatin1String("</b><br>");
    html += tr("Block: ") + s_data()->block(c) + QLatin1String("<br>");
    html += tr("Unicode category: ") + s_data()->categoryText(s_data()->category(c)) + QLatin1String("</p>");

    const QByteArray utf8 = QString::fromUcs4(reinterpret_cast<char32_t *>(&c), 1).toUtf8();

    html += QLatin1String("<p><b>") + tr("Various Useful Representations") + QLatin1String("</b><br>");
    html += tr("UTF-8:");
    for (unsigned char c : utf8) {
        html += QLatin1Char(' ') + s_data()->formatCode(c, 2, QStringLiteral("0x"));
    }
    html += QLatin1String("<br>") + tr("UTF-16: ");
    if (QChar::requiresSurrogates(c)) {
        html += s_data()->formatCode(QChar::highSurrogate(c), 4, QStringLiteral("0x"));
        html += QLatin1Char(' ') + s_data->formatCode(QChar::lowSurrogate(c), 4, QStringLiteral("0x"));
    } else {
        html += s_data()->formatCode(c, 4, QStringLiteral("0x"));
    }
    html += QLatin1String("<br>") + tr("C octal escaped UTF-8: ");
    for (unsigned char c : utf8) {
        html += s_data()->formatCode(c, 3, QStringLiteral("\\"), 8);
    }
    html += QLatin1String("<br>") + tr("XML decimal entity:") + QLatin1String(" &amp;#") + QString::number(c) + QLatin1String(";</p>");

    detailBrowser->setHtml(html);
}

QString KCharSelectPrivate::createLinks(QString s)
{
    static const QRegularExpression rx(QStringLiteral("\\b([\\dABCDEF]{4,5})\\b"), QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatchIterator iter = rx.globalMatch(s);
    QRegularExpressionMatch match;
    QSet<QString> chars;
    while (iter.hasNext()) {
        match = iter.next();
        chars.insert(match.captured(1));
    }

    for (const QString &c : std::as_const(chars)) {
        int unicode = c.toInt(nullptr, 16);
        if (!allPlanesEnabled && QChar::requiresSurrogates(unicode)) {
            continue;
        }
        QString link = QLatin1String("<a href=\"") + c + QLatin1String("\">");
        if (s_data()->isPrint(unicode)) {
            link += QLatin1String("&#8206;&#") + QString::number(unicode) + QLatin1String(";&nbsp;");
        }
        link += QLatin1String("U+") + c + QLatin1Char(' ');
        link += s_data()->name(unicode).toHtmlEscaped() + QLatin1String("</a>");
        s.replace(c, link);
    }
    return s;
}

void KCharSelectPrivate::sectionSelected(int index)
{
    blockCombo->clear();
    QList<uint> chars;
    const QList<int> blocks = s_data()->sectionContents(index);
    for (int block : blocks) {
        if (!allPlanesEnabled) {
            const QList<uint> contents = s_data()->blockContents(block);
            if (!contents.isEmpty() && QChar::requiresSurrogates(contents.at(0))) {
                continue;
            }
        }
        blockCombo->addItem(s_data()->blockName(block), QVariant(block));
        if (index == 0) {
            chars << s_data()->blockContents(block);
        }
    }
    if (index == 0) {
        charTable->setContents(chars);
        updateCurrentChar(charTable->chr());
    } else {
        blockCombo->setCurrentIndex(0);
    }
}

void KCharSelectPrivate::blockSelected(int index)
{
    if (index == -1) {
        // the combo box has been cleared and is about to be filled again (because the section has changed)
        return;
    }
    if (searchMode) {
        // we are in search mode, so don't fill the table with this block.
        return;
    }
    int block = blockCombo->itemData(index).toInt();
    if (sectionCombo->currentIndex() == 0 && block == s_data()->blockIndex(charTable->chr())) {
        // the selected block already contains the selected character
        return;
    }
    const QList<uint> contents = s_data()->blockContents(block);
    if (sectionCombo->currentIndex() > 0) {
        charTable->setContents(contents);
    }
    Q_EMIT q->displayedCharsChanged();
    charTable->setChar(contents[0]);
}

void KCharSelectPrivate::searchEditChanged()
{
    if (searchLine->text().isEmpty()) {
        sectionCombo->setEnabled(true);
        blockCombo->setEnabled(true);

        // upon leaving search mode, keep the same character selected
        searchMode = false;
        uint c = charTable->chr();
        bool oldHistoryEnabled = historyEnabled;
        historyEnabled = false;
        blockSelected(blockCombo->currentIndex());
        historyEnabled = oldHistoryEnabled;
        q->setCurrentCodePoint(c);
    } else {
        sectionCombo->setEnabled(false);
        blockCombo->setEnabled(false);

        int length = searchLine->text().length();
        if (length >= 3) {
            search();
        }
    }
}

void KCharSelectPrivate::search()
{
    if (searchLine->text().isEmpty()) {
        return;
    }
    searchMode = true;
    QList<uint> contents = s_data()->find(searchLine->text());
    if (!allPlanesEnabled) {
        contents.erase(std::remove_if(contents.begin(), contents.end(), QChar::requiresSurrogates), contents.end());
    }

    charTable->setContents(contents);
    Q_EMIT q->displayedCharsChanged();
    if (!contents.isEmpty()) {
        charTable->setChar(contents[0]);
    }
}

void KCharSelectPrivate::linkClicked(QUrl url)
{
    QString hex = url.toString();
    if (hex.size() > 6) {
        return;
    }
    int unicode = hex.toInt(nullptr, 16);
    if (unicode > QChar::LastValidCodePoint) {
        return;
    }
    searchLine->clear();
    q->setCurrentCodePoint(unicode);
}

////

QVariant KCharSelectItemModel::data(const QModelIndex &index, int role) const
{
    int pos = m_columns * (index.row()) + index.column();
    if (!index.isValid() || pos < 0 || pos >= m_chars.size() || index.row() < 0 || index.column() < 0) {
        if (role == Qt::BackgroundRole) {
            return QVariant(qApp->palette().color(QPalette::Button));
        }
        return QVariant();
    }

    char32_t c = m_chars[pos];
    if (role == Qt::ToolTipRole) {
        QString result = s_data()->display(c, m_font) + QLatin1String("<br />") + s_data()->name(c).toHtmlEscaped() + QLatin1String("<br />")
            + tr("Unicode code point:") + QLatin1Char(' ') + s_data()->formatCode(c) + QLatin1String("<br />") + tr("In decimal", "Character")
            + QLatin1Char(' ') + QString::number(c);
        return QVariant(result);
    } else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        if (s_data()->isPrint(c)) {
            return QVariant(QString::fromUcs4(&c, 1));
        }
        return QVariant();
    } else if (role == Qt::BackgroundRole) {
        QFontMetrics fm = QFontMetrics(m_font);
        if (fm.inFontUcs4(c) && s_data()->isPrint(c)) {
            return QVariant(qApp->palette().color(QPalette::Base));
        } else {
            return QVariant(qApp->palette().color(QPalette::Button));
        }
    } else if (role == Qt::FontRole) {
        return QVariant(m_font);
    } else if (role == CharacterRole) {
        return QVariant(c);
    }
    return QVariant();
}

bool KCharSelectItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(parent)
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasText()) {
        return false;
    }

    if (column > 0) {
        return false;
    }
    QString text = data->text();
    if (text.isEmpty()) {
        return false;
    }
    Q_EMIT showCharRequested(text.toUcs4().at(0));
    return true;
}

void KCharSelectItemModel::setColumnCount(int columns)
{
    if (columns == m_columns) {
        return;
    }
    Q_EMIT layoutAboutToBeChanged();
    m_columns = columns;
    Q_EMIT layoutChanged();
}

#include "moc_kcharselect.cpp"
#include "moc_kcharselect_p.cpp"
