/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_H
#define KATE_SCRIPT_H

#include <QJSValue>
#include <QMap>
#include <QString>

class QJSEngine;

namespace KTextEditor
{
class ViewPrivate;
}

class KateScriptEditor;
class KateScriptDocument;
class KateScriptView;

namespace Kate
{
enum class ScriptType {
    /** The script is an indenter */
    Indentation,
    /** The script contains command line commands */
    CommandLine,
    /** Don't know what kind of script this is */
    Unknown
};
}

// BEGIN KateScriptHeader

class KateScriptHeader
{
public:
    KateScriptHeader() = default;
    virtual ~KateScriptHeader() = default;

    inline void setLicense(const QString &license)
    {
        m_license = license;
    }
    inline const QString &license() const
    {
        return m_license;
    }

    inline void setAuthor(const QString &author)
    {
        m_author = author;
    }
    inline const QString &author() const
    {
        return m_author;
    }

    inline void setRevision(int revision)
    {
        m_revision = revision;
    }
    inline int revision() const
    {
        return m_revision;
    }

    inline void setKateVersion(const QString &kateVersion)
    {
        m_kateVersion = kateVersion;
    }
    inline const QString &kateVersion() const
    {
        return m_kateVersion;
    }

    inline void setScriptType(Kate::ScriptType scriptType)
    {
        m_scriptType = scriptType;
    }
    inline Kate::ScriptType scriptType() const
    {
        return m_scriptType;
    }

private:
    QString m_license; ///< the script's license, e.g. LGPL
    QString m_author; ///< the script author, e.g. "John Smith <john@example.com>"
    int m_revision = 0; ///< script revision, a simple number, e.g. 1, 2, 3, ...
    QString m_kateVersion; ///< required katepart version
    Kate::ScriptType m_scriptType = Kate::ScriptType::Unknown; ///< the script type
};
// END

// BEGIN KateScript

/**
 * KateScript objects represent a script that can be executed and inspected.
 */
class KateScript
{
public:
    enum InputType { InputURL, InputSCRIPT };

    typedef QMap<QString, QJSValue> FieldMap;

    /**
     * Create a new script representation, passing either a file or the script
     * content @p urlOrScript to it.
     * In case of a file, loading of the script will happen lazily.
     */
    explicit KateScript(const QString &urlOrScript, enum InputType inputType = InputURL);
    virtual ~KateScript();

    /** The script's URL */
    const QString &url()
    {
        return m_url;
    }

    /**
     * Load the script. If loading is successful, returns true. Otherwise, returns
     * returns false and an error message will be set (see errorMessage()).
     * Note that you don't have to call this -- it is called as necessary by the
     * functions that require it.
     * Subsequent calls to load will return the value it returned the first time.
     */
    bool load();

    /**
     * set view for this script for the execution
     * will trigger load!
     */
    bool setView(KTextEditor::ViewPrivate *view);

    /**
     * Get a QJSValue for a global item in the script given its name, or an
     * invalid QJSValue if no such global item exists.
     */
    QJSValue global(const QString &name);

    /**
     * Return a function in the script of the given name, or an invalid QJSValue
     * if no such function exists.
     */
    QJSValue function(const QString &name);

    /** Return a context-specific error message */
    const QString &errorMessage()
    {
        return m_errorMessage;
    }

    /** Returns the backtrace when a script has errored out */
    static QString backtrace(const QJSValue &error, const QString &header = QString());

    /** Execute a piece of code **/
    QJSValue evaluate(const QString &program, const FieldMap &env = FieldMap());

    /** Displays the backtrace when a script has errored out */
    void displayBacktrace(const QJSValue &error, const QString &header = QString());

    /** Clears any uncaught exceptions in the script engine. */
    void clearExceptions();

    /** set the general header after construction of the script */
    void setGeneralHeader(const KateScriptHeader &generalHeader);
    /** Return the general header */
    KateScriptHeader &generalHeader();

protected:
    /** Checks for exception and gives feedback on the console. */
    bool hasException(const QJSValue &object, const QString &file);

private:
    /** Whether or not there has been a call to load */
    bool m_loaded = false;

    /** Whether or not the script loaded successfully into memory */
    bool m_loadSuccessful = false;

    /** The script's URL */
    QString m_url;

    /** An error message set when an error occurs */
    QString m_errorMessage;

protected:
    /** The Qt interpreter for this script */
    QJSEngine *m_engine = nullptr;

private:
    /** general header data */
    KateScriptHeader m_generalHeader;

    /** wrapper objects */
    KateScriptEditor *m_editor = nullptr;
    KateScriptDocument *m_document = nullptr;
    KateScriptView *m_view = nullptr;

private:
    /** if input is script or url**/
    enum InputType m_inputType;
    QString m_script;
};

// END

#endif
