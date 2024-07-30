/*
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variablelistview.h"
#include "variableeditor.h"
#include "variableitem.h"

#include <QRegularExpression>
#include <QResizeEvent>

VariableListView::VariableListView(const QString &variableLine, QWidget *parent)
    : QScrollArea(parent)
{
    setBackgroundRole(QPalette::Base);

    setWidget(new QWidget(this));

    parseVariables(variableLine);
}

void VariableListView::parseVariables(const QString &line)
{
    QString tmp = line.trimmed();
    if (tmp.startsWith(QLatin1String("kate:"))) {
        tmp.remove(0, 5);
    }

    QStringList variables = tmp.split(QLatin1Char(';'), Qt::SkipEmptyParts);

    const QRegularExpression sep(QStringLiteral("\\s+"));
    for (int i = 0; i < variables.size(); ++i) {
        QStringList pair = variables[i].split(sep, Qt::SkipEmptyParts);
        if (pair.size() < 2) {
            continue;
        }
        if (pair.size() > 2) { // e.g. fonts have spaces in the value. Hence, join all value items again
            QString key = pair[0];
            pair.removeAt(0);
            QString value = pair.join(QLatin1Char(' '));
            pair.clear();
            pair << key << value;
        }

        m_variables[pair[0]] = pair[1];
    }
}

void VariableListView::addItem(VariableItem *item)
{
    // overwrite default value when variable exists in modeline
    auto it = m_variables.find(item->variable());
    if (it != m_variables.end()) {
        item->setValueByString(it->second);
        item->setActive(true);
    }

    VariableEditor *editor = item->createEditor(widget());
    editor->setBackgroundRole((m_editors.size() % 2) ? QPalette::AlternateBase : QPalette::Base);

    m_editors.push_back(editor);
    m_items.push_back(item);

    connect(editor, &VariableEditor::valueChanged, this, &VariableListView::changed);
}

void VariableListView::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);

    // calculate sum of all editor heights
    int listHeight = 0;
    for (QWidget *w : std::as_const(m_editors)) {
        listHeight += w->sizeHint().height();
    }

    // resize scroll area widget
    QWidget *top = widget();
    top->resize(event->size().width(), listHeight);

    // set client geometries correctly
    int h = 0;
    for (QWidget *w : std::as_const(m_editors)) {
        w->setGeometry(0, h, top->width(), w->sizeHint().height());
        h += w->sizeHint().height();
    }
}

void VariableListView::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous()) {
        Q_EMIT aboutToHide();
    }
    QScrollArea::hideEvent(event);
}

QString VariableListView::variableLine()
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        VariableItem *item = m_items[i];

        if (item->isActive()) {
            m_variables[item->variable()] = item->valueAsString();
        } else {
            m_variables.erase(item->variable());
        }
    }

    QString line;
    for (const auto &var : m_variables) {
        if (!line.isEmpty()) {
            line += QLatin1Char(' ');
        }
        line += var.first + QLatin1Char(' ') + var.second + QLatin1Char(';');
    }
    line.prepend(QLatin1String("kate: "));

    return line;
}

#include "moc_variablelistview.cpp"
