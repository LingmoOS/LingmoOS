/*
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katevariableexpansionmanager.h"
#include "katevariableexpansionhelpers.h"

#include "katedocument.h"
#include "kateglobal.h"

#include <KLocalizedString>

#include <QAbstractItemModel>
#include <QListView>
#include <QVBoxLayout>

#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QJSEngine>
#include <QLocale>
#include <QTime>
#include <QUuid>

static void registerVariables(KateVariableExpansionManager &mng)
{
    using KTextEditor::Variable;

    mng.addVariable(Variable(
        QStringLiteral("Document:FileBaseName"),
        i18n("File base name without path and suffix of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return QFileInfo(url).baseName();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:FileExtension"),
        i18n("File extension of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return QFileInfo(url).completeSuffix();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:FileName"),
        i18n("File name without path of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return QFileInfo(url).fileName();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:FilePath"),
        i18n("Full path of the current document including the file name."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return QFileInfo(url).absoluteFilePath();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Text"),
        i18n("Contents of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? view->document()->text() : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Path"),
        i18n("Full path of the current document excluding the file name."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return QFileInfo(url).absolutePath();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:NativeFilePath"),
        i18n("Full document path including file name, with native path separator (backslash on Windows)."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return url.isEmpty() ? QString() : QDir::toNativeSeparators(QFileInfo(url).absoluteFilePath());
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:NativePath"),
        i18n("Full document path excluding file name, with native path separator (backslash on Windows)."),
        [](const QStringView &, KTextEditor::View *view) {
            const auto url = view ? view->document()->url().toLocalFile() : QString();
            return url.isEmpty() ? QString() : QDir::toNativeSeparators(QFileInfo(url).absolutePath());
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Cursor:Line"),
        i18n("Line number of the text cursor position in current document (starts with 0)."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? QString::number(view->cursorPosition().line()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Cursor:Column"),
        i18n("Column number of the text cursor position in current document (starts with 0)."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? QString::number(view->cursorPosition().column()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Cursor:XPos"),
        i18n("X component in global screen coordinates of the cursor position."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? QString::number(view->mapToGlobal(view->cursorPositionCoordinates()).x()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Cursor:YPos"),
        i18n("Y component in global screen coordinates of the cursor position."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? QString::number(view->mapToGlobal(view->cursorPositionCoordinates()).y()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Selection:Text"),
        i18n("Text selection of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return (view && view->selection()) ? view->selectionText() : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Selection:StartLine"),
        i18n("Start line of selected text of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return (view && view->selection()) ? QString::number(view->selectionRange().start().line()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Selection:StartColumn"),
        i18n("Start column of selected text of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return (view && view->selection()) ? QString::number(view->selectionRange().start().column()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Selection:EndLine"),
        i18n("End line of selected text of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return (view && view->selection()) ? QString::number(view->selectionRange().end().line()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Selection:EndColumn"),
        i18n("End column of selected text of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return (view && view->selection()) ? QString::number(view->selectionRange().end().column()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:RowCount"),
        i18n("Number of rows of the current document."),
        [](const QStringView &, KTextEditor::View *view) {
            return view ? QString::number(view->document()->lines()) : QString();
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Document:Variable:"),
        i18n("Read a document variable."),
        [](const QStringView &str, KTextEditor::View *view) {
            return view ? qobject_cast<KTextEditor::DocumentPrivate *>(view->document())->variable(str.mid(18).toString()) : QString();
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("Date:Locale"),
        i18n("The current date in current locale format."),
        [](const QStringView &, KTextEditor::View *) {
            return QLocale().toString(QDate::currentDate(), QLocale::ShortFormat);
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Date:ISO"),
        i18n("The current date (ISO)."),
        [](const QStringView &, KTextEditor::View *) {
            return QDate::currentDate().toString(Qt::ISODate);
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Date:"),
        i18n("The current date (QDate formatstring)."),
        [](const QStringView &str, KTextEditor::View *) {
            return QDate::currentDate().toString(str.mid(5));
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("Time:Locale"),
        i18n("The current time in current locale format."),
        [](const QStringView &, KTextEditor::View *) {
            return QLocale().toString(QTime::currentTime(), QLocale::ShortFormat);
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Time:ISO"),
        i18n("The current time (ISO)."),
        [](const QStringView &, KTextEditor::View *) {
            return QTime::currentTime().toString(Qt::ISODate);
        },
        false));
    mng.addVariable(Variable(
        QStringLiteral("Time:"),
        i18n("The current time (QTime formatstring)."),
        [](const QStringView &str, KTextEditor::View *) {
            return QTime::currentTime().toString(str.mid(5));
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("ENV:"),
        i18n("Access to environment variables."),
        [](const QStringView &str, KTextEditor::View *) {
            return QString::fromLocal8Bit(qgetenv(str.mid(4).toLocal8Bit().constData()));
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("JS:"),
        i18n("Evaluate simple JavaScript statements."),
        [](const QStringView &str, KTextEditor::View *) {
            QJSEngine jsEngine;
            const QJSValue out = jsEngine.evaluate(str.toString());
            return out.toString();
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("PercentEncoded:"),
        i18n("Encode text to make it URL compatible."),
        [](const QStringView &str, KTextEditor::View *) {
            return QString::fromUtf8(QUrl::toPercentEncoding(str.mid(15).toString()));
        },
        true));

    mng.addVariable(Variable(
        QStringLiteral("UUID"),
        i18n("Generate a new UUID."),
        [](const QStringView &, KTextEditor::View *) {
            return QUuid::createUuid().toString(QUuid::WithoutBraces);
        },
        false));
}

KateVariableExpansionManager::KateVariableExpansionManager(QObject *parent)
    : QObject(parent)
{
    // register default variables for expansion
    registerVariables(*this);
}

bool KateVariableExpansionManager::addVariable(const KTextEditor::Variable &var)
{
    if (!var.isValid()) {
        return false;
    }

    // reject duplicates
    const auto alreadyExists = std::any_of(m_variables.begin(), m_variables.end(), [&var](const KTextEditor::Variable &v) {
        return var.name() == v.name();
    });
    if (alreadyExists) {
        return false;
    }

    // require a ':' in prefix matches (aka %{JS:1+1})
    if (var.isPrefixMatch() && !var.name().contains(QLatin1Char(':'))) {
        return false;
    }

    m_variables.push_back(var);
    return true;
}

bool KateVariableExpansionManager::removeVariable(const QString &name)
{
    auto it = std::find_if(m_variables.begin(), m_variables.end(), [&name](const KTextEditor::Variable &var) {
        return var.name() == name;
    });
    if (it != m_variables.end()) {
        m_variables.erase(it);
        return true;
    }
    return false;
}

KTextEditor::Variable KateVariableExpansionManager::variable(const QString &name) const
{
    auto it = std::find_if(m_variables.begin(), m_variables.end(), [&name](const KTextEditor::Variable &var) {
        return var.name() == name;
    });
    if (it != m_variables.end()) {
        return *it;
    }
    return {};
}

const QList<KTextEditor::Variable> &KateVariableExpansionManager::variables() const
{
    return m_variables;
}

bool KateVariableExpansionManager::expandVariable(const QString &name, KTextEditor::View *view, QString &output) const
{
    // first try exact matches
    auto var = variable(name);
    if (!var.isValid()) {
        // try prefix matching
        for (auto &v : m_variables) {
            if (v.isPrefixMatch() && name.startsWith(v.name())) {
                var = v;
                break;
            }
        }
    }

    if (var.isValid()) {
        output = var.evaluate(name, view);
        return true;
    }

    return false;
}

QString KateVariableExpansionManager::expandText(const QString &text, KTextEditor::View *view)
{
    return KateMacroExpander::expandMacro(text, view);
}

void KateVariableExpansionManager::showDialog(const QList<QWidget *> &widgets, const QStringList &names) const
{
    // avoid any work in case no widgets or only nullptrs were provided
    if (widgets.isEmpty() || std::all_of(widgets.cbegin(), widgets.cend(), [](const QWidget *w) {
            return w == nullptr;
        })) {
        return;
    }

    // collect variables
    QList<KTextEditor::Variable> vars;
    if (!names.isEmpty()) {
        for (const auto &name : names) {
            const auto var = variable(name);
            if (var.isValid()) {
                vars.push_back(var);
            }
            // else: Not found, silently ignore for now
            //       Maybe raise a qCWarning(LOG_KTE)?
        }
    } else {
        vars = variables();
    }

    // if we have no vars at all, do nothing
    if (vars.isEmpty()) {
        return;
    }

    // find parent dialog (for taskbar sharing, centering, ...)
    QWidget *parentDialog = nullptr;
    for (auto widget : widgets) {
        if (widget) {
            parentDialog = widget->window();
            break;
        }
    }

    // show dialog
    auto dlg = new KateVariableExpansionDialog(parentDialog);
    for (auto widget : widgets) {
        if (widget) {
            dlg->addWidget(widget);
        }
    }

    // add provided variables...
    for (const auto &var : std::as_const(vars)) {
        if (var.isValid()) {
            dlg->addVariable(var);
        }
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
