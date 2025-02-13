/*
    SPDX-FileCopyrightText: 2007, 2009 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katequickopen.h"
#include "katequickopenmodel.h"

#include "kateapp.h"
#include "katemainwindow.h"
#include "kateviewmanager.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QEvent>
#include <QPainter>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTreeView>

#include <drawing_utils.h>
#include <kfts_fuzzy_match.h>

class QuickOpenFilterProxyModel final : public QSortFilterProxyModel
{
public:
    QuickOpenFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        auto sm = sourceModel();
        if (pattern.isEmpty()) {
            const bool l = static_cast<KateQuickOpenModel *>(sm)->isOpened(sourceLeft);
            const bool r = static_cast<KateQuickOpenModel *>(sm)->isOpened(sourceRight);
            return l < r;
        }
        const int l = static_cast<KateQuickOpenModel *>(sm)->idxScore(sourceLeft);
        const int r = static_cast<KateQuickOpenModel *>(sm)->idxScore(sourceRight);
        return l < r;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &parent) const override
    {
        if (pattern.isEmpty()) {
            return true;
        }

        if (filterMode == Wildcard) {
            return QSortFilterProxyModel::filterAcceptsRow(sourceRow, parent);
        }

        auto sm = static_cast<KateQuickOpenModel *>(sourceModel());
        if (!sm->isValid(sourceRow)) {
            return false;
        }

        QStringView fileNameMatchPattern = pattern;
        // When matching path, we want to match the last section of the pattern
        // with filenames. /path/to/file => pattern: file
        if (matchPath) {
            int lastSlash = pattern.lastIndexOf(QLatin1Char('/'));
            if (lastSlash != -1) {
                fileNameMatchPattern = fileNameMatchPattern.mid(lastSlash + 1);
            }
        }

        const QString &name = sm->idxToFileName(sourceRow);

        int score = 0;
        bool res;
        // dont use the QStringView(QString) ctor
        if (fileNameMatchPattern.isEmpty()) {
            res = true;
        } else {
            res = filterByName(QStringView(name.data(), name.size()), fileNameMatchPattern, score);
        }

        // only match file path if needed
        if (matchPath && res) {
            int scorep = 0;
            QStringView path{sm->idxToFilePath(sourceRow)};
            bool resp = filterByPath(path, QStringView(pattern.data(), pattern.size()), scorep);
            score += scorep;
            res = resp;
            // zero out the score if didn't match
            score *= res;
        }

        if (res) {
            // extra points for opened files
            score += (sm->isOpened(sourceRow)) * (score / 6);

            // extra points if file exists in project root
            // This gives priority to the files at the root
            // of the project over others. This is important
            // because otherwise getting to root files may
            // not be that easy
            if (!matchPath) {
                score += (sm->idxToFilePath(sourceRow) == name) * name.size();
            }
        }
        //         if (res && pattern == QStringLiteral(""))
        //             qDebug() << score << ", " << name << "==================== END\n";

        sm->setScoreForIndex(sourceRow, score);

        return res;
    }

public Q_SLOTS:
    bool setFilterText(const QString &text)
    {
        // we don't want to trigger filtering if the user is just entering line:col
        const auto splitted = text.split(QLatin1Char(':')).at(0);
        if (splitted == pattern) {
            return false;
        }

        if (filterMode == Wildcard) {
            pattern = splitted;
            setFilterWildcard(pattern);
            return true;
        }

        beginResetModel();
        pattern = splitted;
        matchPath = pattern.contains(QLatin1Char('/'));
        endResetModel();

        return true;
    }

    void setFilterMode(FilterMode m)
    {
        beginResetModel();
        filterMode = m;
        if (!pattern.isEmpty() && m == Wildcard) {
            setFilterWildcard(pattern);
        }
        endResetModel();
    }

private:
    static inline bool filterByPath(QStringView path, QStringView pattern, int &score)
    {
        return kfts::fuzzy_match(pattern, path, score);
    }

    static inline bool filterByName(QStringView name, QStringView pattern, int &score)
    {
        return kfts::fuzzy_match(pattern, name, score);
    }

private:
    QString pattern;
    bool matchPath = false;
    FilterMode filterMode = Fuzzy;
};

class QuickOpenStyleDelegate : public QStyledItemDelegate
{
public:
    QuickOpenStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (m_filterMode != Fuzzy) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        QString name = index.data(KateQuickOpenModel::FileName).toString();
        QString path = index.data(KateQuickOpenModel::FilePath).toString();

        // only remove suffix, not where it might occur elsewhere
        const QString suffix = QStringLiteral("/") + name;
        if (path.endsWith(suffix)) {
            path.chop(suffix.size());
        }

        QTextCharFormat fmt;
        fmt.setForeground(options.palette.link().color());
        fmt.setFontWeight(QFont::Bold);

        const int nameLen = name.length();
        // space between name and path
        constexpr int space = 1;
        QVector<QTextLayout::FormatRange> formats;

        // collect formats
        int pos = m_filterString.lastIndexOf(QLatin1Char('/'));
        if (pos > -1) {
            ++pos;
            auto pattern = QStringView(m_filterString).mid(pos);
            auto nameFormats = kfts::get_fuzzy_match_formats(pattern, name, 0, fmt);
            formats.append(nameFormats);
        } else {
            auto nameFormats = kfts::get_fuzzy_match_formats(m_filterString, name, 0, fmt);
            formats.append(nameFormats);
        }
        QTextCharFormat boldFmt;
        boldFmt.setFontWeight(QFont::Bold);
        boldFmt.setFontPointSize(options.font.pointSize() - 1);
        auto pathFormats = kfts::get_fuzzy_match_formats(m_filterString, path, nameLen + space, boldFmt);
        QTextCharFormat gray;
        gray.setForeground(Qt::gray);
        gray.setFontPointSize(options.font.pointSize() - 1);
        formats.append({nameLen + space, static_cast<int>(path.length()), gray});
        formats.append(pathFormats);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // space for icon
        painter->translate(25, 0);

        // draw text
        Utils::paintItemViewText(painter, QString(name + QStringLiteral(" ") + path), options, formats);

        painter->restore();
    }

public Q_SLOTS:
    void setFilterString(const QString &text)
    {
        m_filterString = text;
    }

    void setFilterMode(FilterMode m)
    {
        m_filterMode = m;
    }

private:
    QString m_filterString;
    FilterMode m_filterMode;
};

Q_DECLARE_METATYPE(QPointer<KTextEditor::Document>)

KateQuickOpen::KateQuickOpen(KateMainWindow *mainWindow)
    : QMenu(mainWindow)
    , m_mainWindow(mainWindow)
{
    // ensure the components have some proper frame
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    m_inputLine = new QuickOpenLineEdit(this);
    setFocusProxy(m_inputLine);

    layout->addWidget(m_inputLine);

    m_listView = new QTreeView();
    layout->addWidget(m_listView, 1);
    m_listView->setTextElideMode(Qt::ElideLeft);
    m_listView->setUniformRowHeights(true);

    m_model = new KateQuickOpenModel(this);
    m_styleDelegate = new QuickOpenStyleDelegate(this);
    m_listView->setItemDelegate(m_styleDelegate);

    connect(m_inputLine, &QuickOpenLineEdit::textChanged, this, &KateQuickOpen::reselectFirst, Qt::QueuedConnection);
    connect(m_inputLine, &QuickOpenLineEdit::returnPressed, this, &KateQuickOpen::slotReturnPressed);
    connect(m_inputLine, &QuickOpenLineEdit::listModeChanged, this, &KateQuickOpen::slotListModeChanged);
    connect(m_inputLine, &QuickOpenLineEdit::filterModeChanged, this, &KateQuickOpen::setFilterMode);

    connect(m_listView, &QTreeView::activated, this, &KateQuickOpen::slotReturnPressed);
    connect(m_listView, &QTreeView::clicked, this, &KateQuickOpen::slotReturnPressed); // for single click

    m_inputLine->installEventFilter(this);
    m_listView->installEventFilter(this);
    m_listView->setHeaderHidden(true);
    m_listView->setRootIsDecorated(false);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_proxyModel = new QuickOpenFilterProxyModel(this);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSourceModel(m_model);
    m_listView->setModel(m_proxyModel);
    connect(m_inputLine, &QuickOpenLineEdit::textChanged, m_proxyModel, [this](const QString &text) {
        if (m_proxyModel->setFilterText(text)) {
            m_styleDelegate->setFilterString(text);
            m_listView->viewport()->update();
        }
    });

    setHidden(true);

    setFilterMode();
    m_model->setListMode(m_inputLine->listMode());

    // fill stuff
    updateState();
}

bool KateQuickOpen::eventFilter(QObject *obj, QEvent *event)
{
    // catch key presses + shortcut overrides to allow to have ESC as application wide shortcut, too, see bug 409856
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == m_inputLine) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(m_listView, event);
                return true;
            }

        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(m_inputLine, event);
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void KateQuickOpen::reselectFirst()
{
    int first = 0;
    const auto *model = m_listView->model();
    if (m_mainWindow->viewManager()->views().size() > 1 && model->rowCount() > 1 && m_inputLine->text().isEmpty()) {
        first = 1;
    }

    QModelIndex index = model->index(first, 0);
    m_listView->setCurrentIndex(index);
}

void KateQuickOpen::updateState()
{
    m_model->refresh(m_mainWindow);
    reselectFirst();

    updateViewGeometry();
    show();
    setFocus();
}

void KateQuickOpen::slotReturnPressed()
{
    // save current position before opening new url for location history
    auto vm = m_mainWindow->viewManager();
    if (auto v = vm->activeView()) {
        vm->addPositionToHistory(v->document()->url(), v->cursorPosition());
    }

    // either get view via document pointer or url
    const QModelIndex index = m_listView->model()->index(m_listView->currentIndex().row(), 0);
    KTextEditor::View *view = nullptr;
    if (auto doc = index.data(KateQuickOpenModel::Document).value<KTextEditor::Document *>()) {
        view = m_mainWindow->activateView(doc);
    } else {
        view = m_mainWindow->wrapper()->openUrl(index.data(Qt::UserRole).toUrl());
    }

    const auto strs = m_inputLine->text().split(QLatin1Char(':'));
    if (view && strs.count() > 1) {
        // helper to convert String => Number
        auto stringToInt = [](const QString &s) {
            bool ok = false;
            const int num = s.toInt(&ok);
            return ok ? num : -1;
        };
        KTextEditor::Cursor cursor = KTextEditor::Cursor::invalid();

        // try to get line
        const int line = stringToInt(strs.at(1));
        cursor.setLine(line - 1);

        // if line is valid, try to see if we have column available as well
        if (line > -1 && strs.count() > 2) {
            const int col = stringToInt(strs.at(2));
            cursor.setColumn(col - 1);
        }

        // do we have valid line at least?
        if (line > -1) {
            view->setCursorPosition(cursor);
        }
    }

    hide();
    m_mainWindow->slotWindowActivated();

    // store the new position in location history
    if (view) {
        vm->addPositionToHistory(view->document()->url(), view->cursorPosition());
    }
}

void KateQuickOpen::slotListModeChanged(KateQuickOpenModel::List mode)
{
    m_model->setListMode(mode);
    // this changes things again, needs refresh, let's go all the way
    updateState();
}

void KateQuickOpen::setFilterMode()
{
    auto newMode = m_inputLine->filterMode();
    m_proxyModel->setFilterMode(newMode);
    m_listView->setSortingEnabled(newMode == Fuzzy);
    m_styleDelegate->setFilterMode(newMode);
    m_listView->viewport()->update();
}

void KateQuickOpen::updateViewGeometry()
{
    const QSize centralSize = m_mainWindow->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);
    const QPoint p(xPos, yPos);
    move(p + m_mainWindow->pos());

    setFixedSize(viewMaxSize);
}
