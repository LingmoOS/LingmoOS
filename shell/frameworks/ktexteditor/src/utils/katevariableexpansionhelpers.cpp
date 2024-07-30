/*
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katevariableexpansionhelpers.h"

#include "variable.h"

#include <KLocalizedString>
#include <KTextEditor/Application>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>

#include <QAbstractItemModel>
#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QHelpEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTextEdit>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>

/**
 * Find closing bracket for @p str starting a position @p pos.
 */
static int findClosing(QStringView str, int pos = 0)
{
    const int len = str.size();
    int nesting = 0;

    while (pos < len) {
        const QChar c = str[pos];
        if (c == QLatin1Char('}')) {
            if (nesting == 0) {
                return pos;
            }
            nesting--;
        } else if (c == QLatin1Char('{')) {
            nesting++;
        }
        ++pos;
    }
    return -1;
}

namespace KateMacroExpander
{
QString expandMacro(const QString &input, KTextEditor::View *view)
{
    QString output = input;
    QString oldStr;
    do {
        oldStr = output;
        const int startIndex = output.indexOf(QLatin1String("%{"));
        if (startIndex < 0) {
            break;
        }

        const int endIndex = findClosing(output, startIndex + 2);
        if (endIndex <= startIndex) {
            break;
        }

        const int varLen = endIndex - (startIndex + 2);
        QString variable = output.mid(startIndex + 2, varLen);
        variable = expandMacro(variable, view);
        if (KTextEditor::Editor::instance()->expandVariable(variable, view, variable)) {
            output.replace(startIndex, endIndex - startIndex + 1, variable);
        }
    } while (output != oldStr); // str comparison guards against infinite loop
    return output;
}

}

class VariableItemModel : public QAbstractItemModel
{
public:
    VariableItemModel(QObject *parent = nullptr)
        : QAbstractItemModel(parent)
    {
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid() || row < 0 || row >= m_variables.size()) {
            return {};
        }

        return createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex &index) const override
    {
        Q_UNUSED(index)
        // flat list -> we never have parents
        return {};
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : m_variables.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 3; // name | description | current value
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) {
            return {};
        }

        const auto &var = m_variables[index.row()];
        switch (role) {
        case Qt::DisplayRole: {
            const QString suffix = var.isPrefixMatch() ? i18n("<value>") : QString();
            return QString(var.name() + suffix);
        }
        case Qt::ToolTipRole:
            return var.description();
        }

        return {};
    }

    void setVariables(const QList<KTextEditor::Variable> &variables)
    {
        beginResetModel();
        m_variables = variables;
        endResetModel();
    }

private:
    QList<KTextEditor::Variable> m_variables;
};

class TextEditButton : public QToolButton
{
public:
    TextEditButton(QAction *showAction, QTextEdit *parent)
        : QToolButton(parent)
    {
        setAutoRaise(true);
        setDefaultAction(showAction);
        m_watched = parent->viewport();
        m_watched->installEventFilter(this);
        show();
        adjustPosition(m_watched->size());
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        // reimplement to have same behavior as actions in QLineEdits
        QStylePainter p(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.state = opt.state & ~QStyle::State_Raised;
        opt.state = opt.state & ~QStyle::State_MouseOver;
        opt.state = opt.state & ~QStyle::State_Sunken;
        p.drawComplexControl(QStyle::CC_ToolButton, opt);
    }

public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_watched) {
            switch (event->type()) {
            case QEvent::Resize: {
                auto resizeEvent = static_cast<QResizeEvent *>(event);
                adjustPosition(resizeEvent->size());
            }
            default:
                break;
            }
        }
        return QToolButton::eventFilter(watched, event);
    }

private:
    void adjustPosition(const QSize &parentSize)
    {
        QStyleOption sopt;
        sopt.initFrom(parentWidget());
        const int topMargin = 0; // style()->pixelMetric(QStyle::PM_LayoutTopMargin, &sopt, parentWidget());
        const int rightMargin = 0; // style()->pixelMetric(QStyle::PM_LayoutRightMargin, &sopt, parentWidget());
        if (isLeftToRight()) {
            move(parentSize.width() - width() - rightMargin, topMargin);
        } else {
            move(0, 0);
        }
    }

private:
    QWidget *m_watched;
};

