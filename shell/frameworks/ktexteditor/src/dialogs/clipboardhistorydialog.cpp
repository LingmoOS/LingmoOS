/*
    SPDX-FileCopyrightText: 2022 Eric Armbruster <eric1@armbruster-online.de>
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clipboardhistorydialog.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateview.h"

#include <QBoxLayout>
#include <QCoreApplication>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMimeDatabase>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KTextEditor/Editor>

class ClipboardHistoryModel : public QAbstractTableModel
{
public:
    enum Role { HighlightingRole = Qt::UserRole + 1, OriginalSorting };

    explicit ClipboardHistoryModel(QObject *parent)
        : QAbstractTableModel(parent)
    {
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return m_modelEntries.size();
    }

    int columnCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &idx, int role) const override
    {
        if (!idx.isValid()) {
            return {};
        }

        const ClipboardEntry &clipboardEntry = m_modelEntries.at(idx.row());
        if (role == Qt::DisplayRole) {
            return clipboardEntry.text;
        } else if (role == Role::HighlightingRole) {
            return clipboardEntry.fileName;
        } else if (role == Qt::DecorationRole) {
            return clipboardEntry.icon;
        } else if (role == Role::OriginalSorting) {
            return clipboardEntry.dateSort;
        }

        return {};
    }

    void refresh(const QList<KTextEditor::EditorPrivate::ClipboardEntry> &clipboardEntry)
    {
        QList<ClipboardEntry> temp;

        for (int i = 0; i < clipboardEntry.size(); ++i) {
            const auto entry = clipboardEntry.at(i);

            auto icon = QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(entry.fileName).iconName());
            if (icon.isNull()) {
                icon = QIcon::fromTheme(QStringLiteral("text-plain"));
            }

            temp.append({.text=entry.text, .fileName=entry.fileName, .icon=icon, .dateSort=i});
        }

        beginResetModel();
        m_modelEntries = std::move(temp);
        endResetModel();
    }

    void clear()
    {
        beginResetModel();
        QList<ClipboardEntry>().swap(m_modelEntries);
        endResetModel();
    }

private:
    struct ClipboardEntry {
        QString text;
        QString fileName;
        QIcon icon;
        int dateSort;
    };

    QList<ClipboardEntry> m_modelEntries;
};

class ClipboardHistoryFilterModel : public QSortFilterProxyModel
{
public:
    explicit ClipboardHistoryFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        const int l = sourceLeft.data(ClipboardHistoryModel::OriginalSorting).toInt();
        const int r = sourceRight.data(ClipboardHistoryModel::OriginalSorting).toInt();
        return l > r;
    }
};

class SingleLineDelegate : public QStyledItemDelegate
{
public:
    explicit SingleLineDelegate(const QFont &font)
        : QStyledItemDelegate(nullptr)
        , m_font(font)
        , m_newLineRegExp(QStringLiteral("\\n|\\r|\u2028"), QRegularExpression::UseUnicodePropertiesOption)
    {
    }

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->font = m_font;
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
        QString baseText = QStyledItemDelegate::displayText(value, locale).trimmed();
        auto endOfLine = baseText.indexOf(m_newLineRegExp, 0);
        if (endOfLine != -1) {
            baseText.truncate(endOfLine);
        }

        return baseText;
    }

private:
    QFont m_font;
    QRegularExpression m_newLineRegExp;
};

ClipboardHistoryDialog::ClipboardHistoryDialog(QWidget *mainWindow, KTextEditor::ViewPrivate *viewPrivate)
    : QMenu(mainWindow)
    , m_mainWindow(mainWindow)
    , m_viewPrivate(viewPrivate)
    , m_model(new ClipboardHistoryModel(this))
    , m_proxyModel(new ClipboardHistoryFilterModel(this))
    , m_selectedDoc(new KTextEditor::DocumentPrivate)
{
    // --------------------------------------------------
    // start of copy from Kate quickdialog.cpp (slight changes)
    // --------------------------------------------------

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    setFocusProxy(&m_lineEdit);

    layout->addWidget(&m_lineEdit);

    layout->addWidget(&m_treeView, 2);
    m_treeView.setTextElideMode(Qt::ElideLeft);
    m_treeView.setUniformRowHeights(true);

    connect(&m_lineEdit, &QLineEdit::returnPressed, this, &ClipboardHistoryDialog::slotReturnPressed);
    // user can add this as necessary
    //    connect(m_lineEdit, &QLineEdit::textChanged, delegate, &StyleDelegate::setFilterString);
    connect(&m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView.viewport()->update();
    });
    connect(&m_treeView, &QTreeView::doubleClicked, this, &ClipboardHistoryDialog::slotReturnPressed);
    m_treeView.setSortingEnabled(true);

    m_treeView.setHeaderHidden(true);
    m_treeView.setRootIsDecorated(false);
    m_treeView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView.setSelectionMode(QTreeView::SingleSelection);

    updateViewGeometry();
    setFocus();

    // --------------------------------------------------
    // end of copy from Kate quickdialog.cpp
    // --------------------------------------------------

    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    const QFont font = viewPrivate->rendererConfig()->baseFont();

    m_treeView.setModel(m_proxyModel);
    m_treeView.setItemDelegate(new SingleLineDelegate(font));
    m_treeView.setTextElideMode(Qt::ElideRight);

    m_selectedDoc->setParent(this);
    m_selectedView = new KTextEditor::ViewPrivate(m_selectedDoc, this);
    m_selectedView->setStatusBarEnabled(false);
    m_selectedView->setLineNumbersOn(false);
    m_selectedView->setFoldingMarkersOn(false);
    m_selectedView->setIconBorder(false);
    m_selectedView->setScrollBarMarks(false);
    m_selectedView->setScrollBarMiniMap(false);

    layout->addWidget(m_selectedView, 3);

    m_lineEdit.setFont(font);

    connect(m_treeView.selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current, const QModelIndex &previous) {
        Q_UNUSED(previous);
        showSelectedText(current);
    });

    connect(&m_lineEdit, &QLineEdit::textChanged, this, [this](const QString &s) {
        m_proxyModel->setFilterFixedString(s);

        const auto bestMatch = m_proxyModel->index(0, 0);
        m_treeView.setCurrentIndex(bestMatch);
        showSelectedText(bestMatch);
    });

    m_treeView.installEventFilter(this);
    m_lineEdit.installEventFilter(this);
    m_selectedView->installEventFilter(this);
}

void ClipboardHistoryDialog::showSelectedText(const QModelIndex &idx)
{
    QString text = m_proxyModel->data(idx, Qt::DisplayRole).toString();
    if (m_selectedDoc->text().isEmpty() || text != m_selectedDoc->text()) {
        QString fileName = m_proxyModel->data(idx, ClipboardHistoryModel::Role::HighlightingRole).toString();
        m_selectedDoc->setReadWrite(true);
        m_selectedDoc->setText(text);
        m_selectedDoc->setReadWrite(false);
        const auto mode = KTextEditor::Editor::instance()->repository().definitionForFileName(fileName).name();
        m_selectedDoc->setHighlightingMode(mode);
    }
}

void ClipboardHistoryDialog::resetValues()
{
    m_lineEdit.setPlaceholderText(i18n("Select text to paste."));
}

void ClipboardHistoryDialog::openDialog(const QList<KTextEditor::EditorPrivate::ClipboardEntry> &clipboardHistory)
{
    m_model->refresh(clipboardHistory);
    resetValues();

    if (m_model->rowCount(m_model->index(-1, -1)) == 0) {
        showEmptyPlaceholder();
    } else {
        const auto first = m_proxyModel->index(0, 0);
        m_treeView.setCurrentIndex(first);
        showSelectedText(first);
    }

    exec();
}

void ClipboardHistoryDialog::showEmptyPlaceholder()
{
    QVBoxLayout *noRecentsLayout = new QVBoxLayout(&m_treeView);
    m_treeView.setLayout(noRecentsLayout);
    m_noEntries = new QLabel(&m_treeView);
    QFont placeholderLabelFont;
    // To match the size of a level 2 Heading/KTitleWidget
    placeholderLabelFont.setPointSize(qRound(placeholderLabelFont.pointSize() * 1.3));
    noRecentsLayout->addWidget(m_noEntries);
    m_noEntries->setFont(placeholderLabelFont);
    m_noEntries->setTextInteractionFlags(Qt::NoTextInteraction);
    m_noEntries->setWordWrap(true);
    m_noEntries->setAlignment(Qt::AlignCenter);
    m_noEntries->setText(i18n("No entries in clipboard history"));
    // Match opacity of QML placeholder label component
    auto *effect = new QGraphicsOpacityEffect(m_noEntries);
    effect->setOpacity(0.5);
    m_noEntries->setGraphicsEffect(effect);
}

// --------------------------------------------------
// start of copy from Kate quickdialog.cpp
// --------------------------------------------------

void ClipboardHistoryDialog::slotReturnPressed()
{
    const QString text = m_proxyModel->data(m_treeView.currentIndex(), Qt::DisplayRole).toString();
    m_viewPrivate->paste(&text);

    clearLineEdit();
    hide();
}

bool ClipboardHistoryDialog::eventFilter(QObject *obj, QEvent *event)
{
    // catch key presses + shortcut overrides to allow to have ESC as application wide shortcut, too, see bug 409856
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == &m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(&m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                clearLineEdit();
                keyEvent->accept();
                hide();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(&m_lineEdit, event);
                return true;
            }
        }
    }

    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && !(m_lineEdit.hasFocus() || m_treeView.hasFocus() || m_selectedView->hasFocus())) {
        clearLineEdit();
        hide();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void ClipboardHistoryDialog::updateViewGeometry()
{
    if (!m_mainWindow)
        return;

    const QSize centralSize = m_mainWindow->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);
    const QPoint p(xPos, yPos);
    move(p + m_mainWindow->pos());

    this->setFixedSize(viewMaxSize);
}

void ClipboardHistoryDialog::clearLineEdit()
{
    const QSignalBlocker block(m_lineEdit);
    m_lineEdit.clear();
}

// --------------------------------------------------
// end of copy from Kate quickdialog.cpp
// --------------------------------------------------
