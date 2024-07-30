/*
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2013 Gerald Senarclens de Grancy <oss@senarclens.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variablelineedit.h"

#include "kateautoindent.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katesyntaxmanager.h"
#include "kateview.h"
#include "variableitem.h"
#include "variablelistview.h"

#include <QApplication>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSizeGrip>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <sonnet/speller.h>

VariableLineEdit::VariableLineEdit(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(0, 0, 0, 0);

    m_lineedit = new QLineEdit(this);
    m_button = new QToolButton(this);
    m_button->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    m_button->setToolTip(i18n("Show list of valid variables."));

    hl->addWidget(m_lineedit);
    hl->addWidget(m_button);

    m_popup.reset(new QFrame(nullptr, Qt::Popup));
    m_popup->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    QVBoxLayout *l = new QVBoxLayout(m_popup.get());
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);

    // forward text changed signal
    connect(m_lineedit, &QLineEdit::textChanged, this, &VariableLineEdit::textChanged);

    // open popup on button click
    connect(m_button, &QToolButton::clicked, this, &VariableLineEdit::editVariables);
}

void VariableLineEdit::editVariables()
{
    m_listview = new VariableListView(m_lineedit->text(), m_popup.get());
    addKateItems(m_listview);
    connect(m_listview, &VariableListView::aboutToHide, this, &VariableLineEdit::updateVariableLine);

    m_popup->layout()->addWidget(m_listview);

    if (layoutDirection() == Qt::LeftToRight) {
        QPoint topLeft = mapToGlobal(m_lineedit->geometry().bottomLeft());
        const int w = m_button->geometry().right() - m_lineedit->geometry().left();
        const int h = 300; //(w * 2) / 4;
        m_popup->setGeometry(QRect(topLeft, QSize(w, h)));
    } else {
        QPoint topLeft = mapToGlobal(m_button->geometry().bottomLeft());
        const int w = m_lineedit->geometry().right() - m_button->geometry().left();
        const int h = 300; //(w * 2) / 4;
        m_popup->setGeometry(QRect(topLeft, QSize(w, h)));
    }
    m_popup->show();
}

void VariableLineEdit::updateVariableLine()
{
    Q_ASSERT(m_listview);

    QString variables = m_listview->variableLine();
    m_lineedit->setText(variables);

    m_popup->layout()->removeWidget(m_listview);
    m_listview->deleteLater();
    m_listview = nullptr;
}

void VariableLineEdit::addKateItems(VariableListView *listview)
{
    VariableItem *item = nullptr;

    // If a current active doc is available
    KTextEditor::ViewPrivate *activeView = nullptr;
    KTextEditor::DocumentPrivate *activeDoc = nullptr;

    KateDocumentConfig *docConfig = KateDocumentConfig::global();
    KateViewConfig *viewConfig = KateViewConfig::global();
    KateRendererConfig *rendererConfig = KateRendererConfig::global();

    if ((activeView = qobject_cast<KTextEditor::ViewPrivate *>(KTextEditor::EditorPrivate::self()->application()->activeMainWindow()->activeView()))) {
        activeDoc = activeView->doc();
        viewConfig = activeView->config();
        docConfig = activeDoc->config();
        rendererConfig = activeView->rendererConfig();
    }

    // Add 'auto-brackets' to list
    item = new VariableBoolItem(QStringLiteral("auto-brackets"), false);
    item->setHelpText(i18nc("short translation please", "Enable automatic insertion of brackets."));
    listview->addItem(item);

    // Add 'auto-center-lines' to list
    item = new VariableIntItem(QStringLiteral("auto-center-lines"), viewConfig->autoCenterLines());
    static_cast<VariableIntItem *>(item)->setRange(1, 100);
    item->setHelpText(i18nc("short translation please", "Set the number of autocenter lines."));
    listview->addItem(item);

    // Add 'background-color' to list
    item = new VariableColorItem(QStringLiteral("background-color"), rendererConfig->backgroundColor());
    item->setHelpText(i18nc("short translation please", "Set the document background color."));
    listview->addItem(item);

    // Add 'backspace-indents' to list
    item = new VariableBoolItem(QStringLiteral("backspace-indents"), docConfig->backspaceIndents());
    item->setHelpText(i18nc("short translation please", "Pressing backspace in leading whitespace unindents."));
    listview->addItem(item);

    // Add 'block-selection' to list
    item = new VariableBoolItem(QStringLiteral("block-selection"), false);
    if (activeView) {
        static_cast<VariableBoolItem *>(item)->setValue(activeView->blockSelection());
    }
    item->setHelpText(i18nc("short translation please", "Enable block selection mode."));
    listview->addItem(item);

    // Add 'byte-order-mark' (bom) to list
    item = new VariableBoolItem(QStringLiteral("byte-order-mark"), docConfig->bom());
    item->setHelpText(i18nc("short translation please", "Enable the byte order mark (BOM) when saving Unicode files."));
    listview->addItem(item);

    // Add 'bracket-highlight-color' to list
    item = new VariableColorItem(QStringLiteral("bracket-highlight-color"), rendererConfig->highlightedBracketColor());
    item->setHelpText(i18nc("short translation please", "Set the color for the bracket highlight."));
    listview->addItem(item);

    // Add 'current-line-color' to list
    item = new VariableColorItem(QStringLiteral("current-line-color"), rendererConfig->highlightedLineColor());
    item->setHelpText(i18nc("short translation please", "Set the background color for the current line."));
    listview->addItem(item);

    // Add 'default-dictionary' to list
    Sonnet::Speller speller;
    item = new VariableSpellCheckItem(QStringLiteral("default-dictionary"), speller.defaultLanguage());
    item->setHelpText(i18nc("short translation please", "Set the default dictionary used for spell checking."));
    listview->addItem(item);

    // Add 'dynamic-word-wrap' to list
    item = new VariableBoolItem(QStringLiteral("dynamic-word-wrap"), viewConfig->dynWordWrap());
    item->setHelpText(i18nc("short translation please", "Enable dynamic word wrap of long lines."));
    listview->addItem(item);

    // Add 'end-of-line' (eol) to list
    item = new VariableStringListItem(QStringLiteral("end-of-line"),
                                      {QStringLiteral("unix"), QStringLiteral("mac"), QStringLiteral("dos")},
                                      docConfig->eolString());
    item->setHelpText(i18nc("short translation please", "Sets the end of line mode."));
    listview->addItem(item);

    // Add 'enter-to-insert-completion' to list
    item = new VariableBoolItem(QStringLiteral("enter-to-insert-completion"), viewConfig->value(KateViewConfig::EnterToInsertCompletion).toBool());
    item->setHelpText(i18nc("short translation please", "Enable whether pressing enter will insert selected auto completion item."));
    listview->addItem(item);

    // Add 'folding-markers' to list
    item = new VariableBoolItem(QStringLiteral("folding-markers"), viewConfig->foldingBar());
    item->setHelpText(i18nc("short translation please", "Enable folding markers in the editor border."));
    listview->addItem(item);

    // Add 'folding-preview' to list
    item = new VariableBoolItem(QStringLiteral("folding-preview"), viewConfig->foldingPreview());
    item->setHelpText(i18nc("short translation please", "Enable folding preview in the editor border."));
    listview->addItem(item);

    // Add 'font-size' to list
    item = new VariableIntItem(QStringLiteral("font-size"), rendererConfig->baseFont().pointSize());
    static_cast<VariableIntItem *>(item)->setRange(4, 128);
    item->setHelpText(i18nc("short translation please", "Set the point size of the document font."));
    listview->addItem(item);

    // Add 'font' to list
    item = new VariableFontItem(QStringLiteral("font"), rendererConfig->baseFont());
    item->setHelpText(i18nc("short translation please", "Set the font of the document."));
    listview->addItem(item);

    // Add 'syntax' (hl) to list
    /* Prepare list of highlighting modes */
    QStringList hls;
    hls.reserve(KateHlManager::self()->modeList().size());
    const auto modeList = KateHlManager::self()->modeList();
    for (const auto &hl : modeList) {
        hls << hl.name();
    }

    item = new VariableStringListItem(QStringLiteral("syntax"), hls, hls.at(0));
    if (activeDoc) {
        static_cast<VariableStringListItem *>(item)->setValue(activeDoc->highlightingMode());
    }
    item->setHelpText(i18nc("short translation please", "Set the syntax highlighting."));
    listview->addItem(item);

    // Add 'icon-bar-color' to list
    item = new VariableColorItem(QStringLiteral("icon-bar-color"), rendererConfig->iconBarColor());
    item->setHelpText(i18nc("short translation please", "Set the icon bar color."));
    listview->addItem(item);

    // Add 'icon-border' to list
    item = new VariableBoolItem(QStringLiteral("icon-border"), viewConfig->iconBar());
    item->setHelpText(i18nc("short translation please", "Enable the icon border in the editor view."));
    listview->addItem(item);

    // Add 'indent-mode' to list
    item = new VariableStringListItem(QStringLiteral("indent-mode"), KateAutoIndent::listIdentifiers(), docConfig->indentationMode());
    item->setHelpText(i18nc("short translation please", "Set the auto indentation style."));
    listview->addItem(item);

    // Add 'indent-pasted-text' to list
    item = new VariableBoolItem(QStringLiteral("indent-pasted-text"), docConfig->indentPastedText());
    item->setHelpText(i18nc("short translation please", "Adjust indentation of text pasted from the clipboard."));
    listview->addItem(item);

    // Add 'indent-width' to list
    item = new VariableIntItem(QStringLiteral("indent-width"), docConfig->indentationWidth());
    static_cast<VariableIntItem *>(item)->setRange(1, 200);
    item->setHelpText(i18nc("short translation please", "Set the indentation depth for each indent level."));
    listview->addItem(item);

    // Add 'keep-extra-spaces' to list
    item = new VariableBoolItem(QStringLiteral("keep-extra-spaces"), docConfig->keepExtraSpaces());
    item->setHelpText(i18nc("short translation please", "Allow odd indentation level (no multiple of indent width)."));
    listview->addItem(item);

    // Add 'line-numbers' to list
    item = new VariableBoolItem(QStringLiteral("line-numbers"), viewConfig->lineNumbers());
    item->setHelpText(i18nc("short translation please", "Show line numbers."));
    listview->addItem(item);

    // Add 'newline-at-eof' to list
    item = new VariableBoolItem(QStringLiteral("newline-at-eof"), docConfig->ovr());
    item->setHelpText(i18nc("short translation please", "Insert newline at end of file on save."));
    listview->addItem(item);

    // Add 'overwrite-mode' to list
    item = new VariableBoolItem(QStringLiteral("overwrite-mode"), docConfig->ovr());
    item->setHelpText(i18nc("short translation please", "Enable overwrite mode in the document."));
    listview->addItem(item);

    // Add 'persistent-selection' to list
    item = new VariableBoolItem(QStringLiteral("persistent-selection"), viewConfig->persistentSelection());
    item->setHelpText(i18nc("short translation please", "Enable persistent text selection."));
    listview->addItem(item);

    // Add 'replace-tabs-save' to list
    item = new VariableBoolItem(QStringLiteral("replace-tabs-save"), false);
    item->setHelpText(i18nc("short translation please", "Replace tabs with spaces when saving the document."));
    listview->addItem(item);

    // Add 'replace-tabs' to list
    item = new VariableBoolItem(QStringLiteral("replace-tabs"), docConfig->replaceTabsDyn());
    item->setHelpText(i18nc("short translation please", "Replace tabs with spaces."));
    listview->addItem(item);

    // Add 'remove-trailing-spaces' to list
    item = new VariableRemoveSpacesItem(QStringLiteral("remove-trailing-spaces"), docConfig->removeSpaces());
    item->setHelpText(i18nc("short translation please", "Remove trailing spaces when saving the document."));
    listview->addItem(item);

    // Add 'scrollbar-minimap' to list
    item = new VariableBoolItem(QStringLiteral("scrollbar-minimap"), viewConfig->scrollBarMiniMap());
    item->setHelpText(i18nc("short translation please", "Show scrollbar minimap."));
    listview->addItem(item);

    // Add 'scrollbar-preview' to list
    item = new VariableBoolItem(QStringLiteral("scrollbar-preview"), viewConfig->scrollBarPreview());
    item->setHelpText(i18nc("short translation please", "Show scrollbar preview."));
    listview->addItem(item);

    // Add 'scheme' to list
    QStringList themeNames;
    const auto sortedThemes = KateHlManager::self()->sortedThemes();
    themeNames.reserve(sortedThemes.size());
    for (const auto &theme : sortedThemes) {
        themeNames.append(theme.name());
    }
    item = new VariableStringListItem(QStringLiteral("scheme"), themeNames, rendererConfig->schema());
    item->setHelpText(i18nc("short translation please", "Set the color scheme."));
    listview->addItem(item);

    // Add 'selection-color' to list
    item = new VariableColorItem(QStringLiteral("selection-color"), rendererConfig->selectionColor());
    item->setHelpText(i18nc("short translation please", "Set the text selection color."));
    listview->addItem(item);

    // Add 'show-tabs' to list
    item = new VariableBoolItem(QStringLiteral("show-tabs"), docConfig->showTabs());
    item->setHelpText(i18nc("short translation please", "Visualize tabs and trailing spaces."));
    listview->addItem(item);

    // Add 'smart-home' to list
    item = new VariableBoolItem(QStringLiteral("smart-home"), docConfig->smartHome());
    item->setHelpText(i18nc("short translation please", "Enable smart home navigation."));
    listview->addItem(item);

    // Add 'tab-indents' to list
    item = new VariableBoolItem(QStringLiteral("tab-indents"), docConfig->tabIndentsEnabled());
    item->setHelpText(i18nc("short translation please", "Pressing TAB key indents."));
    listview->addItem(item);

    // Add 'tab-width' to list
    item = new VariableIntItem(QStringLiteral("tab-width"), docConfig->tabWidth());
    static_cast<VariableIntItem *>(item)->setRange(1, 200);
    item->setHelpText(i18nc("short translation please", "Set the tab display width."));
    listview->addItem(item);

    // Add 'undo-steps' to list
    item = new VariableIntItem(QStringLiteral("undo-steps"), 0);
    static_cast<VariableIntItem *>(item)->setRange(0, 100);
    item->setHelpText(i18nc("short translation please", "Set the number of undo steps to remember (0 equals infinity)."));
    listview->addItem(item);

    // Add 'word-wrap-column' to list
    item = new VariableIntItem(QStringLiteral("word-wrap-column"), docConfig->wordWrapAt());
    static_cast<VariableIntItem *>(item)->setRange(20, 200);
    item->setHelpText(i18nc("short translation please", "Set the word wrap column."));
    listview->addItem(item);

    // Add 'word-wrap-marker-color' to list
    item = new VariableColorItem(QStringLiteral("word-wrap-marker-color"), rendererConfig->wordWrapMarkerColor());
    item->setHelpText(i18nc("short translation please", "Set the word wrap marker color."));
    listview->addItem(item);

    // Add 'word-wrap' to list
    item = new VariableBoolItem(QStringLiteral("word-wrap"), docConfig->wordWrap());
    item->setHelpText(i18nc("short translation please", "Enable word wrap while typing text."));
    listview->addItem(item);
}

void VariableLineEdit::setText(const QString &text)
{
    m_lineedit->setText(text);
}

void VariableLineEdit::clear()
{
    m_lineedit->clear();
}

QString VariableLineEdit::text()
{
    return m_lineedit->text();
}

#include "moc_variablelineedit.cpp"