KateVariableExpansionDialog::KateVariableExpansionDialog(QWidget *parent)
    : QDialog(parent, Qt::Tool)
    , m_showAction(new QAction(QIcon::fromTheme(QStringLiteral("code-context")), i18n("Insert variable"), this))
    , m_variableModel(new VariableItemModel(this))
    , m_listView(new QListView(this))
{
    setWindowTitle(i18n("Variables"));

    auto vbox = new QVBoxLayout(this);
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(i18n("Filter"));
    m_filterEdit->setFocus();
    m_filterEdit->installEventFilter(this);
    vbox->addWidget(m_filterEdit);
    vbox->addWidget(m_listView);
    m_listView->setUniformItemSizes(true);

    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setFilterRole(Qt::DisplayRole);
    m_filterModel->setSortRole(Qt::DisplayRole);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterKeyColumn(0);

    m_filterModel->setSourceModel(m_variableModel);
    m_listView->setModel(m_filterModel);

    connect(m_filterEdit, &QLineEdit::textChanged, m_filterModel, &QSortFilterProxyModel::setFilterWildcard);

    auto lblDescription = new QLabel(i18n("Please select a variable."), this);
    auto lblCurrentValue = new QLabel(this);

    vbox->addWidget(lblDescription);
    vbox->addWidget(lblCurrentValue);

    // react to selection changes
    connect(m_listView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            [this, lblDescription, lblCurrentValue](const QModelIndex &current, const QModelIndex &) {
                if (current.isValid()) {
                    const auto &var = m_variables[m_filterModel->mapToSource(current).row()];
                    lblDescription->setText(var.description());
                    if (var.isPrefixMatch()) {
                        lblCurrentValue->setText(i18n("Current value: %1<value>", var.name()));
                    } else {
                        auto activeView = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
                        const auto value = var.evaluate(var.name(), activeView);
                        lblCurrentValue->setText(i18n("Current value: %1", value));
                    }
                } else {
                    lblDescription->setText(i18n("Please select a variable."));
                    lblCurrentValue->clear();
                }
            });

    // insert text on activation
    connect(m_listView, &QAbstractItemView::activated, [this](const QModelIndex &index) {
        if (index.isValid()) {
            const auto &var = m_variables[m_filterModel->mapToSource(index).row()];

            // not auto, don't fall for string builder, see bug 413474
            const QString name = QStringLiteral("%{") + var.name() + QLatin1Char('}');
            if (parentWidget() && parentWidget()->window()) {
                auto currentWidget = parentWidget()->window()->focusWidget();
                if (auto lineEdit = qobject_cast<QLineEdit *>(currentWidget)) {
                    lineEdit->insert(name);
                } else if (auto textEdit = qobject_cast<QTextEdit *>(currentWidget)) {
                    textEdit->insertPlainText(name);
                }
            }
        }
    });

    // show dialog whenever the action is clicked
    connect(m_showAction, &QAction::triggered, [this]() {
        show();
        activateWindow();
    });

    resize(400, 550);
}

KateVariableExpansionDialog::~KateVariableExpansionDialog()
{
    for (auto it = m_textEditButtons.begin(); it != m_textEditButtons.end(); ++it) {
        if (it.value()) {
            delete it.value();
        }
    }
    m_textEditButtons.clear();
}

void KateVariableExpansionDialog::addVariable(const KTextEditor::Variable &variable)
{
    Q_ASSERT(variable.isValid());
    m_variables.push_back(variable);

    m_variableModel->setVariables(m_variables);
}

int KateVariableExpansionDialog::isEmpty() const
{
    return m_variables.isEmpty();
}

void KateVariableExpansionDialog::addWidget(QWidget *widget)
{
    m_widgets.push_back(widget);
    widget->installEventFilter(this);

    connect(widget, &QObject::destroyed, this, &KateVariableExpansionDialog::onObjectDeleted);
}

void KateVariableExpansionDialog::onObjectDeleted(QObject *object)
{
    m_widgets.removeAll(object);
    if (m_widgets.isEmpty()) {
        deleteLater();
    }
}

bool KateVariableExpansionDialog::eventFilter(QObject *watched, QEvent *event)
{
    // filter line edit
    if (watched == m_filterEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown) || (keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return);
            if (forward2list) {
                QCoreApplication::sendEvent(m_listView, event);
                return true;
            }
        }
        return QDialog::eventFilter(watched, event);
    }

    // tracked widgets (tooltips, adding/removing the showAction)
    switch (event->type()) {
    case QEvent::FocusIn: {
        if (auto lineEdit = qobject_cast<QLineEdit *>(watched)) {
            lineEdit->addAction(m_showAction, QLineEdit::TrailingPosition);
        } else if (auto textEdit = qobject_cast<QTextEdit *>(watched)) {
            if (!m_textEditButtons.contains(textEdit)) {
                m_textEditButtons[textEdit] = new TextEditButton(m_showAction, textEdit);
            }
            m_textEditButtons[textEdit]->raise();
            m_textEditButtons[textEdit]->show();
        }
        break;
    }
    case QEvent::FocusOut: {
        if (auto lineEdit = qobject_cast<QLineEdit *>(watched)) {
            lineEdit->removeAction(m_showAction);
        } else if (auto textEdit = qobject_cast<QTextEdit *>(watched)) {
            if (m_textEditButtons.contains(textEdit)) {
                delete m_textEditButtons[textEdit];
                m_textEditButtons.remove(textEdit);
            }
        }
        break;
    }
    case QEvent::ToolTip: {
        QString inputText;
        if (auto lineEdit = qobject_cast<QLineEdit *>(watched)) {
            inputText = lineEdit->text();
        }
        QString toolTip;
        if (!inputText.isEmpty()) {
            auto activeView = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
            toolTip = KTextEditor::Editor::instance()->expandText(inputText, activeView);
        }

        if (!toolTip.isEmpty()) {
            auto helpEvent = static_cast<QHelpEvent *>(event);
            QToolTip::showText(helpEvent->globalPos(), toolTip, qobject_cast<QWidget *>(watched));
            event->accept();
            return true;
        }
        break;
    }
    default:
        break;
    }

    // auto-hide on focus change
    auto parentWindow = parentWidget()->window();
    const bool keepVisible = isActiveWindow() || m_widgets.contains(parentWindow->focusWidget());
    if (!keepVisible) {
        hide();
    }

    return QDialog::eventFilter(watched, event);
}

// kate: space-indent on; indent-width 4; replace-tabs on;
