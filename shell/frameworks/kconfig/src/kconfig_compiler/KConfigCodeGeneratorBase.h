/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2020 Tomaz Cananbrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGCODEGENERATORBASE_H
#define KCONFIGCODEGENERATORBASE_H

#include <QFile>
#include <QList>
#include <QString>
#include <QTextStream>

#include "KConfigCommonStructs.h"
#include "KConfigParameters.h"

class CfgEntry;
struct ParseResult;

/* This class manages the base of writing a C - Based code */
class KConfigCodeGeneratorBase
{
public:
    enum ScopeFinalizer {
        None,
        Semicolon,
    };

    KConfigCodeGeneratorBase(const QString &inputFileName, // The kcfg file
                             const QString &baseDir, // where we should store the generated file
                             const QString &fileName, // the name of the generated file
                             const KConfigParameters &parameters, // parameters passed to the generator
                             ParseResult &parseResult // The pre processed configuration entries
    );
    virtual ~KConfigCodeGeneratorBase();

    // iterates over the header list adding an #include directive.
    void addHeaders(const QStringList &header);

    // Create all the namespace indentation levels based on the parsed result and parameters */
    void beginNamespaces();

    // Closes all the namespaces adding lines with single '}'
    void endNamespaces();

    // Add the correct amount of whitespace in the code.
    QString whitespace() const;

    // start a block scope `{` and increase indentation level.
    void endScope(ScopeFinalizer finalizer = None);

    // end a block scope `}` and decrease indentation level.
    void startScope();

    // start writing to the output file
    virtual void start();

    // save the result on the disk
    void save();

    // Code Implementations
    // Implements the `Get` methods for the CfgEntry
    // TODO: write to the stream directly without returning a QString.
    QString memberAccessorBody(const CfgEntry *e, bool globalEnums) const;

    // Implements the is<Param>Immutable for the CfgEntry
    void memberImmutableBody(const CfgEntry *e, bool globalEnums);

    // Implements the `Set` methods for the CfgEntry
    void memberMutatorBody(const CfgEntry *e);

    // This is the code that creates the logic for the Setter / Mutator.
    // It *just* creates the if test, no body. The reason is that just
    // the if test was more than 20 lines of code and hard to understand
    // what was happening in a bigger function.
    void createIfSetLogic(const CfgEntry *e, const QString &varExpression);

protected:
    /* advance the number of spaces for the indentation level */
    void indent();

    /* reduce the number of spaces for the indentation level */
    void unindent();

    QString inputFile() const
    {
        return m_inputFile;
    }
    QString fileName() const
    {
        return m_fileName;
    }
    QString baseDir() const
    {
        return m_baseDir;
    }
    QString This() const
    {
        return m_this;
    }
    QString Const() const
    {
        return m_const;
    }
    KConfigParameters cfg() const
    {
        return m_cfg;
    }

    // Can't be const.
    QTextStream &stream()
    {
        return m_stream;
    }

    // HACK: This needs to be accessible because the HeaderGenerator actually modifies
    // it while running. Considering that this is a the result of the xml Parse, and not
    // the result of generating code, I consider this to be quite wrong - but moving the
    // changes away from the header generator only created more problems and I have to
    // investigate more.
    ParseResult &parseResult; // the result of the parsed kcfg file

private:
    QString m_inputFile; // the base file name, input file is based on this.

    QString m_baseDir; // Where we are going to save the file
    QString m_fileName; // The file name

    KConfigParameters m_cfg; // The parameters passed via the kcfgc file
    QTextStream m_stream; // the stream that operates in the file to write data.
    QFile m_file; // The file handler.

    // Special access to `this->` and `const` thru the code.
    QString m_this;
    QString m_const;
    int m_indentLevel = 0;
};

#endif
