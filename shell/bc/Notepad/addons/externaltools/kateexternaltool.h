/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KTEXTEDITOR_KATE_EXTERNALTOOL_H
#define KTEXTEDITOR_KATE_EXTERNALTOOL_H

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QUrl>

class KConfigGroup;

/**
 * This class defines a single external tool.
 */
class KateExternalTool
{
public:
    /**
     * Defines whether any document should be saved before running the tool.
     */
    enum class SaveMode {
        //! Do not save any document.
        None,
        //! Save current document.
        CurrentDocument,
        //! Save all documents
        AllDocuments
    };

    /**
     * Defines where to redirect stdout from the tool.
     */
    enum class OutputMode {
        Ignore,
        InsertAtCursor,
        ReplaceSelectedText,
        ReplaceCurrentDocument,
        AppendToCurrentDocument,
        InsertInNewDocument,
        CopyToClipboard,
        DisplayInPane
    };

    enum class Trigger {
        //! No trigger
        None,
        //! Run the tool before saving
        BeforeSave,
        //! Run the tool after saving
        AfterSave,
    };

public:
    /// The category used in the menu to categorize the tool.
    QString category;
    /// The name used in the menu.
    QString name;
    /// the icon to use in the menu.
    QString icon;
    /// The name or path of the executable.
    QString executable;
    /// The command line arguments.
    QString arguments;
    /// The stdin input.
    QString input;
    /// The working directory, if specified.
    QString workingDir;
    /// Optional list of mimetypes for which this action is valid.
    QStringList mimetypes;
    /// The name for the action for persistent keyboard shortcuts.
    /// This is generated first time the action is is created.
    QString actionName;
    /// The name for the commandline.
    QString cmdname;
    /// Possibly save documents prior to activating the tool command.
    SaveMode saveMode = SaveMode::None;
    /// Reload current document after execution
    bool reload = false;
    /// Defines where to redirect the tool's output
    OutputMode outputMode = OutputMode::Ignore;
    /// Trigger to run tool
    Trigger trigger = Trigger::None;

public:
    /// This is set when loading the Tool from disk.
    bool hasexec = false;

    /**
     * @return true if the @p mimetype matches.
     */
    bool matchesMimetype(const QString &mimetype) const;

    /**
     * @return true if "executable" exists and has the executable bit set, or is
     * empty.
     * This is run at least once, and the tool is disabled if it fails.
     */
    bool checkExec() const;

    /**
     * Load tool data from the config group @p cg.
     */
    void load(const KConfigGroup &cg);

    /**
     * Save tool data to the config group @p cg.
     */
    void save(KConfigGroup &cg) const;

    /**
     * Returns the translated name if possible.
     */
    QString translatedName() const;

    /**
     * Returns the translated category if possible.
     */
    QString translatedCategory() const;

    /**
     * Returns the config file name for this tool, created based on the tool "name".
     * this will be the name of the config file in e.g. ~/.config/kate/externaltools/
     * will ensure we end up with some valid file name
     */
    static QString configFileName(QString name)
    {
        // just percent encode the name, see bug 453272
        // we add a file ending to not clash with old files, old files were all lowercase
        return QString::fromUtf8(QUrl::toPercentEncoding(name)) + QStringLiteral(".ini");
    }

    /**
     * OLD names: we need this to cleanup!
     * Returns the config file name for this tool, created based on the tool "name", e.g.
     * "Clang Format Full File" -> clang_format_full_file
     * this will be the name of the config file in e.g. ~/.config/kate/externaltools/
     */
    static QString configFileNameOldStyleOnlyForRemove(QString name)
    {
        name.replace(QLatin1Char(' '), QLatin1Char('_'));
        // '(' and ')' are problematic as file names in the .qrc file
        name.replace(QLatin1Char('('), QLatin1Char('_'));
        name.replace(QLatin1Char(')'), QLatin1Char('_'));
        return name.toLower();
    }
};

/**
 * Compares for equality. All fields have to match.
 */
bool operator==(const KateExternalTool &lhs, const KateExternalTool &rhs);

// for use in QVariant (QAction::setData() and QAction::data())
Q_DECLARE_METATYPE(KateExternalTool *)

#endif // KTEXTEDITOR_KATE_EXTERNALTOOL_H

// kate: space-indent on; indent-width 4; replace-tabs on;
