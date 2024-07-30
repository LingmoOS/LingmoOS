/*
    SPDX-FileCopyrightText: 2024 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateargumenthinttree.h"

#include "kateargumenthintmodel.h"
#include "katecompletionwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QToolButton>

class ArgumentHighlighter : public QSyntaxHighlighter
{
public:
    ArgumentHighlighter(QTextDocument *doc)
        : QSyntaxHighlighter(doc)
    {
    }

    void highlightBlock(const QString &) override
    {
        for (const auto &f : std::as_const(formats)) {
            QTextCharFormat fmt = f.format;
            if (fmt.fontWeight() == QFont::Bold || fmt.fontItalic()) {
                // bold doesn't work with some fonts for whatever reason
                // so we just underline as well for fonts where bold won't work
                fmt.setFontUnderline(true);
            }
            setFormat(f.start, f.length, fmt);
        }
    }

    QVector<QTextLayout::FormatRange> formats;
};

static QList<QTextLayout::FormatRange> highlightingFromVariantList(const QList<QVariant> &customHighlights)
{
    QList<QTextLayout::FormatRange> ret;

    for (int i = 0; i + 2 < customHighlights.count(); i += 3) {
        if (!customHighlights[i].canConvert<int>() || !customHighlights[i + 1].canConvert<int>() || !customHighlights[i + 2].canConvert<QTextFormat>()) {
            continue;
        }

        QTextLayout::FormatRange format;
        format.start = customHighlights[i].toInt();
        format.length = customHighlights[i + 1].toInt();
        format.format = customHighlights[i + 2].value<QTextFormat>().toCharFormat();

        if (!format.format.isValid()) {
            qWarning() << "Format is not valid";
            continue;
        }

        ret << format;
    }
    return ret;
}

ArgumentHintWidget::ArgumentHintWidget(KateArgumentHintModel *model, const QFont &font, KateCompletionWidget *completion, QWidget *parent)
    : QFrame(parent)
    , m_completionWidget(completion)
    , m_view(new QPlainTextEdit(this))
    , m_currentIndicator(new QLabel(this))
    , m_model(model)
    , m_highlighter(new ArgumentHighlighter(m_view->document()))
    , m_leftSide(new QWidget(this))
{
    setAutoFillBackground(true);
    // we have only 1 top level frame
    setFrameStyle(QFrame::Box | QFrame::Raised);
    m_view->setFrameStyle(QFrame::NoFrame);

    auto upButton = new QToolButton(this);
    upButton->setAutoRaise(true);
    upButton->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    connect(upButton, &QAbstractButton::clicked, this, &ArgumentHintWidget::selectPrevious);

    auto downButton = new QToolButton(this);
    downButton->setAutoRaise(true);
    downButton->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    connect(downButton, &QAbstractButton::clicked, this, &ArgumentHintWidget::selectNext);

    auto vLayout = new QVBoxLayout(m_leftSide);
    vLayout->setContentsMargins({});
    vLayout->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(upButton);
    vLayout->addWidget(m_currentIndicator);
    vLayout->addWidget(downButton);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(m_leftSide);
    layout->addWidget(m_view);
    setFixedWidth(380);
    m_view->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_view->document()->setDefaultFont(font);

    connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
        m_current = -1;
        selectNext();
    });
    setVisible(false);
}

void ArgumentHintWidget::selectNext()
{
    int rowCount = m_model->rowCount();
    if (rowCount == 0) {
        clearAndHide();
        return;
    }
    m_current = m_current + 1;
    if (m_current >= rowCount) {
        m_current = 0;
    }

    activateHint(m_current, rowCount);
}

void ArgumentHintWidget::selectPrevious()
{
    int rowCount = m_model->rowCount();
    if (rowCount == 0) {
        clearAndHide();
        return;
    }

    m_current = m_current - 1;
    if (m_current <= 0) {
        m_current = rowCount - 1;
    }

    activateHint(m_current, rowCount);
}

void ArgumentHintWidget::activateHint(int i, int rowCount)
{
    const auto index = m_model->index(i);
    const auto list = index.data(KTextEditor::CodeCompletionModel::CustomHighlight).toList();
    const auto highlights = highlightingFromVariantList(list);
    m_highlighter->formats = highlights;

    if (rowCount == 1) {
        m_leftSide->setVisible(false);
    } else {
        if (m_leftSide->isHidden()) {
            m_leftSide->setVisible(true);
        }
        m_currentIndicator->setText(QStringLiteral("%1/%2").arg(i + 1).arg(rowCount));
    }

    m_view->setPlainText(index.data().toString());
    updateGeometry();
}

void ArgumentHintWidget::updateGeometry()
{
    auto block = m_view->document()->begin();
    int lines = 0;
    QFontMetrics fm(m_view->document()->defaultFont());
    int maxWidth = 0;
    while (block.isValid()) {
        maxWidth = std::max((int)block.layout()->maximumWidth(), maxWidth);
        lines += block.layout()->lineCount();
        block = block.next();
    }
    setFixedHeight((std::max(lines,1) * fm.height()) + 10 + m_view->document()->documentMargin());
    // limit the width to between 400 - 600
    int width = std::max(maxWidth, 400);
    width = std::min(width, 600);
    setFixedWidth(width);

    QPoint pos = m_completionWidget->pos();
    pos.ry() -= this->height();
    pos.ry() -= fm.height();
    pos.ry() -= 4;
    move(pos);
}

void ArgumentHintWidget::positionAndShow()
{
    updateGeometry();
    show();
}

void ArgumentHintWidget::clearAndHide()
{
    m_current = -1;
    m_currentIndicator->clear();
    m_view->clear();
    hide();
}
