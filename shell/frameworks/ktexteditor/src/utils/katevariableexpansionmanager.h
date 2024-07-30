/*
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_VARIABLE_MANAGER
#define KTEXTEDITOR_VARIABLE_MANAGER

#include <QList>
#include <QString>
#include <QWidget>

#include "variable.h"

namespace KTextEditor
{
class View;
}

/**
 * Manager class for variable expansion.
 */
class KateVariableExpansionManager : public QObject
{
public:
    /**
     * Constructor with @p parent that takes ownership.
     */
    KateVariableExpansionManager(QObject *parent);

    /**
     * Adds @p variable to the expansion list view.
     */
    bool addVariable(const KTextEditor::Variable &variable);

    /**
     * Removes variable @p name.
     */
    bool removeVariable(const QString &name);

    /**
     * Returns the variable called @p name.
     */
    KTextEditor::Variable variable(const QString &name) const;

    /**
     * Returns all registered variables.
     */
    const QList<KTextEditor::Variable> &variables() const;

    bool expandVariable(const QString &variable, KTextEditor::View *view, QString &output) const;

    static QString expandText(const QString &text, KTextEditor::View *view);

    void showDialog(const QList<QWidget *> &widgets, const QStringList &names) const;

private:
    QList<KTextEditor::Variable> m_variables;
};

#endif // KTEXTEDITOR_VARIABLE_MANAGER

// kate: space-indent on; indent-width 4; replace-tabs on;
